/**
* \file Solution.cpp
*
* \brief Defines the methods of the class Solution.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#include "src/Solution/Solution.hpp"

namespace Space4AI
{
void
Solution::read_solution_from_file(
  const std::string& file_run,
  const System& system
)
{
  std::ifstream file(file_run);

  if(!file)
  {
    const std::string err_message = "From *Solution::read_solution_from_file(...)*: Cannot open file" + file_run;
    Logger::Error(err_message);
    Logger::Error("Exiting without opening the solution...");
    throw std::runtime_error(err_message);
  }

  const auto& comp_name_to_idx = system.get_system_data().get_comp_name_to_idx();
  const auto& part_name_to_part_idx = system.get_system_data().get_part_name_to_part_idx();
  const auto& res_name_to_type_and_idx = system.get_system_data().get_res_name_to_type_and_idx();
  const auto& gc_name_to_idx = system.get_system_data().get_gc_name_to_idx();
  const auto& components = system.get_system_data().get_components();
  const auto& all_resources = system.get_system_data().get_all_resources();
  nl::json configuration_file;
  file >> configuration_file;
  const std::size_t comp_num = comp_name_to_idx.size();
  const std::size_t res_type_idx_count = ResIdxFromType(ResourceType::Count);
  solution_data.y_hat.resize(comp_num);
  solution_data.used_resources.resize(comp_num);
  this->comp_perfs.resize(comp_num);
  this->path_perfs.resize(gc_name_to_idx.size());

  // loop on components
  for(std::size_t i = 0; i < comp_num; ++i)
  {
    solution_data.y_hat[i].resize(res_type_idx_count);

    // loop on type of resources
    for(std::size_t j = 0; j < res_type_idx_count; ++j)
    {
      solution_data.y_hat[i][j].resize(components[i].get_partitions().size());

      for(std::size_t k = 0; k < components[i].get_partitions().size(); ++k)
      {
        solution_data.y_hat[i][j][k].resize(all_resources.get_number_resources(j));
      }
    }
  }

  std::size_t comp_idx;
  std::size_t res_type_idx;
  std::size_t part_idx;
  std::size_t res_idx;

  for(const auto& [comp, comp_data] : configuration_file.at("components").items())
  {
    comp_idx = comp_name_to_idx.at(comp);

    for(const auto& [part, part_data] : comp_data.items())
    {
      if(part == "response_time")
      {
        comp_perfs[comp_idx] = part_data.get<TimeType>();
        continue;
      }

      if(part == "response_time_threshold")
      {
        continue;
      }

      // part_data has truly partition_data at this point

      for(const auto& [cl, cl_data] : part_data.items())
      {
        for(const auto& [res, res_data] : cl_data.items())
        {
          res_type_idx = ResIdxFromType(res_name_to_type_and_idx.at(res).first);
          res_idx = res_name_to_type_and_idx.at(res).second;
          part_idx = part_name_to_part_idx.at(comp + part);

          if(res_type_idx == ResIdxFromType(ResourceType::Faas))
          {
            solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] = 1;
          }
          else // Edge or VM
          {
            solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] = static_cast<size_t>(res_data.at("number"));
          }

          solution_data.used_resources[comp_idx].emplace_back(part_idx, res_type_idx, res_idx);
        }
      }
    }
  }

  for(const auto& [path, path_data] : configuration_file.at("global_constraints").items())
  {
    const std::size_t path_idx = gc_name_to_idx.at(path);

    if(path_data.at("path_response_time").is_number())
    {
      this->path_perfs[path_idx] = path_data.at("path_response_time").get<TimeType>();
    }
    else
    {
      Logger::Info("In *Solution::read_configuration_file(...): path response_time not a number ...");
      throw("In *Solution::read_configuration_file(...): path response_time not a number ... At the moment program crash");
    }
  }

  total_cost = configuration_file.at("total_cost").get<CostType>();
}

nl::json
Solution::to_json(const System& system) const
{
  const auto& system_data = system.get_system_data();
  const auto& components = system_data.get_components();
  const auto& all_resources = system_data.get_all_resources();
  const auto& local_constraints = system_data.get_local_constraints();
  nl::json jsolution;
  jsolution["Lambda"] = system_data.get_lambda();
  nl::json jcomponents;

  //loop over components
  for(std::size_t i = 0; i < solution_data.used_resources.size(); ++i)
  {
    //get component name
    const auto& comp_name = components[i].get_name();
    //get partitions
    const auto& partitions = components[i].get_partitions();

    for(const auto& tuple : solution_data.used_resources[i])
    {
      //get idxs
      const auto& part_idx = std::get<0>(tuple);
      const auto& res_type_idx = std::get<1>(tuple);
      const auto& res_idx = std::get<2>(tuple);
      //get partition name
      const auto& part_name = partitions[part_idx].get_name();
      //get cl name
      const auto& cl_name = all_resources.get_cl_name(ResTypeFromIdx(res_type_idx), res_idx);
      //get resource name
      const auto& res_name = all_resources.get_name(ResTypeFromIdx(res_type_idx), res_idx);
      //get resource description
      const auto& res_description = all_resources.get_description(ResTypeFromIdx(res_type_idx), res_idx);
      //get resource cost
      const auto& res_cost = all_resources.get_cost(ResTypeFromIdx(res_type_idx), res_idx) * solution_data.y_hat[i][res_type_idx][part_idx][res_idx];
      //get resource memory
      const auto& res_memory = all_resources.get_memory(ResTypeFromIdx(res_type_idx), res_idx);

      if(res_type_idx == ResIdxFromType(ResourceType::Faas))
      {
        const auto& faas_res = all_resources.get_resource<ResourceType::Faas>(res_idx);
        //get idle_time_before_kill
        const auto& idle_time_before_kill = faas_res.get_idle_time_before_kill();
        //get transition cost
        const auto& transition_cost = faas_res.get_transition_cost();
        //create resource json object
        const nl::json jresource =
        {
          {"description", res_description},
          {"cost", res_cost},
          {"memory", res_memory},
          {"idle_time_before_kill", idle_time_before_kill},
          {"transition_cost", transition_cost}
        };
        //populate jcomponents
        jcomponents[comp_name][part_name][cl_name][res_name] = jresource;
      }
      else //the resource is edge or vm
      {
        //get number
        const auto& number = solution_data.y_hat[i][res_type_idx][part_idx][res_idx];
        //create resource json object
        const nl::json jresource =
        {
          {"description", res_description},
          {"cost", res_cost},
          {"memory", res_memory},
          {"number", number}
        };
        //populate jcomponents
        jcomponents[comp_name][part_name][cl_name][res_name] = jresource;
      }
    }

    //get response time of the component i
    jcomponents[comp_name]["response_time"] = comp_perfs[i];
    jcomponents[comp_name]["response_time_threshold"] = local_constraints[i].get_max_res_time();
  }

  jsolution["components"] = jcomponents;
  //Global constraints
  const auto& global_constraints = system_data.get_global_constraints();
  nl::json jgc;

  //loop over the paths
  for(std::size_t k = 0; k < global_constraints.size(); ++k)
  {
    //get path name
    const auto& path_name = global_constraints[k].get_path_name();
    //get comp_idxs
    const auto& comp_idxs = global_constraints[k].get_comp_idxs();
    //vector to save components' names
    std::vector<std::string> comp_names(comp_idxs.size());

    //loop over components and save their names
    for(std::size_t i = 0; i < comp_names.size(); ++i)
    {
      std::size_t comp_idx = comp_idxs[i];
      comp_names[i] = components[comp_idx].get_name();
    }

    //convert the vector comp_names into a json object
    nl::json jpath_comps(comp_names);
    //populate the global constraint json
    jgc[path_name]["components"] = jpath_comps;
    //get path_response_time
    jgc[path_name]["path_response_time"] = path_perfs[k];
    //get path_response_time_threshold
    jgc[path_name]["path_response_time_threshold"] = global_constraints[k].get_max_res_time();
  }

  jsolution["global_constraints"] = jgc;
  //total cost
  jsolution["total_cost"] = total_cost;
  return jsolution;
}

void
Solution::print_solution(const System& system, const std::string& path) const
{
  //get solution as a nl::json object
  nl::json jsolution = to_json(system);
  //print in a json file
  std::ofstream outputsol;
  outputsol.open(path);

  if(!outputsol)   // file couldn't be opened
  {
    Logger::Error("File *" + path + "* cannot be opened");
    throw std::runtime_error("In *Solution::print_solution() *" + path + "* cannot be opened. Make sure that all the folders in path exist!");
  }

  outputsol << std::setw(4) << jsolution << std::endl;
  outputsol.close();
  Logger::Info("Solution saved with success at: " + path);
}

bool
Solution::preliminary_constraints_check_assignments(
  const System& system
) const
{
  bool feasible = true;
  const auto& compatibility_matrix = system.get_system_data().get_compatibility_matrix();
  const auto& all_resources = system.get_system_data().get_all_resources();
  const auto& components = system.get_system_data().get_components();
  const std::size_t tot_comp = components.size();

  // loop over components
  for(std::size_t comp_idx = 0; comp_idx < tot_comp && feasible; ++comp_idx)
  {
    std::vector<bool> parts_with_res(components[comp_idx].get_partitions().size(), false);

    for(auto [part_idx, res_type_idx, res_idx] : solution_data.used_resources[comp_idx])
    {
      // check that each component partition is assigned to exactly one resource
      bool already_inserted = parts_with_res[part_idx];

      // check if the parition was inserted and it is compatible
      if(!already_inserted && compatibility_matrix[comp_idx][res_type_idx][part_idx][res_idx])
      {
        parts_with_res[part_idx] = true;

        // check that the number of resources assigned to each
        // component partition is at most equal to the number of
        // available resources of that type
        // (cheking all the components, in principle only VM should be checked)
        if(solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] > all_resources.get_number_avail(ResTypeFromIdx(res_type_idx), res_idx))
        {
          feasible = false;
          break;
        }
      }
      else
      {
        feasible = false;
        break;
      }
    }
  }

  return feasible;
}

bool
Solution::memory_constraints_check(
  const System& system
) const
{
  bool feasible = true;
  const auto& components = system.get_system_data().get_components();
  const auto& all_resources = system.get_system_data().get_all_resources();
  std::vector<std::vector<DataType>> mem_occupation(
      ResIdxFromType(ResourceType::Count)
    );

  for(size_t type_idx = 0; type_idx < mem_occupation.size(); ++type_idx)
  {
    mem_occupation[type_idx].resize(all_resources.get_number_resources(type_idx), 0.0);
  }

  for(size_t comp_idx = 0; comp_idx < components.size() && feasible; ++comp_idx)
  {
    const auto& partitions = components[comp_idx].get_partitions();

    for(auto [part_idx, res_type_idx, res_idx] : solution_data.used_resources[comp_idx])
    {
      mem_occupation[res_type_idx][res_idx] += partitions[part_idx].get_memory();
      const auto max_memory = (res_type_idx != ResIdxFromType(ResourceType::Faas)) ?
        solution_data.n_used_resources[res_type_idx][res_idx] *
        all_resources.get_memory(ResTypeFromIdx(res_type_idx), res_idx)
        :
        all_resources.get_memory(ResTypeFromIdx(res_type_idx), res_idx);

      if(mem_occupation[res_type_idx][res_idx] > max_memory)
      {
        feasible = false;
        break;
      }
    }
  }

  return feasible;
}

bool
Solution::move_backward_check(
  const System& system
) const
{
  bool feasible = true;
  const auto edge_idx = ResIdxFromType(ResourceType::Edge);
  const auto vm_idx = ResIdxFromType(ResourceType::VM);
  const auto faas_idx = ResIdxFromType(ResourceType::Faas);
  const auto& components = system.get_system_data().get_components();

  for(std::size_t comp_idx = 0; comp_idx < components.size() && feasible; ++comp_idx)
  {
    auto it = solution_data.used_resources[comp_idx].cbegin(); // iterator to find minimum partition running on vm or faas
    auto r_it = solution_data.used_resources[comp_idx].crbegin(); // iterator to find maximum partition running on edge
    auto max_idx_part_on_edge = std::numeric_limits<size_t>::min();
    auto min_idx_part_on_cloud_faas = std::numeric_limits<size_t>::max();
    bool not_found_edge = true;
    bool not_found_cloud_faas = true;

    while(
      feasible &&
      (not_found_edge || not_found_cloud_faas) &&
      it != solution_data.used_resources[comp_idx].cend() &&
      r_it != solution_data.used_resources[comp_idx].crend() &&
      *r_it >= *it)
    {
      if(not_found_edge && std::get<1>(*r_it) == edge_idx)
      {
        not_found_edge = false;
        max_idx_part_on_edge = std::get<0>(*r_it);
      }

      if(not_found_cloud_faas && (std::get<1>(*it) == vm_idx || std::get<1>(*it) == faas_idx))
      {
        not_found_cloud_faas = false;
        min_idx_part_on_cloud_faas = std::get<0>(*it);
      }

      feasible = (max_idx_part_on_edge <= min_idx_part_on_cloud_faas);
      it++;
      r_it++;
    }
  }

  return feasible;
}

bool
Solution::performance_assignment_check(
  const System& system
) const
{
  const std::vector<size_t> res_type_to_check = {ResIdxFromType(ResourceType::Edge), ResIdxFromType(ResourceType::VM)};
  const auto& components = system.get_system_data().get_components();
  const auto& performance = system.get_performance();

  for(auto res_type_idx : res_type_to_check)
  {
    // loop over resources index by type
    for(size_t res_idx = 0; res_idx < system.get_system_data().get_all_resources().get_number_resources(res_type_idx); ++res_idx)
    {
      // count how many partitions are run on the current resource
      size_t count_part{0};

      // loop over components
      for(size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx)
      {
        for(size_t part_idx = 0; part_idx < components[comp_idx].get_partitions().size(); ++part_idx)
        {
          if(solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] > 0)
          {
            // increment_counter
            count_part += 1;

            if(!performance[comp_idx][res_type_idx][part_idx][res_idx]->get_allows_colocation())
            {
              if(count_part > 1)
              {
                return false;
              }
            }
          }
        }
      }
    }
  }

  return true;
}

bool
Solution::local_constraints_check(
  const System& system
)
{
  bool feasible = true;
  const auto& local_constraints = system.get_system_data().get_local_constraints();
  comp_perfs.resize(local_constraints.size(), std::numeric_limits<TimeType>::quiet_NaN());
  local_slack_values.resize(local_constraints.size(), std::numeric_limits<TimeType>::quiet_NaN());

  for(size_t i = 0; i < local_constraints.size(); ++i)
  {
    comp_perfs[i] =
      SystemPerformanceEvaluator::get_perf_evaluation(
        local_constraints[i].get_comp_idx(),
        system,
        solution_data
      );

    // this takes into account the compute_utilization.
    // If utiilization > 1, response_time is negative- (get_perf_evaluation return -1)
    // Check lables of response_time (when is negative, NaN, +inf, max ...)
    if(comp_perfs[i] < 0)
    {
      feasible = false;
    }
    else
    {
      this->local_slack_values[i] = local_constraints[i].get_max_res_time() - comp_perfs[i];

      if(this->local_slack_values[i] < 0)
      {
        feasible = false;
      }
    }
  }

  return feasible;
}

bool
Solution::global_constraints_check(
  const System& system
)
{
  bool feasible = true;
  const auto& global_constraints = system.get_system_data().get_global_constraints();
  path_perfs.resize(global_constraints.size(), std::numeric_limits<TimeType>::quiet_NaN());
  this->global_slack_values.resize(global_constraints.size(), std::numeric_limits<TimeType>::quiet_NaN());

  for(size_t i = 0; i < global_constraints.size(); ++i)
  {
    const auto [path_feasible, path_response_time] =
      this->path_global_constraint_check(
        global_constraints[i].get_comp_idxs(),
        system
      );
    feasible = feasible && path_feasible;
    path_perfs[i] = path_response_time;
    global_slack_values[i] = global_constraints[i].get_max_res_time() - this->path_perfs[i];
    feasible = feasible && (global_slack_values[i] >= 0);
  }

  return feasible;
}

std::pair<bool, double>
Solution::path_global_constraint_check(
  const std::vector<size_t>& comp_idxs,
  const System& system
) const
{
  std::vector<double> perf_components_path(comp_idxs.size(), 0.0);
  const auto& components = system.get_system_data().get_components();

  for(size_t i = 0; i < comp_idxs.size(); ++i)
  {
    perf_components_path[i] = comp_perfs[comp_idxs[i]];

    if(isnan(perf_components_path[i]))
    {
      return std::make_pair(false, std::numeric_limits<double>::quiet_NaN());
    }
  }

  // sum the response times of all components in the current path
  double sum = std::accumulate(
      perf_components_path.begin(),
      perf_components_path.end(),
      0.0
    );

  // network delay
  for(size_t i = 0; i < comp_idxs.size() - 1; ++i)
  {
    // get the indices of the current and the next component
    const auto curr_comp_idx = comp_idxs[i];
    const auto next_comp_idx = comp_idxs[i + 1];
    // resource index of the last partition
    const auto [curr_comp_last_part_idx, res1_type_idx, res1_idx] =
      *(solution_data.used_resources[curr_comp_idx].crbegin());
    //resource index of the first partition
    const auto [next_comp_first_part_idx, res2_type_idx, res2_idx] =
      *(solution_data.used_resources[next_comp_idx].cbegin());

    // different resources.
    if(res1_type_idx != res2_type_idx || res1_idx != res2_idx)
    {
      const auto data_size =
        components[curr_comp_idx].get_partition(curr_comp_last_part_idx).get_data_size();
      sum +=
        SystemPerformanceEvaluator:: get_network_delay(
          ResTypeFromIdx(res1_type_idx), res1_idx,
          ResTypeFromIdx(res2_type_idx), res2_idx,
          data_size,
          system
        );
    }
  }

  if(!isnan(sum))
  {
    return std::make_pair(true, sum);
  }
  else
  {
    return std::make_pair(false, sum);
  }
}

bool
Solution::check_feasibility(
  const System& system
)
{
  Logger::Debug("check_feasibility: Starting feasibility check of the solution ...");
  bool feasible = false;
  Logger::Debug("check_feasibility: Checking preliminary constraints assignments ...");
  feasible = this->preliminary_constraints_check_assignments(
      system
    );
  Logger::Debug("check_feasibility: Done!");

  if(feasible)
  {
    Logger::Debug("check_feasibility: Checking performance assignments ... ");
    feasible = this->performance_assignment_check(
        system
      );
    Logger::Debug("check_feasibility: Done");

    if(feasible)
    {
      Logger::Debug("check_feasibility: Checking memory constraints ... ");
      feasible = this->memory_constraints_check(
          system
        );
      Logger::Debug("check_feasibility: Done!");

      if(feasible)
      {
        Logger::Debug("check_feasibility: Checking move backward ... ");
        feasible = this->move_backward_check(
            system
          );
        Logger::Debug("check_feasibility: Done");

        if(feasible)
        {
          Logger::Debug("check_feasibility: Checking local constraints ...");
          feasible = this->local_constraints_check(system);
          Logger::Debug("check_feasibility: Done!");

          if(feasible)
          {
            Logger::Debug("check_feasibility: Checking global constraints ...");
            feasible = this->global_constraints_check(system);
            Logger::Debug("check_feasibility: Done!");
          }
        }
      }
    }
  }

  if(!feasible)
  {
    Logger::Debug("check_feasibility: Solution not feasible, failed last check!");
  }
  else
  {
    Logger::Debug("check_feasibility: Done feasibility check: Solution is feasible!");
  }

  this->feasibility = feasible;
  return feasible;
}

CostType
Solution::objective_function(
  const System& system
)
{
  Logger::Debug("objective_function: Computing objective function ... ");
  CostType cost = 0.0;
  std::vector<std::vector<bool>> res_cost_alredy_computed(ResIdxFromType(ResourceType::Count));
  const auto& all_resources = system.get_system_data().get_all_resources();
  const auto& components = system.get_system_data().get_components();
  const auto& performance = system.get_performance();

  for(size_t i = 0; i < res_cost_alredy_computed.size(); ++i)
  {
    res_cost_alredy_computed[i].resize(all_resources.get_number_resources(i));
  }

  for(size_t comp_idx = 0; comp_idx < solution_data.used_resources.size(); ++comp_idx)
  {
    for(const auto& [part_idx, res_type_idx, res_idx] : solution_data.used_resources[comp_idx])
    {
      if(!res_cost_alredy_computed[res_type_idx][res_idx])
      {
        res_cost_alredy_computed[res_type_idx][res_idx] = true;

        if(res_type_idx == ResIdxFromType(ResourceType::Edge))
        {
          const auto res_cost = all_resources.get_resource<ResourceType::Edge>(res_idx).get_cost();
          cost += solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] * res_cost;
        }
        else if(res_type_idx == ResIdxFromType(ResourceType::VM))
        {
          const auto res_cost = all_resources.get_resource<ResourceType::VM>(res_idx).get_cost();
          cost += solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] * res_cost;
        }
        else // Faas
        {
          const auto res_cost = all_resources.get_resource<ResourceType::Faas>(res_idx).get_cost();
          const double time = system.get_system_data().get_time();
          // ATTENTO: LA FILIPPINI USA COMP_LAMBDA... SECONDO ME E' GIUSTO PART_LAMBDA MA CHIEDI!
          const auto part_lambda = components[comp_idx].get_partition(part_idx).get_part_lambda();
          const auto warm_time =
            static_cast<FaasPE*>(
              performance[comp_idx][res_type_idx][part_idx][res_idx].get()
            )->get_demandWarm();
          cost += res_cost * warm_time * part_lambda * time;
        }
      }
    }
  }

  total_cost = cost;
  Logger::Debug("objective_function: Done computation of objective function!");
  return cost;
}


} // namespace Space4AI
