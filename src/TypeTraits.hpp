/**
* \file TypeTraits.hpp
*
* \brief Defines the main types and data structures used in the code.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef TYPETRAITS_HPP_
#define TYPETRAITS_HPP_

#include <memory>
#include <utility>
#include <set>
#include <vector>
#include <tuple>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "external/nlohmann/json.hpp"

namespace Space4AI
{
class BasePerformanceModel; // forward declaration

// RANDO: Dopo che Giulia ha commentato tutto, aggiungi tu l'osservazione importante
// sugli indici (fatto che Edge=0, VM=1: non possono essere cambiati perché
// i vettori usano questi indici nell'operator[], come nota sul quaderno)

/** Enum class to identify different types of resources */
enum class ResourceType : size_t
{
	Edge = 0,
	VM = 1,
	Faas = 2,
	Count = 3, /**< total number of resource types */
};

using CostType = double; /**< represents the cost type used */
using ProbType = double; /**< represents transition probabilities of the DAG */
using DataType = double; /**< represents data sizes passed between components */
using LoadType = double; /**< represents the input exogenous workload */
using TimeType = double; /**< represents time */

/** Compatibility Matrix of the system.
*
*   Four-dimensional vector of booleans (indexed by [i][j][k][l]) storing true if
*   partition k of Component i is compatible with resource l of type j, false otherwise
*
*        i: index of the component
*        j: index of the resource type
*        k: index of partition
*        l: index of resource.
*/
using CompatibilityMatrixType =
    std::vector<std::vector<std::vector<std::vector<bool>>>>;

/** Strurcture used to save the transition matrix of the DAG.
*
*   Two-dimensional vector of ProbType (indexed by [i][j]) storing the transition
*   probability from component j to component i.
*/
using DagMatrixType = std::vector<std::vector<ProbType>>;

/** DemandEdgeVMType needed to store demands of resources using QTPE.
*
*   QTPE is the only PE that must know demands from the other instances of the PE
*   (i.e. other components and partitions running on the same Resource).
*   Our design choice is to save a static 4-dim vector
*   (indexed by [comp_idx][type_idx][part_idx][res_idx]) using Nans when
*   the specific resources running the specific partition does not use QTPE.
*   Other design choices are available, for instance, to use less memory,
*   shared_ptr would be optimal but in this case just a copy is better.
*/
using DemandEdgeVMType =
    std::vector<std::vector<std::vector<std::vector<TimeType>>>>;

/** Predictors to compute the demand time.
*
*   For each Partition and each Component, running on a specific Resource of a
*   specific ResourceType, we save a unique_ptr to the BasePerformanceModel class,
*   exploiting Inheritance and Polymorphism.
*/
using PerformanceType =
    std::vector<std::vector<std::vector<std::vector<
    std::unique_ptr<BasePerformanceModel>
    >>>>;

/** Type of the "y hat".
*
*   The indexing is equivalent to the one of CompatibilityMatrixType.
*   It stores the number of resources of a certain ResourceType, running a certain
*   Component-Partition couple.
*/
using YHatType =
    std::vector<std::vector<std::vector<std::vector<
    std::size_t
    >>>>;

/** Structure to save only the deployed resources.
*
*   For each Component (indexing the first vector) we save a vector of tuples,
*   (Partition index, ResourceType index, Resource index) used on that Component.
*   Actually it could be sufficient to use just y_hat, but since the number of
*   chosen resources can be much smaller than available resources, it is better
*   to have a data structure that keeps track only of the used resources.
*
*   The structure is ordered, in fact the inner vector must be ordered by
*   part_idx at some point.
*   Using a std::set would not be efficient since order is not necessary to
*   keep the order for each insert (See report for details)
*/
using UsedResourcesOrderedType =
    std::vector<std::vector<
    std::tuple<size_t, size_t, size_t>
    >>;

/** For each [Resource Type, Resource idx] save the number of instances of
*   such Resource deployed in the System.
*/
using UsedResourcesNumberType =
    std::vector<std::vector<size_t>>;

/** Function that converts a ResourceType::member to its corresponding index
*
*   \param ResourceType A ResourceType::member
*   \return index of the ResourceType::member
*/
inline
size_t
ResIdxFromType(ResourceType resType)
{
	return static_cast<std::size_t>(resType);
}

/** Function to convert an index to the corresponding ResourceType::member
*
*   \param type_idx An index of a ResourceType::member
*   \return ResourceType::member corresponding to type_idx
*/
inline
ResourceType
ResTypeFromIdx(size_t typeIdx)
{
	return static_cast<ResourceType>(typeIdx);
}

} //namespace Space4AI

#endif /* TYPETRAITS_HPP_ */
