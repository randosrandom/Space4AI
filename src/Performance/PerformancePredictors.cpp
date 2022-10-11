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
* \author Giulia Mazzilli
*/

#include "src/Performance/PerformancePredictors.hpp"

#include "external/chrono/chrono.hpp"

namespace Space4AI
{
TimeType
Pacsltk::predict(
  LoadType part_lambda,
  TimeType demandWarm, TimeType demandCold, TimeType idle_time_before_kill
)
{
  TimeType demand = pacsltk_predictor(
      part_lambda, demandWarm, demandCold, idle_time_before_kill
    ).cast<TimeType>();
  return demand;
}
} // namespace Space4AI
