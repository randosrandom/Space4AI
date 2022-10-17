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
* \file LocalSearchManager.cpp
*
* \brief Local Search manager class
*
* \author Randeep Singh
*/

#include "src/Algorithms/LocalSearchManager.hpp"

namespace Space4AI
{
static
std::vector<Solution>
LocalSearchManager::local_search(
  const EliteResult& rg_elite_result,
  size_t max_it,
  bool reproducibility,
  const System& system)
{
  const auto& rg_sols = elite_result.get_solutions();
  std::vector<Solution> ls_sols(rg_sols.size(), Solution(system));
  // MY PRAGMA PARRALELIZATION
  for(size_t i=0; i<rg_sols.size(); ++i)
  {
    LocalSearch ls(rg_sols[i], system, reproducibility);
    ls_sols[i] = std::move(ls.run(max_it));
  }
  return ls_sols;
}

} // namespace Space4AI
