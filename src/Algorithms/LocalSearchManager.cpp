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
  LocalSearchManager::LocalSearchManager(
    const EliteResult& rg_elite_result_,
    const System& system_,
    bool reproducibility_,
    size_t max_it_):
    LocalSearchManager(rg_elite_result_, system_, reproducibility_, max_it_, SelectedResources())
  {}

  LocalSearchManager::LocalSearchManager(
    const EliteResult& rg_elite_result_,
    const System& system_,
    bool reproducibility_,
    size_t max_it_,
    const SelectedResources& curr_rt_sol_sel_res_):
    rg_elite_result(rg_elite_result_), system(system_),
    reproducibility(reproducibility_), max_it(max_it_),
    curr_rt_sol_selected_resources(curr_rt_sol_sel_res_),
    ls_elite_result(EliteResult(rg_elite_result.get_size()))
  {}

  void
  LocalSearchManager::run()
  {
    Logger::Info("Starting LocalSearch...");
    const auto& rg_sols = rg_elite_result.get_solutions();
    this->ls_vec.resize(rg_sols.size(), LocalSearch(&system, &curr_rt_sol_selected_resources));

    // MY PRAGMA
    for(size_t i=0; i<rg_sols.size(); ++i)
    {
      LocalSearch ls_temp(rg_sols[i], &system, &curr_rt_sol_selected_resources);
      ls_temp.run(max_it, reproducibility);
      ls_vec[i] = std::move(ls_temp);
      ls_elite_result.add(std::move(ls_vec[i].curr_sol));
    }
  }
} // namespace Space4AI
