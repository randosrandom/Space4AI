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

inline
std::unique_ptr<BasePerformanceModel>
create_PE(
  const std::string& model, const nl::json& perf_json,
  const SystemData& system_data,
  size_t comp_idx = std::numeric_limits<size_t>::quiet_NaN(),
  size_t part_idx = std::numeric_limits<size_t>::quiet_NaN(),
  size_t res_idx = std::numeric_limits<size_t>::quiet_NaN()
)
{
  if(model == "QTedge" || model == "QTcloud")
  {
    return std::make_unique<QTPE>(model, true, perf_json.at("demand").get<TimeType>());
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
