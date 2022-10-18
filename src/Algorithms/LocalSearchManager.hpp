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
* \file LocalSearchManager.hpp
*
* \brief Local Search manager class
*
* \author Randeep Singh
*/

#ifndef LOCAL_SEARCH_MANAGER_HPP
#define LOCAL_SEARCH_MANAGER_HPP

#include "src/Algorithms/LocalSearch.hpp"
#include "src/Solution/EliteResult.hpp"

namespace Space4AI
{
class LocalSearchManager
{
  static
  std::vector<LocalSearch> // for now, but maybe it's better EliteResult ...
  local_search(
    const EliteResult& solutions_,
    size_t max_it,
    bool reproducibility,
    const System& system,
    const SelectedResources& selected_resources = SelectedResources());

  private:

    const System& system;

    const EliteResult& rg_elite_result;

    const SelectedResources& selected_resources;

    EliteResult ls_elite_result;

    std::vector<LocalSearch> ls_vec;

    bool reproducibility;

    size_t max_it;
}

} //namespace Space4AI

#endif // LOCAL_SEARCH_MANAGER_HPP
