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
* \brief Defines the evaluators to measure the performance of the System
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef SYSTEMPE_HPP_
#define SYSTEMPE_HPP_

#include "src/Solution/SolutionData.hpp"
#include "src/System/System.hpp"
#include "src/TypeTraits.hpp"

namespace Space4AI
{
/** Class designed to evaluate the performance of a NetworkDomain object.
*
*   The performance can be defined as the time
*   required to transfer data between two consecutive Component or Partition
*   objects executed on different devices in the same NetworkDomain.
*/
class NetworkPE
{
  public:

    /** Static method to compute the transfer time in a NetworkDomain.
    *
    *   \param access_delay Access delay characterizing the NetworkDomain
    *   \param bandwidth Bandwidth characterizing the NetworkDomain
    *   \param data Amount of transferred data
    *   \return transfer time
    */
    static
    TimeType
    predict(
      TimeType access_delay,
      double bandwidth,
      DataType data
    )
    {
      return access_delay + (data / bandwidth);
    }

};

/** Class used to evaluate the performance (response times) of the whole System. */
class SystemPE
{
  public:

    /** Static method to evaluate the response time of the all Component objects.
    *
    *   \param system Object containing all the data structures of the System
    *   \param solution_data SolutionData object
    *   \return vector with the response times of all components
    */
    static
    std::vector<TimeType> compute_performance(
      const System& system,
      const SolutionData& solution_data
    );

    /** Static method to evaluate the response time of the Component object
    *   identified by the given index.
    *
    *   \param comp_idx Index of the Component
    *   \param system Object containing all the data structures of the System
    *   \param solution_data SolutionData object
    *   \return Response time of the Component
    */
    static
    TimeType get_perf_evaluation(
      size_t comp_idx,
      const System& system,
      const SolutionData& solution_data
    );

    /** Static method to compute the network delay due to data transfer
    *   operations between two consecutive Components or Partitions object, executed
    *   on different resources in the same NetworkDomain.
    *
    *   \param res1_type Type fo the first Resource
    *   \param res1_idx First Resource index
    *   \param res2_type Type of the second Resource
    *   \param res2_idx Second Resource index
    *   \param data_size Amount of data to be transferred
    *   \param system System under analysis
    *   \return Network transfer time
    */
    static
    TimeType get_network_delay(
      ResourceType res1_type, size_t res1_idx, ResourceType res2_type, size_t res2_idx,
      double data_size,
      const System& system
    );

};

} // namespace Space4AI

#endif /* SYSTEM_PE_ */
