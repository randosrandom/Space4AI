/*
Copyright 2021 AI-SPRINT

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

/**
* \file Solution.cpp
*
* \brief Defines the methods of the class Solution.
*
* \author Randeep Singh
*/

#include <algorithm>

#include "src/Solution/Solution.hpp"

namespace Space4AI
{
Solution::Solution(const System& system):
  feasibility(false),
  total_cost(std::numeric_limits<CostType>::infinity())
{
  // Here I don't resize solution_data.* objects, since they are constructed by
  // the external class algorithm. However, here it is worth resizing all the
  // members that would be initialized during feasibility check, otherwise.
  const auto comp_size = system.get_system_data().get_components().size();
  const auto& all_resources = system.get_system_data().get_all_resources();
  const auto& paths_size = system.get_system_data().get_global_constraints().size();
  // Initialize memory_slack_values
  memory_slack_values.resize(ResIdxFromType(ResourceType::Count));
  res_costs.resize(ResIdxFromType(ResourceType::Count));

  for(size_t type_idx = 0; type_idx < ResIdxFromType(ResourceType::Count); ++type_idx)
  {
    memory_slack_values[type_idx].resize(all_resources.get_number_resources(type_idx), 0.0);
    res_costs[type_idx].resize(all_resources.get_number_resources(type_idx), NaN);
  }

  time_perfs.local_parts_perfs.resize(comp_size);
  time_perfs.local_parts_delays.resize(comp_size);
  time_perfs.comp_perfs.resize(comp_size, 0.0);
  time_perfs.comp_delays.resize(comp_size - 1, 0.0);
  time_perfs.path_perfs.resize(paths_size, 0.0);
}

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

  Logger::Info("solution::read_solution_from_file: Reading Design Time Solution...");
  const auto& comp_name_to_idx = system.get_system_data().get_comp_name_to_idx();
  const auto& part_name_to_part_idx = system.get_system_data().get_part_name_to_part_idx();
  const auto& res_name_to_type_and_idx = system.get_system_data().get_res_name_to_type_and_idx();
  const auto& gc_name_to_idx = system.get_system_data().get_gc_name_to_idx();
  const auto& components = system.get_system_data().get_components();
  const auto& all_resources = system.get_system_data().get_all_resources();
  const auto& cls = system.get_system_data().get_cls();
  const auto& cl_name_to_idx = system.get_system_data().get_cl_name_to_idx();
  nl::json configuration_file;
  file >> configuration_file;
  Logger::Debug("solution::read_solution_from_file: Resizing data structures...");
  const std::size_t comp_num = comp_name_to_idx.size();
  const std::size_t res_type_idx_count = ResIdxFromType(ResourceType::Count);
  solution_data.y_hat.resize(comp_num);
  solution_data.used_resources.resize(comp_num);
  solution_data.n_used_resources.resize(ResIdxFromType(ResourceType::Count));

  // resize y_hat
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

  // resize n_used_resources and dt_selected_resources
  for(std::size_t j = 0; j < res_type_idx_count - 1; ++j)
  {
    solution_data.n_used_resources[j].resize(all_resources.get_number_resources(j));
  }

  // resize selected_resources.selected_edge and sele
  selected_resources.selected_edge.resize(solution_data.n_used_resources[ResIdxFromType(ResourceType::Edge)].size());
  selected_resources.selected_vms.resize(solution_data.n_used_resources[ResIdxFromType(ResourceType::VM)].size());
  selected_resources.selected_vms_by_cl.resize(cls[ResIdxFromType(ResourceType::VM)].size());
  Logger::Debug("solution::read_solution_from_file: Data structure resized!");
  std::size_t comp_idx;
  std::size_t res_type_idx;
  std::size_t part_idx;
  std::size_t res_idx;
  Logger::Debug("solution::read_solution_from_file: Starting reading file...");

  for(const auto& [comp, comp_data] : configuration_file.at("components").items())
  {
    comp_idx = comp_name_to_idx.at(comp);

    for(const auto& [part, part_data] : comp_data.items())
    {
      if(part == "response_time")
      {
        time_perfs.comp_perfs[comp_idx] = part_data.get<TimeType>();
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
            const auto number = static_cast<size_t>(res_data.at("number"));
            solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] = number;
            solution_data.n_used_resources[res_type_idx][res_idx] = number;

            if(res_type_idx ==  ResIdxFromType(ResourceType::Edge))
            {
              selected_resources.selected_edge[res_idx] = true;
            }
            else // VM
            {
              selected_resources.selected_vms_by_cl[cl_name_to_idx[ResIdxFromType(ResourceType::VM)].at(cl)] =
                std::pair<bool, size_t> {true, res_idx};
              selected_resources.selected_vms[res_idx] = true;
            }
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
      time_perfs.path_perfs[path_idx] = path_data.at("path_response_time").get<TimeType>();
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
  for(std::size_t i = 0; i < components.size(); ++i)
  {
    //get component name
    const auto& comp_name = components[i].get_name();
    //get partitions
    const auto& partitions = components[i].get_partitions();

    for(size_t j=0; j<solution_data.used_resources[i].size(); ++j)
    {
      //get idxs
      const auto& tuple = solution_data.used_resources[i][j];

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

      jcomponents[comp_name][part_name]["response_time"] = time_perfs.local_parts_perfs[i][j];
    }
    //get response time of the component i
    jcomponents[comp_name]["response_time"] = time_perfs.comp_perfs[i];
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
    jgc[path_name]["path_response_time"] = time_perfs.path_perfs[k];
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

void
Solution::set_selected_resources(const System& system)
{
  const size_t edge_type_idx = ResIdxFromType(ResourceType::Edge);
  const size_t vm_type_idx = ResIdxFromType(ResourceType::VM);
  const size_t num_cls_vm = system.get_system_data().get_cls()[vm_type_idx].size();
  // Selected EDGE
  selected_resources.selected_edge.resize(solution_data.n_used_resources[edge_type_idx].size());
  std::copy(
    solution_data.n_used_resources[edge_type_idx].begin(),
    solution_data.n_used_resources[edge_type_idx].end(),
    selected_resources.selected_edge.begin());

  // Selected VMs
  selected_resources.selected_vms.resize(solution_data.n_used_resources[vm_type_idx].size());
  std::copy(
    solution_data.n_used_resources[vm_type_idx].begin(),
    solution_data.n_used_resources[vm_type_idx].end(),
    selected_resources.selected_vms.begin()
  );

  selected_resources.selected_vms_by_cl.assign(num_cls_vm, std::make_pair(false, 0));

  const auto& all_resources = system.get_system_data().get_all_resources();
  const auto& cl_name_to_idx_vm = system.get_system_data().get_cl_name_to_idx()[vm_type_idx];

  for(size_t res_idx = 0; res_idx < solution_data.n_used_resources[vm_type_idx].size(); ++res_idx)
  {
    if(solution_data.n_used_resources[vm_type_idx][res_idx] > 0)
    {
      const auto& cl_name = all_resources.get_cl_name(ResourceType::VM, res_idx);
      const size_t cl_idx = cl_name_to_idx_vm.at(cl_name);
      selected_resources.selected_vms_by_cl[cl_idx] = std::make_pair(true, res_idx);
    }
  }
}

bool
Solution::preliminary_constraints_check_assignments(
  const System& system
) const
{
  Logger::Debug("check_feasibility: Checking preliminary constraints assignments ...");
  bool feasible = true;
  const std::size_t tot_comp = system.get_system_data().get_components().size();

  // loop over components
  for(std::size_t comp_idx = 0; comp_idx < tot_comp && feasible; ++comp_idx)
  {
    feasible = feasible && preliminary_constraints_check_assignments(comp_idx, system);
  }

  Logger::Debug("check_feasibility: DONE preliminary constraints assignments ...");
  return feasible;
}

bool
Solution::preliminary_constraints_check_assignments(size_t comp_idx, const System& system) const
{
  bool feasible = true;
  const auto& compatibility_matrix = system.get_system_data().get_compatibility_matrix();
  const auto& all_resources = system.get_system_data().get_all_resources();
  const auto& components = system.get_system_data().get_components();
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

  return feasible;
}

bool
Solution::memory_constraints_check(const System& system, const LocalInfo& local_info)
{
  Logger::Debug("check_feasibility: Checking memory constraints ... ");
  bool feasible = true;
  const auto& components = system.get_system_data().get_components();
  const auto& all_resources = system.get_system_data().get_all_resources();

  for(size_t r_type_idx = 0; r_type_idx < ResIdxFromType(ResourceType::Count); ++r_type_idx)
  {
    for(size_t r_idx = 0; r_idx < memory_slack_values[r_type_idx].size(); ++r_idx)
    {
      if(!local_info.active || local_info.modified_res[r_type_idx][r_idx])
      {
        memory_slack_values[r_type_idx][r_idx] = (r_type_idx != ResIdxFromType(ResourceType::Faas)) ?
          solution_data.n_used_resources[r_type_idx][r_idx] *
          all_resources.get_memory(ResTypeFromIdx(r_type_idx), r_idx)
          :
          all_resources.get_memory(ResTypeFromIdx(r_type_idx), r_idx);
      }
    }
  }

  // check memory occupations
  for(size_t comp_idx = 0; comp_idx < components.size() && feasible; ++comp_idx)
  {
    const auto& partitions = components[comp_idx].get_partitions();

    for(auto [p_idx, r_type_idx, r_idx] : solution_data.used_resources[comp_idx])
    {
      if(!local_info.active || local_info.modified_res[r_type_idx][r_idx])
      {
        memory_slack_values[r_type_idx][r_idx] -= partitions[p_idx].get_memory(r_type_idx, r_idx);

        if(memory_slack_values[r_type_idx][r_idx] < 0)
        {
          const auto message = "Resource of type and idx: " + std::to_string(r_type_idx) + " " + std::to_string(r_idx) + " does not satisfy memory";
          Logger::Debug(message);
          feasible = false;
          break;
        }
      }
    }
  }

  Logger::Debug("check_feasibility: DONE memory constraints ... ");
  return feasible;
}

bool
Solution::move_backward_check(const System& system)
{
  Logger::Debug("check_feasibility: Checking move backward ... ");
  bool feasible = true;

  std::pair<size_t, size_t> last_edge = std::make_pair(solution_data.used_resources.size() , 0);
  std::pair<size_t, size_t> first_cloud = std::make_pair(solution_data.used_resources.size(), 0);

  const auto edge_type_idx = ResIdxFromType(ResourceType::Edge);

  for(size_t comp_idx = 0; comp_idx < solution_data.used_resources.size() && feasible; ++comp_idx)
  {
    for(size_t j=0; j<solution_data.used_resources[comp_idx].size(); ++j)
    {
      const auto [p_idx, res_type_idx, res_idx] = solution_data.used_resources[comp_idx][j];

      if(res_type_idx == edge_type_idx)
      {
        last_edge = std::make_pair(comp_idx, j);
      }
      else if(first_cloud.first == solution_data.used_resources.size())
      {
        first_cloud = std::make_pair(comp_idx, j);
      }
    }
  }
  solution_data.first_cloud = first_cloud;
  if(last_edge > first_cloud)
  {
    feasible = false;
  }
  return feasible;
}

bool
Solution::performance_assignment_check(
  const System& system,
  const LocalInfo& local_info)
{
  Logger::Debug("check_feasibility: Checking performance assignments ... ");
  const std::vector<size_t> res_type_to_check = {ResIdxFromType(ResourceType::Edge), ResIdxFromType(ResourceType::VM)};
  const auto& components = system.get_system_data().get_components();
  const auto& performance = system.get_performance();

  for(auto res_type_idx : res_type_to_check)
  {
    // loop over resources index by type
    for(size_t res_idx = 0; res_idx < system.get_system_data().get_all_resources().get_number_resources(res_type_idx); ++res_idx)
    {
      if(!local_info.active || local_info.modified_res[res_type_idx][res_idx])
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

        if(count_part == 0)
        {
          solution_data.n_used_resources[res_type_idx][res_idx] = 0;
        }
      }
    }
  }

  Logger::Debug("check_feasibility: DONE performance assignments ... ");
  return true;
}

bool
Solution::local_constraints_check(const System& system, const LocalInfo& local_info)
{
  Logger::Debug("check_feasibility: Checking local constraints ...");
  bool feasible = true;
  const auto& local_constraints = system.get_system_data().get_local_constraints();

  for(size_t i = 0; i < local_constraints.size() && feasible; ++i)
  {
    time_perfs.compute_local_perf(i, system, solution_data, local_info);

    if(isnan(time_perfs.comp_perfs[i]) || time_perfs.comp_perfs[i] > local_constraints[i].get_max_res_time())
    {
      feasible = false;
    }
  }

  Logger::Warn("check_feasibility: DONE Checking local constraints ...");
  return feasible;
}

bool
Solution::global_constraints_check(const System& system, const LocalInfo& local_info)
{
  Logger::Debug("check_feasibility: Checking global constraints ...");
  bool feasible = true;
  const auto& global_constraints = system.get_system_data().get_global_constraints();

  for(size_t i = 0; i < global_constraints.size() && feasible; ++i)
  {
    time_perfs.compute_global_perf(i, system, solution_data, local_info);

    if(isnan(time_perfs.path_perfs[i]) || time_perfs.path_perfs[i] > global_constraints[i].get_max_res_time())
    {
      feasible = false;
    }
  }

  Logger::Debug("check_feasibility: DONE global constraints ...");
  return feasible;
}

bool
Solution::check_feasibility(
  const System& system
)
{
  Logger::Debug("check_feasibility: Starting feasibility check of the solution ...");
  bool feasible = false;
  feasible = this->preliminary_constraints_check_assignments(system);

  if(feasible)
  {
    feasible = this->move_backward_check(system);

    if(feasible)
    {
      feasible = this->performance_assignment_check(system);

      if(feasible)
      {
        feasible = this->memory_constraints_check(system);

        if(feasible)
        {
          feasible = this->local_constraints_check(system);

          if(feasible)
          {
            feasible = this->global_constraints_check(system);
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
Solution::objective_function(const System& system, const LocalInfo& local_info)
{
  Logger::Debug("objective_function: Computing objective function ... ");
  this->total_cost = 0.0;
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

        if(!local_info.active || local_info.modified_res[res_type_idx][res_idx])
        {
          const TimeType time = system.get_system_data().get_time();

          if(res_type_idx == ResIdxFromType(ResourceType::Edge))
          {
            const auto res_cost = all_resources.get_resource<ResourceType::Edge>(res_idx).get_cost();
            this->res_costs[res_type_idx][res_idx] = res_cost;
            total_cost += solution_data.n_used_resources[res_type_idx][res_idx] * res_cost;
          }
          else if(res_type_idx == ResIdxFromType(ResourceType::VM))
          {
            const auto res_cost = all_resources.get_resource<ResourceType::VM>(res_idx).get_cost();
            this->res_costs[res_type_idx][res_idx] = res_cost;
            total_cost += solution_data.n_used_resources[res_type_idx][res_idx] * res_cost * time;
          }
          else // Faas
          {
            const auto res_cost = all_resources.get_resource<ResourceType::Faas>(res_idx).get_cost();
            // ATTENTO: LORO USANO COMP_LAMBDA... SECONDO ME E' GIUSTO PART_LAMBDA INVECE
            const auto part_lambda = components[comp_idx].get_partition(part_idx).get_part_lambda();
            const auto warm_time =
              static_cast<FaasPE*>(
                performance[comp_idx][res_type_idx][part_idx][res_idx].get()
              )->get_demandWarm();
            this->res_costs[res_type_idx][res_idx] = res_cost * warm_time * part_lambda * time;
            total_cost += this->res_costs[res_type_idx][res_idx];
          }
        }
        else
        {
          total_cost += res_costs[res_type_idx][res_idx];
        }
      }
    }
  }

  Logger::Debug("objective_function: Done computation of objective function!");
  return total_cost;
}


} // namespace Space4AI
