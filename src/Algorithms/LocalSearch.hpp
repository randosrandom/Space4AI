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

#include "src/System/System.hpp"
#include "src/Solution/Solution.hpp"

namespace Space4AI
{

/** Local Search algortithm.
*   Take a solution and explore its neighborhoods to find the local optimum.
*/
class LocalSearch
{

public:

  LocalSearch(
    const Solution& init_sol_,
    const System& system_,
    const SelectedResources& selected_resources_);

  Solution
  run(size_t max_it);

protected:

  void
  migrate_vm_to_edge();

  void
  migrate_faas_to_vm();

  void
  change_deployment();

  void
  reduce_vm_size();

  bool
  migration_tweaking(
    size_t comp_idx,
    size_t part_idx,
    size_t part_pos_idx,
    size_t res_type_idx_old,
    size_t res_idx_old,
    size_t res_type_idx_new,
    const std::vector<bool>& selected_devices);

protected:

  /** initial solution passed to the LS algorithm */
  Solution best_sol;

  /** System object */
  const System& system;

  /** Selected resources */
  const SelectedResources& selected_resources;

  /** current best solution */
  Solution curr_sol;

  LocalInfo local_info;

  /** Random number generator */
  std::mt19937_64 rng;
  /** seed */
  const size_t seed = 121298;
  /** reproducibility  flag */
  bool reproducibility = true;

  // counter of feasible neighborhoods
  /** counter of VM to Edge migrations */
  size_t vm_to_edge_count = 0;
  /** counter of Faas to VM migrations */
  size_t faas_to_vm_count = 0;
  /** counter of change deployments migrations */
  size_t change_deployment_count = 0;
};

} // namespace Space4AI

#endif // LOCAL_SEARCH_HPP_
