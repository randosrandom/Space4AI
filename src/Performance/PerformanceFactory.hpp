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
* \file PerformanceFactory.hpp
*
* \brief Defines the object factory for the different types of performance models
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef PERFORMANCEFACTORY_HPP_
#define PERFORMANCEFACTORY_HPP_

#include <limits>
#include <memory>
#include <string>

#include "src/Performance/PerformanceModels.hpp"
#include "src/System/SystemData.hpp"

namespace Space4AI
{
/** Object factory for Performance Models.
*
*   \param model Model name of the model
*   \param perf_json  Piece of the system configuration json file
*                     containing information about the specific performance
*                     model.
*   \param system_data  As the name say
*   \param comp_idx   Component index
*   \param part_idx   Partition index
*   \param res_idx    Resource index
*
*   \return unique_ptr to the BasePerformanceModel abstract class
*/
inline
std::unique_ptr<BasePerformanceModel>
create_PE(
  const std::string& model, const nl::json& perf_json,
  const SystemData& system_data,
  size_t comp_idx, size_t part_idx, size_t res_idx
)
{
  if(model == "QTedge" || model == "QTcloud")
  {
    return std::make_unique<QTPE>(model, true);
  }
  else if(model == "PACSLTK")
  {
    return std::make_unique<FaasPacsltkPE>(model, false, perf_json.at("demandWarm").get<TimeType>(), perf_json.at("demandCold").get<TimeType>());
  }

  if(model == "PACSLTKSTATIC") // now I can decide to use PACSLTKSTATIC even for the new version of json!
  {
    return std::make_unique<FaasPacsltkStaticPE>(
        model, false, perf_json.at("demandWarm").get<TimeType>(), perf_json.at("demandCold").get<TimeType>(),
        system_data.get_all_resources().get_resource<ResourceType::Faas>(res_idx).get_idle_time_before_kill(),
        system_data.get_component(comp_idx).get_partition(part_idx).get_part_lambda()
      );
  }

  Logger::Error("In *PerformanceFactory::create_PE(...)* unknown model name passed");
  throw std::invalid_argument("In *PerformanceFactory::create_PE(...)* unknown model name passed");
}

} //namespace Space4AI

#endif /* PERFORMANCEFACTORY_HPP_ */
