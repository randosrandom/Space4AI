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
* \file LocalSearch.cpp
*
* \brief Defines the LocalSearch class methods
*
* \author Randeep Singh
*/

#include "src/Algorithms/LocalSearch.hpp"

namespace Space4AI
{

LocalSearch::LocalSearch(
  const Solution& init_sol_,
  const System& system_,
  const SelectedResources& selected_resources_)
  : best_sol(init_sol_), system(system_), selected_resources(selected_resources_),
    curr_sol(Solution(system_))
{
  const auto& all_resources = system.get_system_data().get_all_resources();

  local_info.active = true;

  local_info.modified_res.resize(ResIdxFromType(ResourceType::Count));
  for(size_t i=0; i<local_info.modified_res.size(); ++i)
  {
    local_info.modified_res[i].resize(all_resources.get_number_resources(i));
  }
}

Solution
LocalSearch::run(size_t max_it)
{
  curr_sol = best_sol;

  for(size_t it=0; it<max_it; ++it)
  {
    migrate_vm_to_edge();
    migrate_faas_to_vm();
    change_deployment();
  }

  return std::move(curr_sol);
}

void
LocalSearch::migrate_vm_to_edge()
{
  // Choose a random component
  const auto& components = system.get_system_data().get_components();
  std::uniform_int_distribution<decltype(rng)::result_type> dist(0, components.size()-1);
  const size_t comp_idx = dist(rng);

  const auto& used_resources_comp = best_sol.get_used_resources()[comp_idx];
  const auto& selected_edge = selected_resources.get_selected_edge();
  const auto edge_type_idx = ResIdxFromType(ResourceType::Edge);

  // Find the first part being executed on VM
  for(size_t i=0; i<used_resources_comp.size(); ++i)
  {
    const auto [p_idx, r_type_idx, r_idx] = used_resources_comp[i];
    if(r_type_idx == ResIdxFromType(ResourceType::Faas)) // If I find a Faas, I can't migrate subsequent vm to edge
    {
      return;
    }
    if(r_type_idx == ResIdxFromType(ResourceType::VM))
    {
      if(migration_tweaking(
          comp_idx, p_idx, i, r_type_idx, r_idx, edge_type_idx, selected_edge))
      {
        ++vm_to_edge_count;
      }
    }
  }
}

void
LocalSearch::migrate_faas_to_vm()
{
  // Choose a random component
  const auto& components = system.get_system_data().get_components();

  std::uniform_int_distribution<decltype(rng)::result_type> dist(0, components.size()-1);
  const size_t comp_idx = dist(rng);

  const auto& used_resources_comp = best_sol.get_used_resources()[comp_idx];

  const size_t vm_type_idx = ResIdxFromType(ResourceType::VM);
  const auto& selected_vms = selected_resources.get_selected_vms();

  // Find the first part being executed on Faas
  for(size_t i=0; i<used_resources_comp.size(); ++i)
  {
    const auto [p_idx, r_type_idx, r_idx] = used_resources_comp[i];

    if(r_type_idx == ResIdxFromType(ResourceType::Faas))
    {
      if(migration_tweaking(
          comp_idx, p_idx, i, r_type_idx, r_idx, vm_type_idx, selected_vms))
      {
        ++faas_to_vm_count;
      }
    }
  }
}

bool
LocalSearch::migration_tweaking(
  size_t comp_idx,
  size_t part_idx,
  size_t part_pos_idx,
  size_t res_type_idx_old,
  size_t res_idx_old,
  size_t res_type_idx_new,
  const std::vector<bool>& selected_devices)
{
  bool feasible{false};

  const auto compatibility_matrix = system.get_system_data().get_compatibility_matrix();

  std::vector<size_t> resources_instersection;
  resources_instersection.reserve(selected_devices.size());
  for(size_t j=0; j<selected_devices.size(); ++j)
  {
    if(selected_devices[j] && compatibility_matrix[comp_idx][res_type_idx_new][part_idx][j])
    {
      resources_instersection.push_back(j);
    }
  }
  if(resources_instersection.size() > 0)
  {
    const size_t n_inter_res = resources_instersection.size();
    std::uniform_int_distribution<decltype(rng)::result_type> dist(0, n_inter_res - 1);
    const size_t random_idx = dist(rng);
    const size_t random_resource = resources_instersection[random_idx];

    curr_sol.solution_data.y_hat[comp_idx][res_type_idx_new][part_idx][random_resource] =
      curr_sol.solution_data.n_used_resources[res_type_idx_new][random_resource];
    curr_sol.solution_data.y_hat[comp_idx][res_type_idx_old][part_idx][res_idx_old] = 0;
    curr_sol.solution_data.used_resources[comp_idx][part_pos_idx] =
      std::make_tuple(part_idx, res_type_idx_new, random_resource);

    // uploading local_info
    local_info.reset();
    local_info.active = true;

    local_info.modified_res[res_type_idx_old][res_idx_old] = true;
    local_info.modified_res[res_type_idx_new][random_resource] = true;

    local_info.modified_comp = std::make_pair(true, comp_idx);
    local_info.modified_single_part = std::make_pair(true, part_pos_idx);
    local_info.old_used_resources_comp_ptr = &(best_sol.solution_data.used_resources[comp_idx]);

    // CONSTRAINTS
    #warning I AM NOT CHECKING ALLOW_COLOCATION. ASK ARDAGNA
    #warning AT THE MOMENT I AM CHECKING MEMORY FROM SCRATCH SINCE THE CODE'S GOING TO VARY SOON
    // memory
    feasible =
      curr_sol.memory_constraints_check(system) &&
      curr_sol.local_constraints_check(system, local_info) &&
      curr_sol.global_constraints_check(system, local_info);
    if(!feasible)
    {
      curr_sol = best_sol; // reset solution
    }
    else // assigning curr_sol to best sol even though cost will be similar
    {
      # warning Inefficiency here: Should manage cost by single resource. However, for the resources \
      whith colocation, I should keep track of the number of partitions running on the resource. \
      Maybe change n_used_resources, adding a std::pair where the second element is the number of partition running.

      curr_sol.objective_function(system);
      best_sol = curr_sol;
    }
  }
  return feasible;
}

void
LocalSearch::change_deployment()
{
  // Choose a random component
  const auto& components = system.get_system_data().get_components();
  std::uniform_int_distribution<decltype(rng)::result_type> dist(0, components.size()-1);
  const size_t comp_idx = dist(rng);
  const auto& used_resources_comp_old = best_sol.get_used_resources()[comp_idx];

  local_info.reset();
  local_info.active = true;
  local_info.modified_comp = std::make_pair(true, comp_idx);
  local_info.modified_single_part = std::make_pair(false, 0);
  local_info.old_used_resources_comp_ptr = &(used_resources_comp_old);

  // Choose random deployment
  const auto& deployments = components[comp_idx].get_deployments();
  const size_t n_dep = deployments.size();
  std::uniform_int_distribution<decltype(rng)::result_type> distr(0, n_dep - 1);
  size_t random_dep_idx = distr(rng);
  const auto& random_dep_temp = deployments[random_dep_idx];
  //// check whether the deployment is different for the current one
  const auto& random_dep_parts_temp = random_dep_temp.get_partition_indices();
  if(random_dep_parts_temp.front() == std::get<0>(used_resources_comp_old.front())) // same deployment
  {
    random_dep_idx = (random_dep_idx + 1) % n_dep;
  }
  const auto& random_dep = deployments[random_dep_idx];

  // get useful things
  const size_t edge_type_idx = ResIdxFromType(ResourceType::Edge);
  const size_t vm_type_idx = ResIdxFromType(ResourceType::VM);
  const auto faas_type_idx = ResIdxFromType(ResourceType::Faas);
  const size_t res_type_idx_count = ResIdxFromType(ResourceType::Count);
  const auto& compatibility_matrix = system.get_system_data().get_compatibility_matrix();
  const auto& all_resources = system.get_system_data().get_all_resources();

  // clear current solution comp_idx used_resources
  for(auto [p_idx, r_type_idx, r_idx]: used_resources_comp_old)
  {
    curr_sol.solution_data.y_hat[comp_idx][r_type_idx][p_idx][r_idx] = 0;
    local_info.modified_res[r_type_idx][r_idx] = true;
  }
  curr_sol.solution_data.used_resources[comp_idx].clear();

  // select new resources for the new deployment
  std::vector<std::vector<bool>> candidate_resources(res_type_idx_count);
  for(size_t i = 0; i < res_type_idx_count; ++i)
  {
    if(i == edge_type_idx)
    {
      candidate_resources[i] = selected_resources.get_selected_edge();
    }
    if(i == vm_type_idx)
    {
      candidate_resources[i] = selected_resources.get_selected_vms();
    }
    if(i == faas_type_idx)
    {
      const size_t n_res = all_resources.get_number_resources(i);
      candidate_resources[i].resize(n_res, true); // select all the faas as candidates
    }
  }

  for(size_t part_idx : random_dep.get_partition_indices())
  {
    std::vector<std::pair<size_t, size_t>> resources_instersection;
    resources_instersection.reserve(compatibility_matrix[comp_idx][faas_type_idx][0].size());

    // Edge and VM
    for(size_t res_type_idx = 0; res_type_idx < res_type_idx_count-1; ++res_type_idx) // Edge or VM
    {
      //loop over the resources of type j
      for(size_t res_idx = 0; res_idx < candidate_resources[res_type_idx].size(); ++res_idx)
      {
        //compute the intersection
        if(candidate_resources[res_type_idx][res_idx] && compatibility_matrix[comp_idx][res_type_idx][part_idx][res_idx])
        {
          resources_instersection.emplace_back(res_type_idx, res_idx);
        }
      }
    }
    //Faas
    const float prob = resources_instersection.size()>0 ? 0.5 : 1;
    std::bernoulli_distribution be(prob);
    for(size_t res_idx = 0; res_idx < candidate_resources[faas_type_idx].size(); ++res_idx)
    {
      if(be(rng) && candidate_resources[faas_type_idx][res_idx] && compatibility_matrix[comp_idx][faas_type_idx][part_idx][res_idx])
        {
          resources_instersection.emplace_back(faas_type_idx, res_idx);
        }
    }
    const size_t n_inter_res = resources_instersection.size();
    std::uniform_int_distribution<decltype(rng)::result_type> dist(0, n_inter_res - 1);
    const size_t random_idx = dist(rng);
    const auto& random_resource = resources_instersection[random_idx];

    curr_sol.solution_data.used_resources[comp_idx].emplace_back(part_idx, random_resource.first, random_resource.second);

    if(random_resource.first == faas_type_idx)
    {
      curr_sol.solution_data.y_hat[comp_idx][random_resource.first][part_idx][random_resource.second] = 1;
    }
    else // Edge or VM
    {
      curr_sol.solution_data.y_hat[comp_idx][random_resource.first][part_idx][random_resource.second] =
        curr_sol.solution_data.n_used_resources[random_resource.first][random_resource.second];
    }

    local_info.modified_res[random_resource.first][random_resource.second] = true;
  }

  // check constraints
  bool feasible = true;

  #warning performance_assignment_check is not efficient. Better to save the number of partitions \
  running on each resources...

  feasible =
    curr_sol.move_backward_check(comp_idx) &&
    curr_sol.memory_constraints_check(system) &&
    curr_sol.performance_assignment_check(system, local_info) &&
    curr_sol.local_constraints_check(system, local_info) &&
    curr_sol.global_constraints_check(system, local_info);

  if(feasible && curr_sol.objective_function(system) < best_sol.get_cost())
  {
      best_sol = curr_sol;
      ++change_deployment_count;
  }
  else
  {
    curr_sol = best_sol;
  }
}

} // namespace Space4AI
