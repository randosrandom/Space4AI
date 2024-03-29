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
* \file RandomGreedy.hpp
*
* \brief Defines the Random Greedy (Design time version) algorithm to solve the
*        optimization problem
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef RANDOM_GREEDY_HPP_
#define RANDOM_GREEDY_HPP_

#include <random>

#include "src/System/System.hpp"
#include "src/Solution/Solution.hpp"
#include "src/Solution/EliteResult.hpp"

namespace Space4AI
{
/** Class to define the Random Greedy algorithm to solve the optimization problem */
class RandomGreedy
{
  public:

    RandomGreedy() = default;

    RandomGreedy(const SelectedResources& fixed_edge_and_curr_rt_vms_)
    : fixed_edge_and_curr_rt_vms(fixed_edge_and_curr_rt_vms_) {}

    /** Method to generate random greedy solutions
    *
    *   \param system Object containing all the data structures of the System
    *   \param max_it Maximum number of iterations for the algorithm
    *   \param num_top_sols Maximum admissible length for the vector of solutions
    *                       (see EliteResult.hpp)
    * 	\param reproducibility_flag Flag to make the random greedy solutions reproducible
    *   \return EliteResult class containing num_top_sols solutions ordered by cost
    */
    EliteResult random_greedy(
      const System& system,
      std::size_t max_it,
      std::size_t num_top_sols,
      bool reproducibility_flag
    );

  private:

    /** Method to create the initial random Solution
    *
    *   It randomly generates a candidate Solution as a starting point to find a
    *   feasible one.
    *
    *   \param system Object containing all the data structures of the System
    *   \return Solution object
    */
    Solution create_random_initial_solution(
      const System& system
    );

    /** Method reduce the number of ResourceType::Edge and ResourceType::VM resources
    *   in a cluster.
    *
    *   \param solution The initial Solution
    *   \param res_type_idx Index indicating the ResourceType of the Resource with index res_idx
    *   \param res_idx Index of the Resource whose cluster needs to be reduced
    *   \param system Object describing the System under analysis
    *   \return Updated Solution with reduced cluster
    */
    Solution reduce_cluster_size(
      const Solution& solution,
      const size_t res_type_idx,
      const size_t res_idx,
      const System& system
    );

  private:

    /** random number generator */
    std::mt19937_64 rng;

    /* Initial seed for the random device */
    inline static constexpr size_t seed = 20122022;

    /** Current run-time solution selected resources (e.g., at design-time) on Edge and VM */
    const SelectedResources fixed_edge_and_curr_rt_vms;


};

} // namespace Space4AI

#endif /* RANDOM_GREEDY_DT_HPP_ */
