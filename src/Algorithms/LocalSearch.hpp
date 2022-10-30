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
* \file LocalSearch.hpp
*
* \brief Defines the LocalSearch algorithm
*
* \author Randeep Singh
*/

#ifndef LOCAL_SEARCH_HPP_
#define LOCAL_SEARCH_HPP_

#include <random>

#include "src/Solution/Solution.hpp"
#include "src/System/System.hpp"

namespace Space4AI
{

/** Local Search algortithm.
*   Take a solution and explore its neighborhoods to find the local optimum.
*/
class LocalSearch
{
  friend class LocalSearchManager;

public:

  LocalSearch(
    System const * const system_,
    SelectedResources const * const fixed_edge_and_curr_rt_vms_):
  best_sol(Solution(nullptr)), curr_sol(Solution(nullptr)),
  system(system_), fixed_edge_and_curr_rt_vms(fixed_edge_and_curr_rt_vms_) {};

  LocalSearch(
    const Solution& init_sol_,
    System const * const system_,
    SelectedResources const * const fixed_edge_and_curr_rt_vms_);

  void
  run(size_t max_it, bool reproducibility);

  const Solution&
  get_best_sol() const {return best_sol;}

protected:

  void
  migrate_first_cloud_to_edge();

  void
  migrate_faas_to_vm();

  void
  migrate_faas_to_faas();

  void
  change_deployment();

  void
  drop_resource();

  void
  change_resource();

  bool
  migration_tweaking(
    size_t comp_idx,
    size_t part_idx,
    size_t part_pos_idx,
    size_t res_type_idx_old,
    size_t res_idx_old,
    size_t res_type_idx_new,
    const std::vector<bool>& selected_devices);

  std::pair<size_t, size_t>
  sample_random_resource(
    size_t comp_idx, size_t part_idx,
    const std::vector<std::vector<bool>>& candidate_resources,
    const CompatibilityMatrixType& compatibility_matrix);

  std::pair<size_t, size_t>
  find_resource_to_drop();

  void
  reduce_cluster_size(size_t res_type_idx, size_t res_idx);

protected:

  /** initial solution passed to the LS algorithm */
  Solution best_sol;

  /** current best solution */
  Solution curr_sol;

  /** System object */
  System const * system;

  /** Curr run-time sol selcted resources */
  SelectedResources const * fixed_edge_and_curr_rt_vms;

  /** local info to track modifications od Local Search */
  LocalInfo local_info;

  /** Random number generator */
  std::mt19937_64 rng;
  /** seed */
  inline static constexpr size_t seed = 20122022;

  // counter of feasible neighborhoods
  /** counter of VM to Edge migrations */
  size_t first_cloud_to_edge_count = 0;
  /** counter of Faas to VM migrations */
  size_t faas_to_vm_count = 0;
  /** counter of mifration faas to faas */
  size_t faas_to_faas_count = 0;
  /** counter of change deployments migrations */
  size_t change_deployment_count = 0;
  /** counter of drop resource */
  size_t drop_resource_count = 0;
  /** counter of change_resource_by_cl */
  size_t change_resource_count = 0;
};

} // namespace Space4AI

#endif // LOCAL_SEARCH_HPP_
