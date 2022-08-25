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
    Timings::Chrono c;
    c.start();
    TimeType demand = pacsltk_predictor(
      part_lambda, demandWarm, demandCold, idle_time_before_kill
    ).cast<TimeType>();
    time_spent += c.wallTimeNow() * 1e-6;
    return demand;
  }
} // namespace Space4AI
