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
* \file EliteResult.cpp
*
* \brief Defines the methods of the class EliteResult.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#include "src/Solution/EliteResult.hpp"

namespace Space4AI
{
void
EliteResult::print_solution(const System& system, const std::string& path, size_t rank) const
{
  if(get_size() == 0)
  {
    Logger::Warn("*EliteResult::print_solution*: No feasible solution present in EliteResult");
  }
  else if(rank > get_size())
  {
    Logger::Warn("rank bigger than available solutions ... printing worst solution of rank: " + std::to_string(get_size()));
    solutions.back().print_solution(system, path);
  }
  else
  {
    solutions[rank].print_solution(system, path);
  }
}

} //namespace Space4AI
