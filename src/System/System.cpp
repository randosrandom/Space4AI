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
* \file System.cpp
*
* \brief Define the methods of the class System.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#include "src/System/System.hpp"
#include "src/Solution/Solution.hpp"

namespace Space4AI
{
void
System::read_configuration_file(const std::string& system_file, LoadType lambda_, double energy_cost_pct_)
{
  std::ifstream file(system_file);
  nl::json configuration_file;

  if(!file)
  {
    std::string err_message = "Cannot open: " + system_file + " json file";
    Logger::Error(err_message.c_str());
    throw std::runtime_error(err_message);
  }
  else
  {
    Logger::Info("****** READING CONFIGURATION FILE: " + system_file + " ... ******");
    file >> configuration_file;
  }

  Logger::Info("****** SYSTEM DATA ... ******");
  this->system_data.read_json(configuration_file, lambda_, energy_cost_pct_);
  Logger::Info("********** DONE! **********");

  if(configuration_file.contains("Performance"))
  {
//#warning Actually also here you can have only static models, but it depends on the PerfModel
//At the moment I just set DynamicPerfModels to true
    // this->dynamicPerfModels = true;
    Logger::Info("****** READING PERFORMANCE MODELS... ******");
    this->initialize_performance(configuration_file.at("Performance"));
    Logger::Info("********** DONE! **********");
  }
  else
  {
    Logger::Error("*System::read_configuration_file(...)*: Performance field (or DemandMatrix field, for old configuration) not present in json file");
    throw std::invalid_argument("*System::read_configuration_file(...)*: Performance field (or DemandMatrix field, for old configuration) not present in json file");
  }
}

void
System::initialize_performance(const nl::json& performance_json)
{
  DemandEdgeVMType all_demands;
  all_demands.reserve(this->system_data.components.size());

  for(const auto& [idx, comp] : system_data.idx_to_comp_name)
  {
    const auto& comp_data = performance_json.at(comp);
    const auto& partitions = system_data.components[system_data.comp_name_to_idx[comp]].get_partitions();
    // create and initialize demands_edge_vm_temp
    DemandEdgeVMType::value_type demands_edge_vm_temp(2); // "2" because it's only needed for Edge and VM

    for(size_t i = 0; i < demands_edge_vm_temp.size(); ++i)
    {
      demands_edge_vm_temp[i].resize(partitions.size());

      for(auto& res_vec : demands_edge_vm_temp[i])
        res_vec.resize(
          system_data.all_resources.get_number_resources(i),
          std::numeric_limits<TimeType>::quiet_NaN()
        );
    }

    // create and initialize perf_temp
    PerformanceType::value_type perf_temp(
      ResIdxFromType(ResourceType::Count)
    );

    for(std::size_t i = 0; i < perf_temp.size(); ++i)
    {
      perf_temp[i].resize(partitions.size());

      for(auto& res_vec : perf_temp[i])
      {
        res_vec.resize(system_data.all_resources.get_number_resources(i));
      }
    }

    for(const auto& [part, part_data] : comp_data.items())
    {
      for(const auto& [res, perf_data] : part_data.items())
      {
        const std::size_t comp_idx = system_data.comp_name_to_idx.at(comp);
        const auto res_type_idx = ResIdxFromType(system_data.res_name_to_type_and_idx.at(res).first);
        const std::size_t part_idx = system_data.part_name_to_part_idx.at(comp + part);
        const auto res_idx = system_data.res_name_to_type_and_idx.at(res).second;

        if(system_data.compatibility_matrix[comp_idx][res_type_idx][part_idx][res_idx])
        {
          const std::string model = perf_data.at("model").get<std::string>();
          perf_temp[res_type_idx][part_idx][res_idx] =
            create_PE(model, perf_data, system_data, comp_idx, part_idx, res_idx);

          // THINK ABOUT HOW TO AVOID THIS ...
          if(model == "QTedge" || model == "QTcloud")
          {
            demands_edge_vm_temp[res_type_idx][part_idx][res_idx] = perf_data.at("demand").get<TimeType>();
          }
        }
        else // non compatible
        {
          const std::string err_message =
            "In System::initialize_performance(...) error in allocation of performance for \
              incompatible resource: " + res + "and component " + comp + "with partition " + part + "\n";
          Logger::Error(err_message);
          throw std::logic_error(err_message);
        }
      }
    }

    this->performance.push_back(std::move(perf_temp));
    all_demands.push_back(std::move(demands_edge_vm_temp));
  }

  QTPE::set_all_demands(std::move(all_demands));
}

} // namespace Space4AI
