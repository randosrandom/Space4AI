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
public:

    LocalSearchManager(
      const EliteResult& rg_elite_result_,
      const System& system_,
      bool reproducibility_,
      size_t max_it_,
      size_t max_num_sols);

    LocalSearchManager(
      const EliteResult& rg_elite_result_,
      const System& system_,
      bool reproducibility_,
      size_t max_it_,
      size_t max_num_sols,
      const SelectedResources& fixed_edge_and_curr_rt_vms_);

    void
    run();

    const EliteResult&
    get_ls_elite_result() const {return ls_elite_result;};

    const std::vector<LocalSearch>&
    get_ls_vec() const {return ls_vec;};


  private:

    /** elite_result output of the Random Greedy*/
    const EliteResult& rg_elite_result;

    const System& system;

    bool reproducibility;

    size_t max_it;

    /** contains the selected_resources in the previous solution, empty if working at design time */
    const SelectedResources fixed_edge_and_curr_rt_vms;

    /** EliteResult after the Local Search */
    EliteResult ls_elite_result;

    /** Local Search data for each solution */
    std::vector<LocalSearch> ls_vec;
};

} //namespace Space4AI

#endif // LOCAL_SEARCH_MANAGER_HPP
