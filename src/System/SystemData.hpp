/**
* \file SystemData.hpp
*
* \brief Defines the class to store the system configuration read from json
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef SYSTEMDATA_HPP_
#define SYSTEMDATA_HPP_

#include <fstream>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "Graph/Component.hpp"
#include "Graph/Dag.hpp"
#include "Constraints/GlobalConstraint.hpp"
#include "nlohmann/json.hpp"
#include "Constraints/LocalConstraint.hpp"
#include "NetworkTechnology.hpp"
#include "Resources.hpp"
#include "TypeTraits.hpp"

namespace Space4AI
{
  class System; // forward declaration

  namespace nl = nlohmann;

  /** SystemData
  *
  *   Class to store the data structures that define the system configuration described
  *   in the .json file except for the performance section
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
    *   \param configuration_file json object extracted from system_file
    */
    void
    ReadJson(const nl::json& configuration_file);

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

    /** resources getter */
    const Resources&
    get_resources() const {return resources;}

    /** res_name_to_type_and_idx getter */
    const std::unordered_map<std::string, std::pair<ResourceType, std::size_t>>&
    get_res_name_to_type_and_idx() const {return res_name_to_type_and_idx;}

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

    /** Method to initialize the components
    *
    *   \param components_json json object extracted from configuration_file containing
    *                          the description of the components
    */
    void
    InitializeComponents(const nl::json& components_json);

    /** Method to initialize the resources
    *
    *   \param resources_json json object extracted from configuration_file containing
    *                         the description of the resources
    */
    template<ResourceType Type>
    void
    InitializeResources(const nl::json& resources_json);

    /** Method to initialize the compatibility matrix
    *
    *   \param comp_matrix_json json object extracted from configuration_file containing
    *                           the description of the compatibility matrix
    */
    void
    InitializeCompatibilityMatrix(const nl::json& comp_matrix_json);

    /** Method to initialize the network technology
    *
    *   \param network_technology_json json object extracted from configuration_file containing
    *                                  the description of the network technology
    */
    void
    InitializeNetworkTechnology(const nl::json& network_technology_json);

    /** Method to initialize the local constraints
    *
    *   \param local_constraints_json json object extracted from configuration_file containing
    *                                 the description of the local constraints
    */
    void
    InitializeLocalConstraints(const nl::json& local_constraints_json);

    /** Method to initialize the global constraints
    *
    *   \param global_constraints_json json object extracted from configuration_file containing
    *                                  the description of the global constraints
    */
    void
    InitializeGlobalConstraints(const nl::json& global_constraints_json);

  private:

    /** Method to find the order of the partition in the json, and assign to them
    *   increasing indexes.
    *   Note that this ordering method is different from the
    *   one of the dag, since here only one partition can come after another,
    *   while for the dag, you can have multiple subsequents for each node.
    *
    *   \param part_json json object extracted from the configuration file, containing
    *                    the partitions of a specific component
    */
    std::map<size_t, std::string>
    FindOrderParts(const nl::json& parts_json) const;

  private:

    /** Object to represent the Directed Acyclic Graph */
    DAG dag;

    /** Incoming workload of the DAG */
    LoadType lambda;

    /** Object to keep track of the execution time */
    TimeType time;

    /** Vector storing all the components in a ordered way */
    std::vector<Component> components;

    /** Hash map from component name to the assigned index.
    *   Actually it is the same to the one stored in the dag, but it makes sense
    *   to have it also here for performance efficiency
    */
    std::unordered_map<std::string, std::size_t>
    comp_name_to_idx;

    /** Ordered map from index to component name.
    *   It is useful when there is the need to sometihing in order
    *   Actually it is the same to the one stored in the dag, but it makes sense
    *   to have it also here for performance efficiency
    */
    std::map<size_t, std::string>
    idx_to_comp_name;

    /** Map to obtain a partition's index in the Component.partitions vector knowing its name
    *   and its component name: the key is comp_name+part_name
    */
    std::unordered_map<std::string, std::size_t>
    part_name_to_part_idx;

    /** Object to store the computational layers for each resource type
    *
    *   cls[i][j] contains the j-th computational layer of type i (0,1 or 2 i.e. Edge, VM or Faas)
    */
    std::vector<std::vector<ComputationalLayer>> cls;

    /** Object storing all the resources */
    Resources resources;

    /** Map to obtain a resource's type and index in Resources.edge_resources or
    *   Resources.VM_resources or Resources.Faas_resources, depending on its type, knowing its name
    */
    std::unordered_map<std::string, std::pair<ResourceType, std::size_t>>
    res_name_to_type_and_idx;

    /** Object storing the compatibility matrix */
    CompatibilityMatrixType compatibility_matrix;

    /** Vector storing all the network domains */
    std::vector<NetworkDomain> network_domains;

    /** Map to obtain the indexes of the network domains (in the network_domains vector)
    *   that contain a certain computational layer (identified by its name)
    *   std::vector<std::size_t> should be ordered (since then it'll be used std::intersect)
    */
    std::unordered_map<std::string, std::vector<std::size_t>> cl_to_network_domains;

    /** Vector storing the local constraints */
    std::vector<LocalConstraint> local_constraints;

    /** Vector storing the global constraints */
    std::vector<GlobalConstraint> global_constraints;

    /** Map to obtain a global constraint's index in the global_constraints vector knowing its
    *   name (that corresponds to the path's name)
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
  SystemData::InitializeResources(const nl::json& resources_json)
  {
    std::size_t res_idx = 0;

    for(const auto& [cl, data]: resources_json.items())
    {
      this->cls[index(Type)].emplace_back(cl, Type);

      for(const auto& [res_name, res_data]: data.items())
      {
        std::string description = "No description available";

        std::size_t n_cores = 1;

        if(res_data.contains("description"))
          description = res_data.at("description").template get<std::string>();
        if(res_data.contains("n_cores"))
          n_cores = res_data.at("n_cores").template get<size_t>();

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

        this->cls[index(Type)].back().add_resource(res_idx++);

        this->resources.add_resource(std::move(res));
      }
    }
  }

  template<>
  inline
  void
  SystemData::InitializeResources<ResourceType::Faas>(const nl::json& faas_json)
  {
    std::size_t res_idx = 0;
    double transition_cost = 0;

    for(const auto& [key, data]: faas_json.items())
    {
      // Necessary to deal with both old and new version of json files
      if(key == "transition_cost")
      {
        transition_cost = data.template get<double>();
        continue;
      }

      this->cls[index(ResourceType::Faas)].emplace_back(key, ResourceType::Faas); // only one computational layer for faas!

      for(const auto& [res_name, res_data]: data.items())
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
          description = res_data.at("description").template get<std::string>();

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

        this->cls[index(ResourceType::Faas)].back().add_resource(res_idx++);

        this->resources.add_resource(std::move(res));
      }
    }
  }

} // namespace Space4AI

#endif /* SYSTEMDATA_HPP_ */
