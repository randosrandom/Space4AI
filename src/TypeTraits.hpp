/**
* \file TypeTraits.hpp
*
* \brief Defines the main types and data structures used in the code.
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef TYPETRAITS_HPP_
#define TYPETRAITS_HPP_

#include <memory>
#include <utility>
#include <set>
#include <vector>
#include <tuple>

namespace Space4AI
{
  class BasePerformanceModel; // forward declaration

  /** Enum class to identify different types of resources */
  enum class ResourceType: size_t
  {
    Edge = 0,
    VM = 1,
    Faas = 2,
    Count = 3, /**< total number of resource types */
  };

  using CostType = double; /**< represent the cost type used */
  using ProbType = double; /**< represent ransition probabilities of the dag */
  using DataType = double; /**< represent data sizes passed between components */
  using LoadType = double; /**< represent the input exogenous workload */
  using TimeType = double; /**< represent time */

  /*
  * Compatibility Matrix of the system.
  * Four dimensional vector if bool (indexed by [i][j][k][l]) storing true if
  * partition k of Component i is compatible with resource l of type j, false otherwise
  *
  *        i: index of the component
  *        j: index of the resource type
  *        k: index of partition
  *        l: index of resource.
  */
  using CompatibilityMatrixType =
    std::vector<std::vector<std::vector<std::vector<bool>>>>;

  /**
  * Used to save the transition matrix of the DAG.
  * Two dimensional vector of ProbType (indexed by [i][j]) storing the transition
  * probability from component j to component i.
  */
  using DagMatrixType = std::vector<std::vector<ProbType>>;

  /**
  * DemandEdgeVMType needed to store demands of resources using of QTPE.
  *
  * QTPE is the only PE that must know demands from the other instances of the PE
  * (other components and partition running on the same resource). The design choice
  * we choose is to save a static 4-dim vector
  * (indexed by [comp_idx][type_idx][part_idx][res_idx]) and we save Nans when
  * the specific resources running the specific partition does not use QTPE.
  * Other design choice are available but to use less memory would require
  * shared_ptr but in this case just a copy is better.
  */
  using DemandEdgeVMType =
    std::vector<std::vector<std::vector<std::vector<TimeType>>>>;

  /** Predictors to compute the demand time.
  *
  *   For ach partition or each component, running on a specific resource of a
  *   specific type, we save an unique_ptr to the BasePerformanceModel class,
  *   exploiting Inheritance and Polymorphism.
  */
  using PerformancePredictorsType =
    std::vector<std::vector<std::vector<std::vector<
      std::unique_ptr<BasePerformanceModel>
    >>>>;

  /** Type of the "y hat".
  *   Indexing equivalent to CompatibilityMatrixType. It stores the number of
  *   resources of a certain type, running a certain component-partition couple.
  */
  using YHatType =
    std::vector<std::vector<std::vector<std::vector<
      std::size_t
    >>>>;

  /** Save only the deployed resources.
  *   For each component (indexing the first vector) we save a vector of tuples,
  *   (partition index, ResourceType index, Resource index) used on that component.
  *   Actually it could be sufficient to use just YHat, but since the number of
  *   chosen resources can be much smaller than available resources, it is better
  *   to have a data structure that keeps track only of the used resources.
  *
  *   There is Ordered in name, since the inner vector must be ordered by part_idx at some point.
  *   Using a std::set would not be efficient since order it's not necessary to
  *   keep the order for each insert (See report for details)
  */
  using UsedResourcesOrderedType =
    std::vector<std::vector<
      std::tuple<size_t, size_t, size_t>
    >>;

  /** For each [Resource Type, Resource idx] save the number of instances of
  *   such resource deployed in the system.
  */
  using UsedResourcesNumberType =
    std::vector<std::vector<size_t>>;

  /**
  * Function that convert a ResourceType::member to the corresponding index
  * \param ResType a ResourceType::member
  * \return index of the ResourceType::member
  */
  inline
  size_t
  ResourceIndexFromType(ResourceType ResType)
  {
    return static_cast<std::size_t>(ResType);
  }

  /**
  * function to convert an index to the corresponding ResourceType::member
  * \param type_idx an index of a ResourceType::member
  * \return ResourceType::member corresponding to type_idx
  */
  inline
  ResourceType
  ResourceTypeFromIndex(size_t type_idx)
  {
    return static_cast<ResourceType>(type_idx);
  }

} //namespace Space4AI

#endif /* TYPETRAITS_HPP_ */
