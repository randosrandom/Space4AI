/**
* \file PerformancePredictors.cpp
*
* \brief definition of the methods of the class Pacsltk
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#include "src/Performance/PerformancePredictors.hpp"

namespace Space4AI
{
  TimeType
  Pacsltk::predict(
    LoadType part_lambda,
    TimeType demandWarm, TimeType demandCold, TimeType idle_time_before_kill
  ) const
  {
    return pacsltk_predictor(
      part_lambda, demandWarm, demandCold, idle_time_before_kill
    ).cast<TimeType>();
  }
} // namespace Space4AI
