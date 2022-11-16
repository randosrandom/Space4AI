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
* \file Resources.hpp
*
* \brief Defines the classes to manage Edge, VM, Faas resources.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef RESOURCES_HPP_
#define RESOURCES_HPP_

#include "src/TypeTraits.hpp"

namespace Space4AI
{
/** Class to represent a computational layer, namely a vector of resources of a
*   fixed type.
*/
class ComputationalLayer
{
  public:

    /** ComputationalLayer constructor.
    *
    * 	\param name_ Name of the ComputationalLayer
    * 	\param type_of_resources_ Type of resources in the layer
    * 							  (either ResourceType::Edge, ResourceType::Vm or
    * 							  ResourceType::Faas)
    */
    ComputationalLayer(const std::string& name_, ResourceType type_of_resources_):
      name(name_),
      type_of_resources(type_of_resources_),
      res_idxs({})
    {}

    /** Method to add the index of a Resource in the ComputationalLayer
    *
    * 	\param res_idx Index of the Resource to add to ComputationalLayer.res_idxs
    */
    void
    add_resource(size_t res_idx) { res_idxs.push_back(res_idx); }

    /** name getter */
    std::string
    get_name() const {return name; }

    /** type_of_resources getter */
    ResourceType
    get_type_of_resources() const {return type_of_resources; }

    /** getter of the resources indexes in the layer */
    const std::vector<size_t>&
    get_res_idxs() const { return res_idxs; }

  private:

    /** name of the ComputationalLayer */
    const std::string name;

    /** type of the resources saved in the layer (either ResourceType::Edge,
    *   ResourceType::VM or ResourceType::Faas)
    */
    const ResourceType type_of_resources;

    /** vector storing the indexes of the resources in the ComputationalLayer */
    std::vector<size_t> res_idxs;
};

/** Template class to manage different types of resources.
*
* 	This will be used for ResourceType::Edge and ResourceType::VM.
*/
template <ResourceType Type>
class Resource
{
  public:

    /** Resource constructor.
    *
    * 	\param name_ Name of the Resource
    * 	\param description_ Eventual description of the Resource
    * 	\param cl_name_ name of the ComputationalLayer where the Resource is located
    * 	\param cost_ Cost of the Resource
    * 	\param memory_ Amount of available memory on the Resource
    * 	\param number_avail_ Number of available objects of the current type
    * 	\param n_cores_ Number of cores of the Resource
    */
    Resource(
      const std::string& name_,
      const std::string& description_,
      const std::string& cl_name_,
      CostType cost_,
      DataType memory_,
      size_t number_avail_,
      size_t n_cores_
    ):
      name(name_),
      description(description_),
      cl_name(cl_name_),
      cost(cost_),
      memory(memory_),
      number_avail(number_avail_),
      n_cores(n_cores_)
    {}

    /** name getter */
    const std::string&
    get_name() const { return name; };

    /** description getter */
    const std::string&
    get_description() const { return description; };

    /** cl_name getter */
    const std::string&
    get_cl_name() const { return cl_name; };

    /** cost getter */
    CostType
    get_cost() const { return cost; };

    /** pure cost getter */
    CostType
    get_pure_cost() const {return cost; };

    /** memory getter */
    DataType
    get_memory() const { return memory; };

    /** number_avail getter */
    size_t
    get_number_avail() const { return number_avail; }

    /** n_cores getter */
    size_t
    get_n_cores() const { return n_cores; };

  private:

    /** Name of the Resource */
    const std::string name;

    /** Eventual description of the Resource */
    const std::string description;

    /** ComputationalLayer where the Resource is located */
    const std::string cl_name;

    /** Cost of the Resource */
    const CostType cost;

    /** Amount of available memory on the Resource */
    const DataType memory;

    /** Number of available objects of the current type */
    const size_t number_avail;

    /** Number of cores of the Resource */
    const size_t n_cores;
};

/** Specialization of the template class Resource for
*   FaaS.
*/
template <>
class Resource<ResourceType::Faas>
{
  public:

    /** Resource constructor.
    *
    * 	\param name_ Name of the Resource
    * 	\param description_ Eventual description of the Resource
    * 	\param cl_name_ ComputationalLayer where the Resource is located
    * 	\param cost_ Cost of the Resource
    * 	\param memory_ Amount of available memory on the Resource
    * 	\param transition_cost_ Transition cost
    * 	\param idle_time_before_kill_ How long does the platform keep the servers up after being idle
    */
    Resource(
      const std::string& name_,
      const std::string& description_,
      const std::string& cl_name_,
      CostType cost_,
      DataType memory_,
      CostType transition_cost_,
      TimeType idle_time_before_kill_
    ):
      name(name_),
      description(description_),
      cl_name(cl_name_),
      cost(cost_),
      memory(memory_),
      transition_cost(transition_cost_),
      idle_time_before_kill(idle_time_before_kill_)
    {}

    /** name getter */
    const std::string&
    get_name() const { return name; };

    /** description getter */
    const std::string&
    get_description() const { return description; };

    /** computational layer name getter */
    const std::string&
    get_cl_name() const { return cl_name; };

    /** cost getter */
    CostType
    get_cost() const { return cost; };

    /** memory getter */
    DataType
    get_memory() const { return memory; };

    /** transition_cost getter */
    TimeType
    get_transition_cost() const { return transition_cost; };

    /** idle_time_before_kill getter */
    TimeType
    get_idle_time_before_kill() const { return idle_time_before_kill; };

  private:

    /** Name of the Resource */
    const std::string name;

    /** Eventual description of the Resource */
    const std::string description;

    /** ComputationalLayer where the Resource is located */
    const std::string cl_name;

    /** Cost of the Resource */
    const CostType cost;

    /** Amount of available memory on the Resource */
    const DataType memory;

    /** Transition cost */
    const TimeType transition_cost;

    /** How long does the platform keep the servers up after being idle */
    const TimeType idle_time_before_kill;
};

/** Class to store all types of resources in a single container. */
class AllResources
{
  public:

    /** AllResources constructor.
    *
    * 	Initializes number_resources as a N-dimensional vector, where N is the
    * 	number of different types of resources.
    */
    AllResources():
      number_resources(std::vector<size_t>(ResIdxFromType(ResourceType::Count), 0))
    {}

    /** Template method that adds the input Resource of type T in the
    * 	corresponding container.
    */
    template<class T>
    void add_resource(T&& resource);

    /** Template getter that returns all the resources of type T. */
    template<ResourceType T>
    const std::vector<Resource<T>>& get_resources() const;

    /** Template getter that returns the Resource of type T with the input index.
    *
    *	\param res_idx Index of the Resource to be returned
    *	\return const reference to the chosen Resource
    */
    template<ResourceType T>
    const Resource<T>& get_resource(size_t res_idx) const;

    /** Method to get the name of a Resource.
    *
    *	\param res_type Resource Type (ResourceType::VM, ResourceType::Edge or ResourceType::Faas)
    *	\param res_idx Index of the resource
    * 	\return name of the Resource
    */
    const std::string& get_name(ResourceType res_type, size_t res_idx) const;

    /** Method to get the description of a Resource.
    *
    *	\param res_type Resource Type (ResourceType::VM, ResourceType::Edge or ResourceType::Faas)
    *	\param res_idx Index of the Resource
    * 	\return description of the Resource
    */
    const std::string& get_description(ResourceType res_type, size_t res_idx) const;

    /** Method to get the total number of resources of the type corresponding to
    * 	the type_idx (see ResourceType enum class in TypeTraits.hpp)
    *
    * 	\param res_type_idx Index corresponding to a certain ResourceType
    * 						(see ResourceType enum class in TypeTraits.hpp)
    * 	\return total number of resources of the input type
    */
    size_t get_number_resources(size_t res_type_idx) const;

    /** Method to get the ComputationalLayer of the input Resource.
    *
    *   This method is needed at runtime, when the ResourceType is unknown.
    *
    * 	\param res_type Resource Type (either ResourceType::Edge, ResourceType::VM
    *				    or ResourceType::Faas)
    * 	\param res_idx Index of the Resource
    * 	\return name of the ComputationalLayer including the input Resource
    */
    const std::string& get_cl_name(ResourceType res_type, size_t res_idx) const;

    /** Method to get the memory of the input Resource.
    *
    * 	This method is needed at runtime, when the ResourceType is unknown.
    *
    * 	\param res_type Resource Type (either ResourceType::Edge, ResourceType::VM
    *				    or ResourceType::Faas)
    * 	\param res_idx Index of the resource
    * 	\return memory of the input Resource
    */
    DataType get_memory(ResourceType res_type, size_t res_idx) const;


    /** Method to get the cost of the input Resource.
    *
    * 	This method is needed at runtime, when the ResourceType is unknown.
    *
    * 	\param res_type Resource Type (either ResourceType::Edge, ResourceType::VM
    *				    or ResourceType::Faas)
    * 	\param res_idx Index of the Resource
    * 	\return cost of the input Resource
    */
    CostType get_cost(ResourceType res_type, size_t res_idx) const;


    /** Method to get the number_avail of the input Resource.
    *
    * 	This method is needed at runtime, when the ResourceType is unknown.
    * 	Note that the method accepts also ResourceType::Faas resources as input,
    * 	even though ResourceType::Faas lacks the concept of available
    * 	objects. Anyway, in such case, 1 is returned.
    *
    * 	\param res_type Resource Type (either ResourceType::Edge, ResourceType::VM
    *				    or ResourceType::Faas)
    * 	\param res_idx Index of the Resource
    * 	\return number_avail of the input Resource
    */
    size_t get_number_avail(ResourceType res_type, size_t res_idx) const;

  private:

    /** Store for each type of Resource, the number of resources of that type */
    std::vector<size_t> number_resources;

    /** ResourceType::Edge resources vector */
    std::vector<Resource<ResourceType::Edge>> edge_resources;

    /** ResourceType::VM resources vector */
    std::vector<Resource<ResourceType::VM>> vm_resources;

    /** ResourceType::Faas Resources vector */
    std::vector<Resource<ResourceType::Faas>> faas_resources;
};

// ---------------------------------------------------------------------------
// DEFINITION OF TEMPLATES and METHODS
// ---------------------------------------------------------------------------

template <class T>
void
AllResources::add_resource(T&& resource)
{
  if constexpr(
    std::is_same_v<T, Resource<ResourceType::Edge>> // passed by lvalue
    ||
    std::is_same_v<T, Resource<ResourceType::Edge>&&>) // passed by rvalue
  {
    edge_resources.emplace_back(std::forward<T>(resource));
    number_resources[ResIdxFromType(ResourceType::Edge)]++;
  }
  else if constexpr(
    std::is_same_v<T, Resource<ResourceType::VM>>
    ||
    std::is_same_v<T, Resource<ResourceType::VM>&&>)
  {
    vm_resources.emplace_back(std::forward<T>(resource));
    number_resources[ResIdxFromType(ResourceType::VM)]++;
  }
  else if constexpr(
    std::is_same_v<T, Resource<ResourceType::Faas>>
    ||
    std::is_same_v<T, Resource<ResourceType::Faas>&&>)
  {
    faas_resources.emplace_back(std::forward<T>(resource));
    number_resources[ResIdxFromType(ResourceType::Faas)]++;
  }
  else
  {
    Logger::Error("Wrong type provided in *add_resource* of Resources.hpp");
    throw std::invalid_argument("Wrong type provided in *add_resource* of Resources.hpp");
  }
}

template<ResourceType T>
const std::vector<Resource<T>>&
AllResources::get_resources() const
{
  if constexpr(T == ResourceType::Edge)
  {
    return edge_resources;
  }
  else if constexpr(T == ResourceType::VM)
  {
    return vm_resources;
  }
  else if constexpr(T == ResourceType::Faas)
  {
    return faas_resources;
  }
  else
  {
    Logger::Error("Bad type provided in *get_resources()* in Resources.hpp");
    throw std::invalid_argument("Wrong type provided in *get_resources()* in Resources.hpp");
  }
}

template<ResourceType T>
const Resource<T>&
AllResources::get_resource(size_t res_idx) const
{
  if constexpr(T == ResourceType::Edge)
  {
    return edge_resources[res_idx];
  }
  else if constexpr(T == ResourceType::VM)
  {
    return vm_resources[res_idx];
  }
  else if constexpr(T == ResourceType::Faas)
  {
    return faas_resources[res_idx];
  }
  else
  {
    Logger::Error("Bad type provided in *get_resources(size_t)* in Resources.hpp");
    throw std::invalid_argument("Wrong type provided in *get_resources(size_t)* in Resources.hpp");
  }
}

inline
const std::string&
AllResources::get_name(ResourceType res_type, size_t res_idx) const
{
  switch(res_type)
  {
    case ResourceType::Edge: return edge_resources[res_idx].get_name();

    case ResourceType::VM: return vm_resources[res_idx].get_name();

    case ResourceType::Faas: return faas_resources[res_idx].get_name();

    default:
      Logger::Error("Wrong type provided to Resources::get_name(...)");
      throw std::invalid_argument("Wrong type provided to Resources::get_name(...)");
  }
}

inline
const std::string&
AllResources::get_description(ResourceType res_type, size_t res_idx) const
{
  switch(res_type)
  {
    case ResourceType::Edge: return edge_resources[res_idx].get_description();

    case ResourceType::VM: return vm_resources[res_idx].get_description();

    case ResourceType::Faas: return faas_resources[res_idx].get_description();

    default:
      Logger::Error("Wrong type provided to Resources::get_name(...)");
      throw std::invalid_argument("Wrong type provided to Resources::get_name(...)");
  }
}

inline
size_t
AllResources::get_number_resources(size_t res_type_idx) const
{
  return number_resources[res_type_idx];
}

// I need this necessarily, size at compile time I cannot know the type
// related to the function SystemPerformanceEvaluator::get_network_delay()
inline
const std::string&
AllResources::get_cl_name(ResourceType res_type, size_t res_idx) const
{
  switch(res_type)
  {
    case ResourceType::Edge: return edge_resources[res_idx].get_cl_name();

    case ResourceType::VM: return vm_resources[res_idx].get_cl_name();

    case ResourceType::Faas: return faas_resources[res_idx].get_cl_name();

    default:
      Logger::Error("Wrong type provided to Resources::get_cl_name(ResourceType, size_t)");
      throw std::invalid_argument("Wrong type provided to Resources::get_cl_name(ResourceType, size_t)");
  }
}

inline
DataType
AllResources::get_memory(ResourceType res_type, size_t res_idx) const
{
  switch(res_type)
  {
    case ResourceType::Edge: return edge_resources[res_idx].get_memory();

    case ResourceType::VM: return vm_resources[res_idx].get_memory();

    case ResourceType::Faas: return faas_resources[res_idx].get_memory();

    default:
      Logger::Error("Wrong type provided to Resources::get_memory(ResourceType, size_t)");
      throw std::invalid_argument("Wrong type provided to Resources::get_memory(ResourceType, size_t)");
  }
}

inline
CostType
AllResources::get_cost(ResourceType res_type, size_t res_idx) const
{
  switch(res_type)
  {
    case ResourceType::Edge: return edge_resources[res_idx].get_cost();

    case ResourceType::VM: return vm_resources[res_idx].get_cost();

    case ResourceType::Faas: return faas_resources[res_idx].get_cost();

    default:
      Logger::Error("Wrong type provided to Resources::get_cost(ResourceType, size_t)");
      throw std::invalid_argument("Wrong type provided to Resources::get_cost(ResourceType, size_t)");
  }
}

inline
size_t
AllResources::get_number_avail(ResourceType res_type, size_t res_idx) const
{
  switch(res_type)
  {
    case ResourceType::Edge: return edge_resources[res_idx].get_number_avail();

    case ResourceType::VM: return vm_resources[res_idx].get_number_avail();

    case ResourceType::Faas:
      Logger::Debug("Getting number avail of Faas, which is unnecessary and maybe inefficient");
      return 1; // Actually, you should not pass Faas to this function

    default:
      Logger::Error("Wrong type provided to Resources::get_number_avail(ResourceType, size_t)");
      throw std::invalid_argument("Wrong type provided to Resources::get_number_avail(ResourceType, size_t)");
  }
}

} //namespace Space4AI

#endif // RESOURCES_HPP_
