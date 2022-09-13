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
* \file SystemData.hpp
*
* \brief Defines the class to store the System configuration read from json file.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef SYSTEMDATA_HPP_
#define SYSTEMDATA_HPP_

#include <fstream>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "src/Constraints/GlobalConstraint.hpp"
#include "src/Constraints/LocalConstraint.hpp"
#include "src/Graph/Component.hpp"
#include "src/Graph/Dag.hpp"
#include "src/NetworkTechnology.hpp"
#include "src/Resources.hpp"
#include "src/TypeTraits.hpp"

namespace Space4AI
{
class System; // forward declaration

namespace nl = nlohmann;

/** Class to store the System configuration.
*
*   Class to store the data structures that define the System configuration described
*   in the json configuration file except for the performance section.
*/
class SystemData
{
    friend class System;

  private:

    SystemData() = default;
    SystemData(const SystemData&) = default;
    SystemData& operator=(const SystemData&) = default;

  public:

    /** Method to populate all the memebers of SystemData.
    *
    *   \param configuration_file nl::json object extracted from .json system_file
    */
    void read_json(const nl::json& configuration_file);

    /** DAG getter */
    const DAG&
    get_dag() const {return dag;}

    /** lambda getter */
    LoadType
    get_lambda() const {return lambda;}

    /** time getter */
    TimeType
    get_time() const {return time;}

    /** components getter */
    const std::vector<Component>&
    get_components() const { return components; }

    /** component getter by idx
    *
    *   \param comp_idx index of the desired Component in SystemData.components
    *   \return Component object corresponding to the input index.
    */
    const Component&
    get_component(size_t comp_idx) const { return components[comp_idx]; }

    /** comp_name_to_idx getter */
    const std::unordered_map<std::string, std::size_t>&
    get_comp_name_to_idx() const { return comp_name_to_idx; }

    /** idx_to_comp_name getter */
    const std::map<size_t, std::string>&
    get_idx_to_comp_name() const { return idx_to_comp_name; }

    /** part_name_to_part_idx getter */
    const std::unordered_map<std::string, std::size_t>&
    get_part_name_to_part_idx() const {return part_name_to_part_idx;}

    /** cls getter */
    const std::vector<std::vector<ComputationalLayer>>&
    get_cls() const {return cls;}

    /** cl_name_to_idx getter */
    const std::vector<std::unordered_map<std::string, std::size_t>>
    get_cl_name_to_idx() const {return cl_name_to_idx;}

    /** resources getter */
    const AllResources&
    get_all_resources() const {return all_resources;}

    /** res_name_to_type_and_idx getter */
    const std::unordered_map<std::string, std::pair<ResourceType, std::size_t>>&
    get_res_name_to_type_and_idx() const {return res_name_to_type_and_idx;}

    /** dt_selected_resources getter */
    const DTSelectedResourcesType&
    get_dt_selected_resources() const {return dt_selected_resources;}

    /** compatibility matrix getter */
    const CompatibilityMatrixType&
    get_compatibility_matrix() const {return compatibility_matrix;}

    /** network domains getter */
    const std::vector<NetworkDomain>&
    get_network_domains() const {return network_domains;}

    /** cl_to_network_domains getter */
    const std::unordered_map<std::string, std::vector<std::size_t>>&
    get_cl_to_network_domains() const {return cl_to_network_domains;}

    /** local constraints getter */
    const std::vector<LocalConstraint>&
    get_local_constraints() const {return local_constraints;}

    /** global constraints getter */
    const std::vector<GlobalConstraint>&
    get_global_constraints() const {return global_constraints;}

    /** gc_name_to_idx getter */
    const std::unordered_map<std::string, std::size_t>
    get_gc_name_to_idx() const {return gc_name_to_idx;}

  private:

    /** Method to initialize the Components.
    *
    *   \param components_json nl::json object extracted from the .json configuration file
    *                          containing the description of the components
    */
    void initialize_components(const nl::json& components_json);

    /** Method to initialize the Resources.
    *
    *   \param resources_json nl::json object extracted from the .json configuration file
    *                         containing the description of the resources
    */
    template<ResourceType Type>
    void initialize_resources(const nl::json& resources_json);

    /** Method to initialize the compatibility_matrix.
    *
    *   \param comp_matrix_json nl::json object extracted from the .json configuration file
    *                           containing the description of the compatibility matrix
    */
    void initialize_compatibility_matrix(const nl::json& comp_matrix_json);

    /** Method to initialize the network technology defined in NetworkTechnology.hpp.
    *
    *   \param network_technology_json nl::json object extracted from the .json configuration file
    *                                  containing the description of the network technology
    */
    void initialize_network_technology(const nl::json& network_technology_json);

    /** Method to initialize the LocalConstraint constraints.
    *
    *   \param local_constraints_json nl::json object extracted from the .json configuration file
    *                                 containing the description of the local constraints
    */
    void initialize_local_constraints(const nl::json& local_constraints_json);

    /** Method to initialize the GlobalConstraint constraints.
    *
    *   \param global_constraints_json nl::json object extracted from the .json configuration file
    *                                  containing the description of the global constraints
    */
    void Initialize_global_constraints(const nl::json& global_constraints_json);

  private:

    /** Method to find the order of the Partition in the .json configuration_file
    *   and assign to them increasing indexes.
    *   Note that this ordering method is different from the
    *   one of the DAG, since here only one Partition can come after another,
    *   while for the DAG, you can have multiple subsequents for each node.
    *
    *   \param parts_json nl::json object extracted from the .json configuration file,
    *                    containing the partitions of a specific Component
    *   \return a std::map that associates an index to each Partition (identified by its name)
    */
    std::map<size_t, std::string> find_order_parts(const nl::json& parts_json) const;

  private:

    /** Object to represent the Directed Acyclic Graph (DAG) */
    DAG dag;

    /** Incoming workload of the DAG */
    LoadType lambda;

    /** Object to keep track of the execution time */
    TimeType time;

    /** Vector storing all the components in a ordered way */
    std::vector<Component> components;

    /** Hash map from Component name to the assigned index.
    *   Actually it is the same to the one stored in the dag, but it makes sense
    *   to have it also here.
    */
    std::unordered_map<std::string, std::size_t>
    comp_name_to_idx;

    /** Ordered map from index to component name.
    *   It is useful when there is the need to loop on components
    *		following the execution order.
    *   Actually it is equal to the one stored in the DAG, but it makes sense
    *   to have it also here.
    */
    std::map<size_t, std::string>
    idx_to_comp_name;

    /** Map to obtain a Partition's index in the Component.partitions vector knowing its name
    *   and its Component name: the key is comp_name+part_name.
    */
    std::unordered_map<std::string, std::size_t>
    part_name_to_part_idx;

    /** Object to store the computational layers (defined in Resources.hpp) for each Resource type.
    *
    *   cls[i][j] contains the j-th ComputationalLayer of type i (0,1 or 2 i.e.
    *   ResourceType::Edge, ResourceType::VM or ResourceType::Faas)
    */
    std::vector<std::vector<ComputationalLayer>> cls;

    /** Object to store map from computational layers to index.
    *
    *   cl_name_to_idx[i] store the maps from layers containing resources out of
    *   type i (0,1 or 2 i.e. ResourceType::Edge, ResourceType::VM or
    *   ResourceType::Faas) to the correspondent indexed.
    */
    std::vector<std::unordered_map<std::string, size_t>>
    cl_name_to_idx;

    /** Object storing all the resources */
    AllResources all_resources;

    /** Map to obtain a Resource's type and index in AllResources.edge_resources or
    *   AllResources.VM_resources or AllResources.Faas_resources, depending on its type,
    *   knowing its name.
    */
    std::unordered_map<std::string, std::pair<ResourceType, std::size_t>>
    res_name_to_type_and_idx;

    /** For each [ResourceType::Edge/VM, ComputationalLayer idx] save a pair<bool, size_t>
    *   to indicate whether the specific layer has been selected in a solution
    *   or not.
    *
    *   This is needed at Run-time, to keep track of the selected solution at
    *   Design-Time: if in a layer I select a resource at design-time, at run-time
    *   I can't choose another resource at that layer. FaaS layer are not tracked,
    *   since they are all located in a single one.
    */
    DTSelectedResourcesType dt_selected_resources;

    /** Object storing the compatibility matrix */
    CompatibilityMatrixType compatibility_matrix;

    /** Vector storing all the network domains (defined in NetworkTechnology.hpp) */
    std::vector<NetworkDomain> network_domains;

    /** Map to obtain the indexes of the network domains (in the SystemData.network_domains vector)
    *   that contain a certain ComputationalLayer (identified by its name)
    */
    std::unordered_map<std::string, std::vector<std::size_t>> cl_to_network_domains;

    /** Vector storing the LocalConstraint constraints */
    std::vector<LocalConstraint> local_constraints;

    /** Vector storing the GlobalConstraint constraints */
    std::vector<GlobalConstraint> global_constraints;

    /** Map to obtain a GlobalConstraint's index in the SystemData.global_constraints vector
    *   knowing its name (that corresponds to the path's name)
    */
    std::unordered_map<std::string, std::size_t> gc_name_to_idx;

  public:

    /** Default destructor */
    ~SystemData() = default;

};

// ---------------------------------------------------------------------------
// Template definitions
//----------------------------------------------------------------------------

template<ResourceType Type>
void
SystemData::initialize_resources(const nl::json& resources_json)
{
  std::size_t res_idx = 0;

  for(const auto& [cl, data] : resources_json.items())
  {
    this->cls[ResIdxFromType(Type)].emplace_back(cl, Type);
    cl_name_to_idx[ResIdxFromType(Type)].emplace(
      cl,
      cl_name_to_idx[ResIdxFromType(Type)].size()
    );

    for(const auto& [res_name, res_data] : data.items())
    {
      std::string description = "No description available";
      std::size_t n_cores = 1;

      if(res_data.contains("description"))
      {
        description = res_data.at("description").template get<std::string>();
      }

      if(res_data.contains("n_cores"))
      {
        n_cores = res_data.at("n_cores").template get<size_t>();
      }

      Resource<Type> res(
        static_cast<std::string>(res_name),
        description,
        static_cast<std::string>(cl),
        res_data.at("cost").template get<double>(),
        res_data.at("memory").template get<double>(),
        res_data.at("number").template get<size_t>(),
        n_cores
      );
      this->res_name_to_type_and_idx.emplace(
        res_name,
        std::make_pair(Type, res_idx)
      );
      this->cls[ResIdxFromType(Type)].back().add_resource(res_idx++);
      this->all_resources.add_resource(std::move(res));
    }
  }
}

template<>
inline
void
SystemData::initialize_resources<ResourceType::Faas>
(const nl::json& faas_json)
{
  std::size_t res_idx = 0;
  double transition_cost = 0;

  for(const auto& [key, data] : faas_json.items())
  {
    // Necessary to deal with both old and new version of json files
    if(key == "transition_cost")
    {
      transition_cost = data.template get<double>();
      continue;
    }

    // key is really a computationallayers!

    this->cls[ResIdxFromType(ResourceType::Faas)].emplace_back(key, ResourceType::Faas); // only one computational layer for faas!
    cl_name_to_idx[ResIdxFromType(ResourceType::Faas)].emplace(
      key,
      cl_name_to_idx[ResIdxFromType(ResourceType::Faas)].size()
    );

    for(const auto& [res_name, res_data] : data.items())
    {
      // Necessary to deal with both old and new version of json files
      if(static_cast<std::string>(res_name) == "transition_cost")
      {
        transition_cost = res_data.template get<double>() ;
        continue;
      }

      // At this point I really have Faas resources
      std::string description = "No description available";

      if(res_data.contains("description"))
      {
        description = res_data.at("description").template get<std::string>();
      }

      Resource<ResourceType::Faas> res(
        static_cast<std::string>(res_name),
        description,
        static_cast<std::string>(key),
        res_data.at("cost").template get<double>(),
        res_data.at("memory").template get<double>(),
        transition_cost,
        res_data.at("idle_time_before_kill").template get<double>()
      );
      this->res_name_to_type_and_idx.emplace(
        res_name,
        std::make_pair(ResourceType::Faas, res_idx)
      );
      this->cls[ResIdxFromType(ResourceType::Faas)].back().add_resource(res_idx++);
      this->all_resources.add_resource(std::move(res));
    }
  }
}

} // namespace Space4AI

#endif /* SYSTEMDATA_HPP_ */
