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
  bool reproducibility_)
  : best_sol(init_sol_), system(system_), curr_sol(Solution(system_)),
    reproducibility(reproducibility_)
{
  const auto& all_resources = system.get_system_data().get_all_resources();

  local_info.active = true;

  local_info.modified_res.resize(ResIdxFromType(ResourceType::Count));
  for(size_t i=0; i<local_info.modified_res.size(); ++i)
  {
    local_info.modified_res[i].resize(all_resources.get_number_resources(i));
  }

  if(reproducibility)
  {
    rng.seed(seed);
  }
  else
  {
    std::random_device dev;
    rng.seed(dev());
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
    drop_resource();
  }

  return curr_sol;
}

void
LocalSearch::migrate_vm_to_edge()
{
  // Choose a random component
  const auto& components = system.get_system_data().get_components();
  std::uniform_int_distribution<decltype(rng)::result_type> dist(0, components.size()-1);
  const size_t comp_idx = dist(rng);

  const auto& used_resources_comp = best_sol.get_used_resources()[comp_idx];
  const auto& selected_edge = best_sol.selected_resources.get_selected_edge();
  const auto edge_type_idx = ResIdxFromType(ResourceType::Edge);

  bool interrupt{false};
  // migrate FIRST (and only first) VM to Edge
  for(size_t i=0; i<used_resources_comp.size() && !interrupt; ++i)
  {
    const auto [p_idx, r_type_idx, r_idx] = used_resources_comp[i];
    if(r_type_idx == ResIdxFromType(ResourceType::Faas)) // If I find a Faas, I can't migrate subsequent vm to edge
    {
      interrupt = true;
    }
    else if(r_type_idx == ResIdxFromType(ResourceType::VM))
    {
      if(migration_tweaking(
          comp_idx, p_idx, i, r_type_idx, r_idx, edge_type_idx, selected_edge))
      {
        ++vm_to_edge_count;
        curr_sol.set_selected_resources(system); // VM can be switched off
      }
      interrupt = true;
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
  const auto& selected_vms = best_sol.selected_resources.get_selected_vms();

  // migrating ALL (I repeat, ALL) possible FaaS executed on comp_idx!
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

    curr_sol.solution_data.y_hat[comp_idx][res_type_idx_old][part_idx][res_idx_old] = 0;
    curr_sol.solution_data.y_hat[comp_idx][res_type_idx_new][part_idx][random_resource] =
      curr_sol.solution_data.n_used_resources[res_type_idx_new][random_resource];
    curr_sol.solution_data.used_resources[comp_idx][part_pos_idx] =
      std::make_tuple(part_idx, res_type_idx_new, random_resource);

    // uploading local_info
    local_info.reset();
    local_info.active = true;

    local_info.modified_res[res_type_idx_old][res_idx_old] = true;
    local_info.modified_res[res_type_idx_new][random_resource] = true;

    local_info.modified_comp = std::make_pair(true, comp_idx);

    local_info.old_local_parts_perfs_ptr = &(best_sol.time_perfs.local_parts_perfs);
    local_info.old_local_parts_delays_ptr = &(best_sol.time_perfs.local_parts_delays);

    // CONSTRAINTS
    #warning I AM NOT CHECKING ALLOW_COLOCATION. ASK ARDAGNA
    #warning AT THE MOMENT I AM CHECKING MEMORY FROM SCRATCH SINCE THE CODE'S GOING TO VARY SOON
    // memory
    feasible =
      curr_sol.memory_constraints_check(system, local_info) &&
      curr_sol.performance_assignment_check(system, local_info) &&
      curr_sol.local_constraints_check(system, local_info) &&
      curr_sol.global_constraints_check(system, local_info);
  }
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
  local_info.old_local_parts_perfs_ptr = &(best_sol.time_perfs.local_parts_perfs);
  local_info.old_local_parts_delays_ptr = &(best_sol.time_perfs.local_parts_delays);

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
      candidate_resources[i] = best_sol.selected_resources.get_selected_edge();
    }
    if(i == vm_type_idx)
    {
      candidate_resources[i] = best_sol.selected_resources.get_selected_vms();
    }
    if(i == faas_type_idx)
    {
      const size_t n_res = all_resources.get_number_resources(i);
      candidate_resources[i].resize(n_res, true); // select all the faas as candidates
    }
  }

  for(size_t part_idx : random_dep.get_partition_indices())
  {
    const auto random_resource = sample_random_resource(
      comp_idx, part_idx, candidate_resources, compatibility_matrix);

    if(random_resource.first == res_type_idx_count)
    {
      curr_sol = best_sol;
      return;
    }
    else if(random_resource.first == faas_type_idx)
    {
      curr_sol.solution_data.y_hat[comp_idx][random_resource.first][part_idx][random_resource.second] = 1;
    }
    else // Edge or VM
    {
      curr_sol.solution_data.y_hat[comp_idx][random_resource.first][part_idx][random_resource.second] =
        curr_sol.solution_data.n_used_resources[random_resource.first][random_resource.second];
    }
    curr_sol.solution_data.used_resources[comp_idx].emplace_back(part_idx, random_resource.first, random_resource.second);

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

  if(feasible && (curr_sol.objective_function(system) < best_sol.get_cost()))
  {
      best_sol = curr_sol;
      ++change_deployment_count;
  }
  else
  {
    curr_sol = best_sol;
  }
}

void
LocalSearch::drop_resource()
{
  const auto edge_type_idx = ResIdxFromType(ResourceType::Edge);
  const auto vm_type_idx = ResIdxFromType(ResourceType::VM);
  const auto faas_type_idx = ResIdxFromType(ResourceType::Faas);
  const auto res_type_idx_count = ResIdxFromType(ResourceType::Count);

  local_info.reset();
  local_info.active = true;
  local_info.modified_comp = std::make_pair(false, 0);
  local_info.old_local_parts_perfs_ptr = &(best_sol.time_perfs.local_parts_perfs);
  local_info.old_local_parts_delays_ptr = &(best_sol.time_perfs.local_parts_delays);

  const auto del_res = find_resource_to_drop();
  if(del_res.first == res_type_idx_count)
  {
    return; // no resource to drop found!
  }
  curr_sol.solution_data.n_used_resources[del_res.first][del_res.second] = 0;

  // select new resources for the new deployment
  std::vector<std::vector<bool>> candidate_resources(res_type_idx_count);
  for(size_t i = 0; i < res_type_idx_count; ++i)
  {
    if(i == edge_type_idx)
    {
      candidate_resources[i] = best_sol.selected_resources.get_selected_edge();
    }
    if(i == vm_type_idx)
    {
      candidate_resources[i] = best_sol.selected_resources.get_selected_vms();
    }
    if(i == faas_type_idx)
    {
      const size_t n_res = system.get_system_data().get_all_resources().get_number_resources(i);
      candidate_resources[i].resize(n_res, true); // select all the faas as candidates
    }
  }
  candidate_resources[del_res.first][del_res.second] = false; // remove the deleted resource from candidates
  local_info.modified_res[del_res.first][del_res.second] = true; // ACTUALLY NOT NEEDED SINCE I AM DELETING THE RES!

  const auto& compatibility_matrix = system.get_system_data().get_compatibility_matrix();

  for(size_t comp_idx=0; comp_idx < best_sol.solution_data.used_resources.size(); ++comp_idx)
  {
    const auto& used_resources_comp = best_sol.solution_data.used_resources[comp_idx];

    for(size_t i=0; i < used_resources_comp.size(); ++i)
    {
      const auto [part_idx, res_type_idx, res_idx] = used_resources_comp[i];

      if(res_type_idx == del_res.first && res_idx == del_res.second)
      {
        curr_sol.solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] = 0;

        const auto random_resource = sample_random_resource(
          comp_idx, part_idx, candidate_resources, compatibility_matrix);

        if(random_resource.first == res_type_idx_count)
        {
          curr_sol = best_sol;
          return;
        }
        else if(random_resource.first == res_type_idx_count)
        {
          curr_sol = best_sol;
          return;
        }
        else if(random_resource.first == faas_type_idx)
        {
          curr_sol.solution_data.y_hat[comp_idx][random_resource.first][part_idx][random_resource.second] = 1;
        }
        else // Edge or VM
        {
          curr_sol.solution_data.y_hat[comp_idx][random_resource.first][part_idx][random_resource.second] =
            curr_sol.solution_data.n_used_resources[random_resource.first][random_resource.second];
        }
        curr_sol.solution_data.used_resources[comp_idx][i] =
          std::make_tuple(part_idx, random_resource.first, random_resource.second);

        local_info.modified_res[random_resource.first][random_resource.second] = true;
      }
    }
  }

  // check_constraints
  bool feasible = true;

  #warning performance_assignment_check is not efficient. Better to save the number of partitions \
  running on each resources...
  feasible =
    curr_sol.move_backward_check(system) &&
    curr_sol.memory_constraints_check(system, local_info) &&
    curr_sol.performance_assignment_check(system, local_info) &&
    curr_sol.local_constraints_check(system, local_info) &&
    curr_sol.global_constraints_check(system, local_info);

  if(feasible && (curr_sol.objective_function(system) < best_sol.get_cost()))
  {
      curr_sol.set_selected_resources(system);
      best_sol = curr_sol;
      ++drop_resource_count;

      std::cout << "NON HO PERSO TEMPO" << std::endl;
  }
  else
  {
    curr_sol = best_sol;
  }
}

std::pair<size_t, size_t>
LocalSearch::sample_random_resource(
  size_t comp_idx, size_t part_idx,
  const std::vector<std::vector<bool>>& candidate_resources,
  const CompatibilityMatrixType& compatibility_matrix)
{
  const auto faas_type_idx = ResIdxFromType(ResourceType::Faas);
  const size_t res_type_idx_count = ResIdxFromType(ResourceType::Count);

  std::vector<std::pair<size_t, size_t>> resources_instersection;
  resources_instersection.reserve(compatibility_matrix[comp_idx][faas_type_idx][0].size()); // just to reserve enough...

  // Edge and VM
  for(size_t res_type_idx = 0; res_type_idx < res_type_idx_count; ++res_type_idx)
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
  const size_t n_inter_res = resources_instersection.size();
  if(n_inter_res == 0)
  {
    return std::make_pair(res_type_idx_count, 0);
  }
  std::uniform_int_distribution<decltype(rng)::result_type> dist(0, n_inter_res - 1);
  const size_t random_idx = dist(rng);
  return resources_instersection[random_idx];
}

std::pair<size_t, size_t>
LocalSearch::find_resource_to_drop()
{
  std::vector<std::pair<size_t, size_t>> active_res;

  const auto edge_type_idx = ResIdxFromType(ResourceType::Edge);
  const auto vm_type_idx = ResIdxFromType(ResourceType::VM);
  const auto faas_type_idx = ResIdxFromType(ResourceType::Faas);
  const auto res_type_idx_count = ResIdxFromType(ResourceType::Count);

  const auto& selected_edge = best_sol.selected_resources.get_selected_edge();
  const auto& selected_vms = best_sol.selected_resources.get_selected_vms();

  const auto& dt_selected_edge = this->selected_resources.get_selected_edge();

  if(dt_selected_edge.size() == 0) // If am at DT I can drop Edge!
  {
    for(size_t i=0; i<selected_edge.size(); ++i)
    {
      if(selected_edge[i])
      {
        active_res.emplace_back(edge_type_idx,i);
      }
    }
  }
  for(size_t i=0; i<selected_vms.size(); ++i)
  {
    if(selected_vms[i])
    {
      active_res.emplace_back(vm_type_idx,i);
    }
  }
  const size_t n_inter_res = active_res.size();
  if(n_inter_res == 0)
  {
    return std::make_pair(res_type_idx_count, 0);
  }
  std::uniform_int_distribution<decltype(rng)::result_type> dist(0, n_inter_res-1);
  const size_t rand_res_idx = dist(rng);
  return active_res[rand_res_idx];
}


// void
// LocalSearch::change_resource()
// {
//   const auto& all_resources = system.get_system_data().get_all_resources();
//   const auto& cl_name_to_idx = system.get_system_data().get_cl_name_to_idx();
//   const auto& cls = system.get_system_data().get_cls();
//
//   const auto edge_type_idx = ResIdxFromType(ResourceType::Edge);
//   const auto vm_type_idx = ResIdxFromType(ResourceType::VM);
//   const auto faas_type_idx = ResIdxFromType(ResourceType::Faas);
//   const auto res_type_idx_count = ResIdxFromType(ResourceType::Count);
//
//   local_info.reset();
//   local_info.active = true;
//   local_info.modified_comp = std::make_pair(false, 0);
//   local_info.old_local_parts_perfs_ptr = &(best_sol.time_perfs.local_parts_perfs);
//   local_info.old_local_parts_delays_ptr = &(best_sol.time_perfs.local_parts_delays);
//
//   const auto del_res = find_resource_to_drop();
//   if(del_res.first == res_type_idx_count)
//   {
//     return; // no resource to change found!
//   }
//   curr_sol.solution_data.n_used_resources[del_res.first][del_res.second] = 0;
//
//   // find layer of the resource to drop
//   //
//   const auto cl_name = all_resources.get_cl_name(ResTypeFromIdx(del_res.first), del_res.second);
//   // populate candidate resurces with what already selected and what selectable
//   std::vector<size_t> altern_resources;
//   altern_resource.reserve(all_resources.get_number_resources(del_res.first));
//   std::vector<bool> already_selected_cls(cls[del_res.first].size());
//
//   if(del_res.first == edge_type_idx)
//   {
//     const auto& selected_edge = best_sol.selected_resources.get_selected_edge();
//     for(size_t i=0; i<selected_edge.size(); ++i)
//     {
//       if(selected_edge[i] && i != del_res.second)
//       {
//         const auto cl_name = all_resources.get_cl_name(edge_type_idx, i);
//         already_selected_cls[cl_name_to_idx.at(cl_name)] = true;
//         altern_resource.push_back(i);
//       }
//     }
//   }
//   else // VM
//   {
//     const auto& selected_vms = best_sol.selected_resources.get_selected_vms();
//     candidate_resources = selected_vms;
//     for(size_t i=0; i<selected_vms.size(); ++i)
//     {
//       if(selected_vms[i] && i != del_res.second)
//       {
//         const auto cl_name = all_resources.get_cl_name(vm_type_idx, i);
//         already_selected_cls[cl_name_to_idx.at(cl_name)] = true;
//         altern_resource.push_back(i);
//       }
//     }
//   }
//
//   // populate candidate_resources by comp layer
//   for(size_t cl_idx=0; cl_idx < cls[del_res.first].size(); ++cl_idx)
//   {
//     if(!already_selected_cls[cl_idx])
//     {
//       //pick a random resource of the cl
//       const std::vector<size_t> res_idxs = cls[vm_type_idx][cl_idx].get_res_idxs();
//       const size_t n_res = res_idxs.size();
//       std::uniform_int_distribution<decltype(rng)::result_type> dist(0, n_res - 1);
//       const size_t random_res_idx = res_idxs[dist(rng)];
//       altern_resource.push_back(random_res_idx);
//     }
//   }
//
//   const auto num_altern_res = altern_resource.size();
//   const size_t count = 0;
//   std::uniform_int_distribution<decltype(rng)::result_type> dist(0, num_altern_res-1);
//   size_t idx = dist(rng);
//   bool found_new_res = false;
//
//   const CostType orig_cost =
//     best_sol.solution_data.n_used_resources[del_res.first][del_res.second] *
//     all_resources.get_cost(ResTypeFromIdx(del_res.first), del_res.second);
//
//   while(count<num_altern_res && !found_new_res)
//   {
//     const CostType new_single_cost = all_resources.get_cost(ResTypeFromIdx(del_res.first), altern_resource[idx]);
//     const size_t inc_clust_size = static_cast<size_t>(orig_cost / new_single_cost);
//
//     if(best_sol.solution_data.n_used_resources[del_res.first][altern_resource[idx]] > 0 || inc_clust_size > 0)
//     {
//       found_new_res = true;
//       const size_t max_num_istance_avail = all_resources.get_number_avail(ResTypeFromIdx(del_res.first), altern_resource[idx]);
//       const size_t new_clust_size =
//        (best_sol.solution_data.n_used_resources[del_res.first][altern_resource[idx]] + inc_clust_size) <= max_num_istance_avail ?
//        (best_sol.solution_data.n_used_resources[del_res.first][altern_resource[idx]] + inc_clust_size)
//        :
//        max_num_istance_avail;
//
//       curr_sol.solution_data.n_used_resources[del_res.first][altern_resource[idx]] += new_clust_size;
//     }
//     else
//     {
//       idx = (idx+1) % num_altern_res;
//       ++count;
//     }
//   }
//   if(!found_new_res)
//   {
//     curr_sol = best_sol;
//     return;
//   }
//   // assign partitions of old component to new component
//   bool feasible = true;
//
//   const auto& compatibility_matrix = system.get_system_data().get_compatibility_matrix();
//
//   for(size_t comp_idx=0; comp_idx < best_sol.solution_data.used_resources.size() && feasible; ++comp_idx)
//   {
//     const auto& used_resources_comp = best_sol.solution_data.used_resources[comp_idx];
//
//     for(size_t i=0; i < used_resources_comp.size() && feasible; ++i)
//     {
//       const auto [part_idx, res_type_idx, res_idx] = used_resources_comp[i];
//
//       if(res_type_idx == del_res.first && res_idx == del_res.second)
//       {
//         curr_sol.solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] = 0;
//         if(compatibility_matrix[comp_idx][res_type_idx][part_idx][altern_resource[idx]])
//         {
//           curr_sol.solution_data.y_hat[comp_idx][res_type_idx][part_idx][altern_resource[idx]] =
//             curr_sol.solution_data.n_used_resources[res_type_idx][altern_resource[idx]];
//
//           curr_sol.solution_data.used_resources[comp_idx][i] =
//             std::make_tuple(part_idx, res_type_idx, altern_resource[idx]);
//         }
//         else
//         {
//           feasible = false;
//         }
//       }
//     }
//   }
//
//   // check constraints
//
//
//
//   /*
//   // find new resource on that layer
//   const auto& all_res_idxs = cls[del_res.first][cl_name_to_idx.at(cl_name)].get_res_idxs();
//   if(all_res_idxs.size() <= 1)
//   {
//     return; // cannot change resource if I have no or only one resource on the layer.
//   }
//   std::uniform_int_distribution<decltype(rng)::result_type> dist(0, all_res_idxs.size()-1);
//   size_t rand_i = dist(rng);
//   if(all_res_idxs[rand_i] == del_res.second)
//   {
//     rand_i = (rand_i + 1) % all_res_idxs.size();
//   }
//   const size_t new_res_idx = all_res_idxs[rand_i];
//
//   const auto orig_tot_cost =
//     best_sol.solution_data.n_used_resources[del_res.first][del_res.second] *
//     all_resources.get_cost(ResTypeFromIdx(del_res.first), del_res.second);
//
//   const auto new_cost = all_resources.get_cost(ResTypeFromIdx(del_res.first), new_res_idx);
//   size_t clust_size = static_cast<size_t>(orig_tot_cost / new_cost);
//   if(clust_size < 1)
//   {
//     return; // new cost is too high
//   }
//   local_info.modified_res[del_res.first][del_res.second] = true;
//   curr_sol.solution_data.n_used_resources[del_res.first][del_res.second] = 0;
//
//   local_info.modified_res[del_res.first][new_res_idx] = true;
//   curr_sol.solution_data.n_used_resources[del_res.first][new_res_idx] = clust_size;
//
//   for(size_t comp_idx=0; comp_idx < best_sol.solution_data.used_resources.size(); ++comp_idx)
//   {
//     const auto& used_resources_comp = best_sol.solution_data.used_resources[comp_idx];
//
//     for(size_t i=0; i<used_resources_comp.size(); ++i)
//     {
//       const auto [part_idx, res_type_idx, res_idx] = used_resources_comp[i];
//
//       if(res_type_idx == del_res.first && res_idx == del_res.second)
//       {
//         curr_sol.solution_data.y_hat[comp_idx][res_type_idx][part_idx][res_idx] = 0;
//         curr_sol.solution_data.y_hat[comp_idx][res_type_idx][part_idx][new_res_idx] = clust_size;
//
//         curr_sol.solution_data.used_resources[comp_idx][i] =
//           std::make_tuple(part_idx, del_res.first, new_res_idx);
//       }
//     }
//   }
//
//   // check_constraints
//   bool feasible = true;
//
//   #warning performance_assignment_check is not efficient. Better to save the number of partitions \
//   running on each resources...
//   feasible =
//     curr_sol.move_backward_check(system) &&
//     curr_sol.memory_constraints_check(system) &&
//     curr_sol.performance_assignment_check(system, local_info) &&
//     curr_sol.local_constraints_check(system, local_info) &&
//     curr_sol.global_constraints_check(system, local_info);
//
//   if(feasible && (curr_sol.objective_function(system) < best_sol.get_cost()))
//   {
//       curr_sol.set_selected_resources(system);
//       best_sol = curr_sol;
//       ++drop_resource_count;
//
//       std::cout << "NON HO PERSO TEMPO" << std::endl;
//   }
//   else
//   {
//     curr_sol = best_sol;
//   }
//   */
// }


} // namespace Space4AI