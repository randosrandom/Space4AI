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
* \file PerformancePredictors.cpp
*
* \brief definition of the methods of the class Pacsltk
*
* \author Randeep Singh
*/

#include <cpr/cpr.h>

#include "external/chrono/chrono.hpp"
#include "src/Performance/PerformancePredictors.hpp"
#include "src/Logger.hpp"


namespace Space4AI
{

TimeType
Pacsltk::predict(
  LoadType part_lambda,
  TimeType demandWarm, TimeType demandCold, TimeType idle_time_before_kill
)
{
  nlohmann::json data;

  data["arrival_rate"] = part_lambda;
  data["warm_service_time"] = demandWarm;
  data["cold_service_time"] = demandCold;
  data["idle_time_before_kill"] = idle_time_before_kill;
 
  std::string url = HOST;
  url += "/pacsltk";

  cpr::Response r = cpr::Post(
    cpr::Url{url},
    cpr::Body{nlohmann::to_string(data)},
    cpr::Header{{"Content-Type", "text/plain"}});

  if(r.status_code != 200)
  {
    Logger::Error("Something went wrong in communication with Web Server in Pacsltk predict");
    throw std::runtime_error("Something went wrong in communication with Web Server in Pacsltk predict");
  }

  return std::stod(r.text);

}

} // namespace Space4AI
