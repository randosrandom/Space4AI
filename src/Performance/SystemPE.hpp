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
* \file SystemPE.hpp
*
* \brief Defines the evaluators to measure the performance of the components and paths
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef SYSTEMPE_HPP_
#define SYSTEMPE_HPP_

#include <algorithm>

#include "src/Solution/SolutionData.hpp"
#include "src/System/System.hpp"
#include "src/TypeTraits.hpp"

namespace Space4AI
{
  /** Class used to save which components, resources and partitions has been modified
  *   by the local algortithms.
  */
  class LocalInfo
  {
  public:

    /** Method to reset modified_res */
    void
    reset() {
      active = false;
      for(auto& vec : modified_res)
        vec.assign(vec.size(), false);
      modified_comp = std::make_pair(false, 0);
      old_local_parts_perfs_ptr = nullptr;
      old_local_parts_delays_ptr = nullptr;
    }

  public:

    /** flag set to true if the class in not empty */
    bool active = false;

    /** For each resource type and resource, true if that device has been modified
    * by the local search */
    std::vector<std::vector<bool>> modified_res;

    /** Index of the modified component */
    std::pair<bool, size_t> modified_comp;

    /** pointer to the original local_parts_perfs */
    std::vector<std::vector<TimeType>> const * old_local_parts_perfs_ptr = nullptr;

    /** pointer to the original local_parts_delays */
    std::vector<std::vector<TimeType>> const * old_local_parts_delays_ptr = nullptr;

  };

/** Class used to evaluate the performance (response times) of the whole System. */
class SystemPE
{
  public:

    friend class Solution;
    friend class LocalSearch;

    // /** Method to evaluate the response time of the all Component objects.
    // *
    // *   \param system Object containing all the data structures of the System
    // *   \param solution_data SolutionData object
    // */
    // void
    // compute_local_perfs(
    //   const System& system,
    //   const SolutionData& solution_data
    // ); // CURRENTLY NOT BEING USED!

    /** Method to evaluate the response time of the Component object
    *   identified by the given index.
    *
    *   \param comp_idx Index of the Component
    *   \param system Object containing all the data structures of the System
    *   \param solution_data SolutionData object
    */
    void
    compute_local_perf(
      size_t comp_idx,
      const System& system,
      const SolutionData& solution_data,
      const LocalInfo& local_info);

    /** Method to evaluate the performance of a path
    *
    *   \param path_idx Index of the path
    *   \param system System object
    *   \param solution_data SolutionData object
    */
    void
    compute_global_perf(
      size_t path_idx,
      const System& system,
      const SolutionData& solution_data,
      const LocalInfo& local_info = LocalInfo());

private:

    /** Method to compute the network delay due to data transfer
    *   operations between two consecutive Components or Partitions object, executed
    *   on different resources in the same NetworkDomain.
    *
    *   \param res1_type Type fo the first Resource
    *   \param res1_idx First Resource index
    *   \param res2_type Type of the second Resource
    *   \param res2_idx Second Resource index
    *   \param data_size Amount of data to be transferred
    *   \param system System under analysis
    */
    TimeType
    compute_network_delay(
      ResourceType res1_type, size_t res1_idx, ResourceType res2_type, size_t res2_idx,
      DataType data_size,
      const System& system);

  private:

    /** For each component, save timings of the chosen deployment partition */
    std::vector<std::vector<TimeType>> local_parts_perfs;

    /** For each component, save network delays between partitions */
    std::vector<std::vector<TimeType>> local_parts_delays;

    /** Component (local) performance */
    std::vector<TimeType> comp_perfs;

    /** Delays between components */
    std::vector<TimeType> comp_delays;

    /** Global paths performance */
    std::vector<TimeType> path_perfs;

};

} // namespace Space4AI

#endif /* SYSTEM_PE_ */
