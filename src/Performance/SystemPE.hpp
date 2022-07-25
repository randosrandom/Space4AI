/**
* \file SystemPE.hpp
*
* \brief Defines the evaluators to measure the performance of the system
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef SYSTEMPE_HPP_
#define SYSTEMPE_HPP_

#include "Solution/SolutionData.hpp"
#include "System/System.hpp"
#include "TypeTraits.hpp"


namespace Space4AI
{
  /** NetworkPerformanceEvaluator.
  *
  *   Class designed to evaluate the performance of a NetworkTechnology object,
  *   namely the time required to transfer data between two consecutive
  *   Component or Partition objects executed on different devices in the same network domain.
  */
  class NetworkPerformanceEvaluator
  {

  public:

    /** Static method to compute the transfer time in a network domain.
    *
    * \param access_delay Access delay characterizing the network domain
    * \param bandwidth Bandwidth characterizing the network domain
    * \param data Amount of transferred data
    * \return transfer time
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

  /** SystemPerformanceEvaluator.
  *   Class used to evaluate the performance (response time) of the whole system.
  */
  class SystemPerformanceEvaluator
  {
  public:

    /**
    *   Static method to evaluate the response time of the all Component objects.
    *
    *   \param system System under analysis
    *   \param solution_data Store y_hat and used_resources (main dat aof the solution)
    *
    *   \return vector with the response times of all components
    */
    static
    std::vector<TimeType>
    compute_performance(
      const System& system,
      const SolutionData& solution_data
    );

    /** Static method to evaluate the response time of the Component object
    *   identified by the given index.
    *
    *   \param comp_idx Index of the component
    *   \param system
    *   \param solution_data
    *
    *   \return Response time
    */
    static
    TimeType
    get_perf_evaluation(
      size_t comp_idx,
      const System& system,
      const SolutionData& solution_data
    );

    /** Static method to compute the network delay due to data transfer
    *   operations between two consecutive components (or partitions), executed
    *   on different resources in the same network domain.
    *
    *   \param res1_type Type fo the first resource
    *   \param res1_idx First resource index
    *   \param res2_type Type of the second resource
    *   \param res_idx Second resource index
    *   \param data_size Amount of data to be transferred
    *   \param system
    *
    *   \return Network transfer time
    */
    static
    double
    get_network_delay(
      ResourceType res1_type, size_t res1_idx, ResourceType res2_type, size_t res2_idx,
      double data_size,
      const System& system
    );

  };

} // namespace Space4AI

#endif /* SYSTEM_PE_ */
