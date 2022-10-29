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
* \file SystemData.cpp
*
* \brief Defines the methods of the class SystemData.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#include "src/System/SystemData.hpp"
#include "src/Logger.hpp"

namespace Space4AI
{

void
SystemData::read_json(const nl::json& configuration_file, LoadType lambda_)
{
  if(configuration_file.contains("DirectedAcyclicGraph") && configuration_file.contains("Components"))
  {
    Logger::Info("Reading DAG...");
    this->dag.read_from_file(configuration_file.at("DirectedAcyclicGraph"), configuration_file.at("Components"));
    this->comp_name_to_idx = dag.get_comp_name_to_idx();
    this->idx_to_comp_name = dag.get_idx_to_comp_name();
    Logger::Info("Done!");
  }
  else
  {
    Logger::Error("Cannot find DirectedAcyclicGraph or Components in json file");
    throw std::invalid_argument("Cannot find DirectedAcyclicGraph or Components in json");
  }

  if(!std::isnan(lambda_))
  {
    this->lambda = lambda_;

  }
  else if(configuration_file.contains("Lambda"))
  {
    Logger::Info("Reading lambda...");
    this->lambda = configuration_file.at("Lambda").get<LoadType>();
    Logger::Info("Done!");
  }
  else
  {
    Logger::Error("Cannot find Lambda in json file");
    throw std::invalid_argument("Cannot find Lambda in json");
  }

  if(configuration_file.contains("Time"))
  {
    Logger::Info("Reading time...");
    this->time = configuration_file.at("Time").get<TimeType>();
    Logger::Info("Done!");
  }
  else
  {
    Logger::Error("Cannot find Time in json file");
    throw std::invalid_argument("Cannot find Time in json");
  }

  Logger::Info("Resizing cls...");
  this->cls.resize(ResIdxFromType(ResourceType::Count));
  this->cl_name_to_idx.resize(ResIdxFromType(ResourceType::Count));
  Logger::Info("Done!");
  bool at_least_one_resource_type = false;

  if(configuration_file.contains("EdgeResources"))
  {
    Logger::Info("Reading EdgeResources...");
    this->initialize_resources<ResourceType::Edge>(configuration_file.at("EdgeResources"));
    Logger::Info("Done!");
    at_least_one_resource_type = true;
  }
  else
  {
    Logger::Info("No EdgeResources in json file");
  }

  if(configuration_file.contains("CloudResources"))
  {
    Logger::Info("Reading CloudResources...");
    this->initialize_resources<ResourceType::VM>(configuration_file.at("CloudResources"));
    Logger::Info("Done!");
    at_least_one_resource_type = true;
  }
  else
  {
    Logger::Info("No CloudResources in json file");
  }

  if(configuration_file.contains("FaaSResources"))
  {
    Logger::Info("Reading FaaSResources...");
    this->initialize_resources<ResourceType::Faas>(configuration_file.at("FaaSResources"));
    Logger::Info("Done!");
    at_least_one_resource_type = true;
  }
  else
  {
    Logger::Info("No FaasResources in json file");
  }

  if(!at_least_one_resource_type)
  {
    Logger::Error("No resource provided in json file");
    throw std::logic_error("No resource provided in json file");
  }

  // read components after the resources so I can resize memory field of partitions without pain...
  if(configuration_file.contains("Components"))
  {
    Logger::Info("Reading components...");
    this->initialize_components(configuration_file.at("Components"));
    Logger::Info("Done!");
  }
  else
  {
    Logger::Error("Cannot find Components json file");
    throw std::invalid_argument("Cannot find Components json");
  }

  if(configuration_file.contains("CompatibilityMatrix"))
  {
    Logger::Info("Reading CompatibilityMatrix...");
    this->initialize_compatibility_matrix(configuration_file.at("CompatibilityMatrix"));
    Logger::Info("Done!");
  }
  else
  {
    Logger::Error("Cannot find Compatibility Matrix in json file");
    throw std::invalid_argument("Cannot find Compatibility Matrix");
  }

  if(configuration_file.contains("LocalConstraints"))
  {
    Logger::Info("Reading LocalConstraints...");
    this->initialize_local_constraints(configuration_file.at("LocalConstraints"));
    Logger::Info("Done!");
  }
  else
  {
    Logger::Info("No Local Constraints provided in json file. Initializing them with +inf");
    this->initialize_local_constraints(nl::basic_json());
  }

  if(configuration_file.contains("GlobalConstraints"))
  {
    Logger::Info("Reading GlobalConstraints...");
    this->Initialize_global_constraints(configuration_file.at("GlobalConstraints"));
    Logger::Info("Done!");
  }
  else
  {
    Logger::Error("Cannot find Global Constraints in json file");
    throw std::invalid_argument("Cannot find Global Constraints");
  }

  if(configuration_file.contains("NetworkTechnology"))
  {
    Logger::Info("Reading NetworkTechnology...");
    this->initialize_network_technology(configuration_file.at("NetworkTechnology"));
    Logger::Info("Done!");
  }
  else
  {
    Logger::Error("Cannot find Network Technology in json file");
    throw std::invalid_argument("Cannot find Network Technology");
  }
}

void
SystemData::initialize_components(const nl::json& components_json)
{
  const size_t num_edge = all_resources.get_number_resources(ResIdxFromType(ResourceType::Edge));
  const size_t num_vms = all_resources.get_number_resources(ResIdxFromType(ResourceType::VM));
  const size_t num_faas = all_resources.get_number_resources(ResIdxFromType(ResourceType::Faas));

  std::string debug_message;

  // traverse components in order
  for(const auto& [comp_idx, comp] : idx_to_comp_name)
  {
    const auto& deploys = components_json.at(comp);
    debug_message = "Initializing component " + comp;
    Logger::Debug(debug_message);

    if(deploys.size() == 0)
    {
      debug_message = "Component " + comp + " has no deployments";
      Logger::Error(debug_message);
      throw std::runtime_error("Component" + comp + "has no deployments");
    }

    std::vector<Deployment> deployments;
    std::vector<Partition> partitions;
    const auto& input_edges = dag.input_edges(comp_idx);
    std::vector<std::size_t> input_comps;

    for(std::size_t i = 0; i < input_edges.size(); ++i)
    {
      if(input_edges[i] > 0)
      {
        input_comps.emplace_back(i);
      }
    }

    LoadType comp_lambda = std::numeric_limits<LoadType>::quiet_NaN(); // lambda^i in the paper (formula (1))

    if(input_comps.size() == 0) // first node
    {
      if(comp_idx != 0)
      {
        debug_message = "Multiple entry nodes";
        Logger::Warn(debug_message);
      }

      Logger::Debug("** Initializing entry nodes...");
      comp_lambda = this->lambda;
    }
    else // not the first node
    {
      Logger::Debug("** Initializing subsequent nodes...");
      LoadType sum = 0.;

      // compute the load factor (comp_lambda) of the component comp
      for(const auto& i : input_comps)
      {
        const ProbType prob = input_edges[i];
        const  LoadType ll = (this->components)[i].get_comp_lambda();
        sum += prob * ll;
      }

      comp_lambda = sum;
    }

    // loop over deployments
    for(const auto& [dep, parts] : deploys.items())
    {
      debug_message = "**** Initializing deployment " + dep + " of component " + comp;
      Logger::Debug(debug_message);
      LoadType part_lambda = -1;
      std::vector<std::size_t> part_idx;

      // simple check
      if(parts.size() == 0)
      {
        debug_message = "**** Deployment " + dep + " has no partitions";
        Logger::Error(debug_message);
        throw std::runtime_error("Deployment " + dep + " has no partitions");
      }

      // find order of the partition
      std::map<size_t, std::string> ordered_parts = find_order_parts(parts);

      // loop on ordered partitions
      ProbType early_exit_probability = 0.0;
      for(const auto& [idx, part] : ordered_parts)
      {
        const auto& data = parts.at(part);
        debug_message =  "****** Initializing partition " + part + " of component " + comp;
        Logger::Debug(debug_message);

        if(part_lambda > 0)
        {
          part_lambda *= (1 - early_exit_probability);
        }
        else
        {
          part_lambda = comp_lambda;
        }

        early_exit_probability = data.at("early_exit_probability").get<ProbType>(); // early exit probability for the next partition

        part_idx.emplace_back(partitions.size());
        std::unordered_map<size_t, DataType> next_data_sizes;
        if(idx == ordered_parts.size()-1) // last partition
        {
          for(size_t i=0; i<data.at("next").size(); ++i)
          {
            const auto comp_next_name = data.at("next")[i].get<std::string>();
            const auto data_size_next = data.at("data_size")[i].get<DataType>();
            next_data_sizes.emplace(comp_name_to_idx.at(comp_next_name), data_size_next);
          }
        }
        else
        {
          const auto part_next_name = data.at("next")[0].get<std::string>();
          const auto data_size_next = data.at("data_size")[0].get<DataType>();
          next_data_sizes.emplace(part_idx.back()+1, data_size_next);
        }
        partitions.emplace_back(
          static_cast<std::string>(part),
          part_lambda,
          early_exit_probability,
          std::move(next_data_sizes),
          num_edge, num_vms, num_faas);

        part_name_to_part_idx.emplace(comp + part, part_idx.back());
        Logger::Debug("****** Done!");
      }

      deployments.emplace_back(dep, part_idx);
      Logger::Debug("**** Done!");
    }

    this->components.emplace_back(
      comp,
      std::move(deployments),
      std::move(partitions),
      comp_lambda
    );
    Logger::Debug("** Done!");
  } // end loop on components
} // end function

std::map<size_t, std::string>
SystemData::find_order_parts(const nl::json& parts_json) const
{
  std::map<size_t, std::string> idx_to_part_name;
  std::unordered_map<std::string, size_t> all_parts;
  std::unordered_map<std::string, std::string> parts_next;

  // read names of all partition
  for(const auto& [part_name, part_data] : parts_json.items())
  {
    all_parts.emplace(part_name, all_parts.size());
    const auto next_parts_json = part_data.at("next").get<std::vector<std::string>>();

    if(next_parts_json.size() == 0) // last partition of last component
      continue;

    if(comp_name_to_idx.count(next_parts_json.at(0)) == 0) // if next_part is not a component
    {
      parts_next.emplace(next_parts_json.at(0), part_name);
    }
  }

  // find root name (but index is assigned while reading in order)
  std::string root_part;

  for(const auto& [part_name, part_idx] : all_parts)
  {
    if(parts_next.count(part_name) == 0)
    {
      root_part = part_name;
      break;
    }
  }

  // read in order
  for(std::size_t i = 0; i < all_parts.size(); ++i)
  {
    if(all_parts.count(root_part) == 0)
    {
      throw std::logic_error("Error in find_order_parts: " + root_part + " is not a partition");
    }

    idx_to_part_name.emplace(i, root_part);
    if(parts_json.at(root_part).at("next").size()) // =0 for last partition of last component
      root_part = parts_json.at(root_part).at("next")[0].get<std::string>();
  }

  return idx_to_part_name;
}

void
SystemData::initialize_compatibility_matrix(const nl::json& comp_matrix_json)
{
  std::string debug_message;

  // traversing components in order
  for(const auto& [idx, comp] : idx_to_comp_name)
  {
    const auto& comp_data = comp_matrix_json.at(comp);
    debug_message = "Initializing compatibility_matrix for component " + comp;
    Logger::Debug(debug_message);
    //resize the second level of the compatibility matrix with the number of resource types
    CompatibilityMatrixType::value_type comp_temp_matrix(
      ResIdxFromType(ResourceType::Count)
    );
    // get the partitions of the current component
    const auto& partitions = components[comp_name_to_idx.at(comp)].get_partitions();

    for(std::size_t i = 0; i < comp_temp_matrix.size(); ++i)
    {
      // resize the third level of the compatibility matrix with the number of partitions
      // in the current component
      comp_temp_matrix[i].resize(
        partitions.size()
      );

      for(auto& res_vec : comp_temp_matrix[i])
      {
        // resize the fourth level of the compatibility matrix with the number of resources
        // of type i
        res_vec.resize(all_resources.get_number_resources(i));
      }
    }

    for(const auto& [part, part_data] : comp_data.items())
    {
      const auto p_idx = part_name_to_part_idx.at(comp + part);
      auto& partition_p_idx = components[idx].get_partition(p_idx);

      for(const auto& res_and_mem : part_data)
      {
        const auto res = res_and_mem.at("resource");
        const auto memory = res_and_mem.at("memory");
        //select compatible resources
        const auto& res_info = res_name_to_type_and_idx[res.get<std::string>()];
        comp_temp_matrix[ResIdxFromType(res_info.first)]
                        [p_idx]
                        [res_info.second] = 1;
        partition_p_idx.set_memory(memory.get<DataType>(), ResIdxFromType(res_info.first), res_info.second); // set memory
      }
    }

    // I can do the follwing since components are traversed in order!
    this->compatibility_matrix.push_back(std::move(comp_temp_matrix));
  }
}

void
SystemData::initialize_network_technology(const nl::json& network_technology_json)
{
  std::string debug_message;

  for(const auto& [name, value] : network_technology_json.items())
  {
    debug_message = "Initializing network domain " + name;
    Logger::Debug(debug_message);
    std::vector<std::string> cls_names;

    for(const auto& cl : value.at("computationallayers"))
    {
      cls_names.push_back(cl.get<std::string>());
      this->cl_to_network_domains[cl.get<std::string>()].push_back(network_domains.size());
    }

    this->network_domains.emplace_back(
      name,
      cls_names,
      value.at("AccessDelay").get<TimeType>(),
      value.at("Bandwidth").get<double>()
    );
    Logger::Debug("Done!");
  }

  // order the cl_to_network_domains because then I will do a std::intersect
  for(auto& [name, vec] : cl_to_network_domains)
  {
    std::sort(vec.begin(), vec.end());
  }
}

void
SystemData::initialize_local_constraints(const nl::json& local_constraints_json)
{
  for(size_t i = 0; i < components.size(); ++i)
  {
    this->local_constraints.emplace_back(
      i,
      std::numeric_limits<TimeType>::infinity()
    );
  }

  if(!local_constraints_json.empty())
  {
    for(const auto& [comp, data] : local_constraints_json.items())
    {
      this->local_constraints.at(comp_name_to_idx.at(comp)) = LocalConstraint(comp_name_to_idx.at(comp), data.at("local_res_time").get<TimeType>());
    }
  }
}

void
SystemData::Initialize_global_constraints(const nl::json& global_constraints_json)
{
  std::string debug_message;

  for(const auto& [name, data] : global_constraints_json.items())
  {
    debug_message = "Initializing global constraints for path " +  name;
    Logger::Debug(debug_message);
    std::vector<std::size_t> comp_idxs;

    for(const auto& comp : data.at("components"))
    {
      comp_idxs.push_back(comp_name_to_idx.at(comp.get<std::string>()));
    }

    this->gc_name_to_idx.emplace(name, gc_name_to_idx.size());
    this->global_constraints.emplace_back(
      name,
      comp_idxs,
      data.at("global_res_time").get<TimeType>()
    );
  }
}

} //namespace Space4AI
