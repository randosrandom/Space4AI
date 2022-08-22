/**
* \file Component.hpp
*
* \brief Defines the classes Component, Deployment, Partition
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef COMPONENT_HPP_
#define COMPONENT_HPP_

#include <vector>
#include <string>
#include <utility>

#include "src/TypeTraits.hpp"

namespace Space4AI
{
/** Class to represent a Partition of a Component. */
class Partition
{
  public:

    /** Partition class constructor.
     *
     *  \param name_ Partition name
     *  \param memory_ Memory requirement of the Partition
     *  \param part_lambda_ Load factor of the Partition
     *  \param early_exit_probability_ Early stopping probablility of the NN
     *  \param next_ Name of the subsequent Partition
     *  \param data_size_ Amount of data transferred to the subsequent Partition
     */
    Partition(
      const std::string& name_,
      DataType memory_,
      LoadType part_lambda_,
      ProbType early_exit_probability_,
      const std::string& next_,
      DataType data_size_
    ):
      name(name_), memory(memory_), part_lambda(part_lambda_),
      early_exit_probability(early_exit_probability_), next(next_), data_size(data_size_)
    {}

    /** name getter */
    std::string
    get_name() const { return name; };

    /** memory getter */
    DataType
    get_memory() const { return memory; };

    /** part_lambda getter */
    LoadType
    get_part_lambda() const { return part_lambda; };

    /** early_exit_probability getter */
    ProbType
    get_early_exit_probability() const { return early_exit_probability; };

    /** next Partition name getter */
    std::string
    get_next() const { return next; };

    /** data_size getter */
    DataType
    get_data_size() const { return data_size; }

  private:

    /** Partition name */
    const std::string name;

    /** Memory requirement of the Partition */
    const DataType memory;

    /** Load factor of the Partition */
    const LoadType part_lambda;

    /** Early stopping probability of the NN */
    const ProbType early_exit_probability;

    /** Name of the subsequent Partition */
    const std::string next;

    /** Amount of data transferred to the subsequent Partition */
    const DataType data_size;

};

/** Class to represent a candidate Deployment for a Component. */
class Deployment
{
  public:

    /** Deployment class constructor.
    *
    *   \param name_ Name of the Deployment
    *   \param partitions_indices_ Vector of indexes (corresponding to the ones of
    *                             Component.partitions) of the partitions in the Deployment
    */
    Deployment(
      const std::string& name_,
      const std::vector<std::size_t>& partitions_indices_
    ):
      name(name_), partitions_indices(partitions_indices_)
    {}

    /** name getter */
    std::string
    get_name() const { return name; };

    /** partition_indices getter */
    const std::vector<std::size_t>&
    get_partition_indices() const { return partitions_indices; };

  private:

    /** Name of the Deployment */
    const std::string name;

    /** Vector of indexes of Component.partitions objects characterizing the Deployment */
    const std::vector<std::size_t> partitions_indices;

};

/** Class to represent the components, namely DAG nodes. */
class Component
{
  public:

    /** Component class constructor (move semantic used)
    *
    *   \param name_ Component name
    *   \param deployments_ Vector of Deployment objects
    *   \param partitions_ Vector of Partition objects
    *   \param comp_lambda_ Load factor of the Component
    */
    template<class D, class P>
    Component(
      const std::string& name_,
      D&& deployments_,
      P&& partitions_,
      LoadType comp_lambda_
    ):
      name(name_),
      deployments(std::forward<D>(deployments_)), partitions(std::forward<P>(partitions_)),
      comp_lambda(comp_lambda_)
    {}

    /** name getter */
    std::string
    get_name() const { return name; }

    /** deployments vector getter */
    const std::vector<Deployment>&
    get_deployments() const
    {
      return deployments;
    }

    /** partitions vector getter */
    const std::vector<Partition>&
    get_partitions() const { return partitions; }

    /** Single partition getter
    *
    *   \param idx index of the requested Partition
    *   \return requested Partition
    */
    const Partition&
    get_partition(size_t idx) const { return partitions[idx]; }

    /** comp_lambda getter */
    LoadType
    get_comp_lambda() const { return comp_lambda; }

  private:

    /** Name of the component */
    const std::string name;

    /** Vector of Deployment objects */
    const std::vector<Deployment> deployments;

    /** Vector of Partition objects */
    const std::vector<Partition> partitions;

    /** Load factor of the Component */
    const LoadType comp_lambda;

};

} //namespace Space4AI

#endif /* COMPONENT_HPP_ */
