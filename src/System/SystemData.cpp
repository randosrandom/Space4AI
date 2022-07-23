/**
* \file SystemData.hpp
*
* \brief Defines the methods of SystemData class
*
* \author rando98
* \author giuliamazzeellee
*/

#include "SystemData.hpp"
#include "Logger.hpp"

namespace Space4AI
{

  void
  SystemData::ReadJson(const nl::json& configuration_file)
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

    if(configuration_file.contains("Lambda"))
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

    if(configuration_file.contains("Components"))
    {
      Logger::Info("Reading components...");
      this->InitializeComponents(configuration_file.at("Components"));
      Logger::Info("Done!");
    }
    else
    {
      Logger::Error("Cannot find Components json file");
      throw std::invalid_argument("Cannot find Components json");
    }

    Logger::Info("Resizing cls...");
    this->cls.resize(index(ResourceType::Count));
    Logger::Info("Done!");

    bool at_least_one_resource_type = false;

    if(configuration_file.contains("EdgeResources"))
    {
      Logger::Info("Reading EdgeResources...");
      this->InitializeResources<ResourceType::Edge>(configuration_file.at("EdgeResources"));
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
      this->InitializeResources<ResourceType::VM>(configuration_file.at("CloudResources"));
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
      this->InitializeResources<ResourceType::Faas>(configuration_file.at("FaaSResources"));
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

    if(configuration_file.contains("CompatibilityMatrix"))
    {
      Logger::Info("Reading CompatibilityMatrix...");
      this->InitializeCompatibilityMatrix(configuration_file.at("CompatibilityMatrix"));
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
      this->InitializeLocalConstraints(configuration_file.at("LocalConstraints"));
      Logger::Info("Done!");
    }
    else
    {
      Logger::Info("No Local Constraints provided in json file. Initializing them with +inf");
      this->InitializeLocalConstraints(nl::basic_json());
    }

    if(configuration_file.contains("GlobalConstraints"))
    {
      Logger::Info("Reading GlobalConstraints...");
      this->InitializeGlobalConstraints(configuration_file.at("GlobalConstraints"));
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
      this->InitializeNetworkTechnology(configuration_file.at("NetworkTechnology"));
      Logger::Info("Done!");
    }
    else
    {
      Logger::Error("Cannot find Network Technology in json file");
      throw std::invalid_argument("Cannot find Network Technology");
    }

  }

  void
  SystemData::InitializeComponents(const nl::json& components_json)
  {
    std::string debug_message;

    // traverse components in order
    for(const auto& [comp_idx, comp]: idx_to_comp_name)
    {
      const auto& deploys = components_json.at(comp);

      debug_message = "Initializing component " + comp;
      Logger::Debug(debug_message);

      if(deploys.size() == 0)
      {
        debug_message = "Component "+ comp + " has no deployments";
        Logger::Error(debug_message);
        throw std::runtime_error("Component" + comp + "has no deployments");
      }

      std::vector<Deployment> deployments;
      std::vector<Partition> partitions;

      const auto& input_edges = dag.InputEdges(comp_idx);

      std::vector<std::size_t> input_comps;

      for(std::size_t i = 0; i<input_edges.size(); ++i)
      {
        if(input_edges[i]>0)
        {
          input_comps.emplace_back(i);
        }
      }

      LoadType lambda_input_comp = std::numeric_limits<LoadType>::quiet_NaN(); // lambda^i in the paper (formula (1))

      if(input_comps.size()==0) // first node
      {
        if(comp_idx != 0)
        {
          debug_message = "Something went work in ordering: in InitializeComponents first component does not have '0' index";
          Logger::Error(debug_message);
          throw std::runtime_error("Something went work in ordering: in InitializeComponents first component does not have '0' index");
        }

        Logger::Debug("** Initializing first node...");

        lambda_input_comp = this->lambda;
      }
      else // note the first node
      {
        Logger::Debug("** Initializing other node...");
        LoadType sum = 0.;

        // compute the load factor (lambda_comp) of the component comp
        for(const auto& i: input_comps)
        {
          ProbType prob = input_edges[i];
          LoadType ll = (this->components)[i].get_comp_lambda();
          sum += prob*ll;
        }

        lambda_input_comp = sum;
      }

      // loop over deployments
      for (const auto& [dep, parts]: deploys.items())
      {
        debug_message = "**** Initializing deployment "+ dep + " of component " + comp;
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
        std::map<size_t, std::string> ordered_parts = FindOrderParts(parts);

        // loop on ordered partitions
        for(const auto& [idx, part]: ordered_parts)
        {
          const auto& data = parts.at(part);

          debug_message =  "****** Initializing partition " + part + " of component " + comp;
          Logger::Debug(debug_message);

          if(part_lambda > -1)
          {
            ProbType prob = data.at("early_exit_probability").get<ProbType>();
            part_lambda *= (1-prob);
          }
          else
            part_lambda = lambda_input_comp;

          part_idx.emplace_back(partitions.size());

          partitions.emplace_back(
            static_cast<std::string>(part),
            data.at("memory").get<double>(),
            part_lambda,
            data.at("early_exit_probability").get<double>(),
            data.at("next").get<std::string>(),
            data.at("data_size").get<double>()
          );

          part_name_to_part_idx.emplace(
            comp+part, part_idx.back()
          );
          Logger::Debug("****** Done!");
        }

        deployments.emplace_back(dep, part_idx);
        Logger::Debug("**** Done!");
      }

      this->components.emplace_back(
        comp,
        std::move(deployments),
        std::move(partitions),
        this->lambda
      );

      Logger::Debug("** Done!");
    } // end loop on components

    Logger::Debug("Done!");

  } // end function

  std::map<size_t, std::string>
  SystemData::FindOrderParts(const nl::json& parts_json) const
  {
    std::map<size_t, std::string> idx_to_part_name;

    std::unordered_map<std::string, size_t> all_parts;
    std::unordered_map<std::string, std::string> parts_next;

    // read names of all partition
    for(const auto& [part_name, part_data] : parts_json.items())
    {
      all_parts.emplace(part_name, all_parts.size());
      const std::string next_part = part_data.at("next").get<std::string>();
      if(comp_name_to_idx.count(next_part) == 0) // if next is not the final partition (next is not a component)
      {
        parts_next.emplace(next_part, part_name);
      }
    }

    // find root
    std::string root_part;
    for(const auto& [part_name, part_idx]: all_parts)
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
        throw std::logic_error("Error in FindOrderParts: " + root_part + " is not a partition");

      idx_to_part_name.emplace(i, root_part);
      root_part = parts_json.at(root_part).at("next").get<std::string>();
    }

    return idx_to_part_name;
  }

  void
  SystemData::InitializeCompatibilityMatrix(const nl::json& comp_matrix_json)
  {
    std::string debug_message;

    for(const auto& [idx, comp]: idx_to_comp_name)
    {
      const auto& comp_data = comp_matrix_json.at(comp);

      debug_message = "Initializing compatibility_matrix for component "+comp;
      Logger::Debug(debug_message);
      //resize the second level of the compatibility matrix with the number of resource types
      CompatibilityMatrixType::value_type comp_temp_matrix(
        index(ResourceType::Count)
      );

      // get the partitions of the current component
      const auto& partitions=components[comp_name_to_idx.at(comp)].get_partitions();

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
          res_vec.resize(resources.get_number_resources(i));
        }
      }

      for(const auto& [part, part_data]: comp_data.items())
      {
        for(const auto& res: part_data)
        {
          //select compatible resources
          const auto& res_info = res_name_to_type_and_idx[res.get<std::string>()];

          comp_temp_matrix [index(res_info.first)]
          [part_name_to_part_idx[comp+part]]
          [res_info.second] = 1;
        }
      }
      // assuming the same order in the json for components in "compatibility matrix" wrt to "components"
      this->compatibility_matrix.push_back(std::move(comp_temp_matrix));
      Logger::Debug("Done!");
    }
  }

  void
  SystemData::InitializeNetworkTechnology(const nl::json& network_technology_json)
  {
    std::string debug_message;

    for(const auto& [name, value]: network_technology_json.items())
    {
      debug_message = "Initializing network domain "+name;
      Logger::Debug(debug_message);
      std::vector<std::string> cls_names;

      for(const auto& cl : value.at("computationallayers"))
      {
        cls_names.push_back(cl.get<std::string>());
        cl_to_network_domains[cl.get<std::string>()].push_back(network_domains.size());
      }

      network_domains.emplace_back(
        name,
        cls_names,
        value.at("AccessDelay").get<TimeType>(),
        value.at("Bandwidth").get<double>()
      );
      Logger::Debug("Done!");
    }

    // order the cl_to_network_domains
    for(auto& [name, vec]: cl_to_network_domains)
      std::sort(vec.begin(), vec.end());
  }

  void
  SystemData::InitializeLocalConstraints(const nl::json& local_constraints_json)
  {
    #warning this->get_comp_name_to_idx() is not working ... WHY?
    //const auto& comp_name_to_idx = dag.get_comp_name_to_idx();

    for(size_t i=0; i<components.size(); ++i)
    {
      local_constraints.emplace_back(
        i,
        std::numeric_limits<TimeType>::infinity()
      );
    }
    if(!local_constraints_json.empty())
    {
      for(const auto& [comp, data]: local_constraints_json.items())
      {
        local_constraints[comp_name_to_idx.at(comp)] = std::move(LocalConstraint(comp_name_to_idx.at(comp), data.at("local_res_time").get<TimeType>()));
      }
    }
  }

  void
  SystemData::InitializeGlobalConstraints(const nl::json& global_constraints_json)
  {
    std::size_t idx = 0;
    std::string debug_message;

    for(const auto& [name, data]: global_constraints_json.items())
    {
      debug_message = "Initializing global constraints for path "+name;
      Logger::Debug(debug_message);
      std::vector<std::size_t> comp_idxs;

      for(const auto& comp: data.at("components"))
          comp_idxs.push_back(comp_name_to_idx.at(comp.get<std::string>()));

      this->gc_name_to_idx.emplace(name,idx);

      this->global_constraints.emplace_back(
        name,
        comp_idxs,
        data.at("global_res_time").get<TimeType>()
      );
      Logger::Debug("Done!");
    }
  }

} //namespace Space4AI
