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
      LoadType part_lambda_,
      ProbType early_exit_probability_,
      const std::unordered_map<size_t, DataType>&& next_data_sizes,
      size_t num_edge, size_t num_vms, size_t num_faas
    ):
      name(name_), part_lambda(part_lambda_), early_exit_probability(early_exit_probability_),
      next_data_sizes(next_data_sizes)
    {
      memory.resize(ResIdxFromType(ResourceType::Count));
      memory[ResIdxFromType(ResourceType::Edge)].resize(num_edge, NaN);
      memory[ResIdxFromType(ResourceType::VM)].resize(num_vms, NaN);
      memory[ResIdxFromType(ResourceType::Faas)].resize(num_faas, NaN);
    }

    void
    set_memory(DataType mem_val_, size_t res_type_idx, size_t res_idx)
    {
      memory[res_type_idx][res_idx] = mem_val_;
    }

    /** name getter */
    const std::string&
    get_name() const { return name; };

    /** memory getter */
    DataType
    get_memory(size_t res_type_idx, size_t res_idx) const { return memory[res_type_idx][res_idx]; };

    /** part_lambda getter */
    LoadType
    get_part_lambda() const { return part_lambda; };

    /** early_exit_probability getter */
    ProbType
    get_early_exit_probability() const { return early_exit_probability; };

    const std::unordered_map<size_t, DataType>&
    get_next_data_sizes() const {return next_data_sizes;}

  private:

    /** Partition name */
    const std::string name;

    /** Memory requirement of the Partition */
    std::vector<std::vector<DataType>> memory;

    /** Load factor of the Partition */
    const LoadType part_lambda;

    /** Early stopping probability of the NN */
    const ProbType early_exit_probability;

    /** true if partition is the last in its deployment */
    // bool last_part_in_depl;

    /** map form next index, to the ransferred data size. If the partition is the last
    *   in its deployment, next index are related to components, other to the next parition */
    const std::unordered_map<size_t, DataType> next_data_sizes;
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
    const std::string&
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
  friend class SystemData;

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
    const std::string&
    get_name() const { return name; }

    /** deployments vector getter */
    const std::vector<Deployment>&
    get_deployments() const {return deployments;}

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

    Partition&
    get_partition(size_t idx) {return partitions[idx];}

  private:

    /** Name of the component */
    const std::string name;

    /** Vector of Deployment objects */
    const std::vector<Deployment> deployments;

    /** Vector of Partition objects */
    std::vector<Partition> partitions;

    /** Load factor of the Component */
    const LoadType comp_lambda;

};

} //namespace Space4AI

#endif /* COMPONENT_HPP_ */
