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
/** Class designed to evaluate the performance of a NetworkTechnology object.
*
*   The performance of a NetworkTechnology object can be defined as the time
*   required to transfer data between two consecutive Component or Partition
*   objects executed on different devices in the same NetworkDomain.
*/
class NetworkPerformanceEvaluator
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

/** Class used to evaluate the performance (response time) of the whole System. */
class SystemPerformanceEvaluator
{
  public:

    /** Static method to evaluate the response time of the all Component objects.
    *
    *   \param system Object containing all the data structures of the System
    *   \param solution_data Object storing SolutionData.y_hat and SolutionData.used_resources
    *                        (main data of the Solution)
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
    *   \param solution_data Object storing SolutionData.y_hat and SolutionData.used_resources
    *                        (main data of the Solution)
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
    *   \param system Object containing all the data structures of the System
    *   \return Network transfer time
    */
    static
    double get_network_delay(
      ResourceType res1_type, size_t res1_idx, ResourceType res2_type, size_t res2_idx,
      double data_size,
      const System& system
    );

};

} // namespace Space4AI

#endif /* SYSTEM_PE_ */
