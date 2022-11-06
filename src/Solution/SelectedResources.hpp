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
* \file SelectedResources.hpp
*
* \brief Store the selected resources at design-time.
*
* \author Randeep Singh
*/

#ifndef SELECTED_RESOURCES_HPP
#define SELECTED_RESOURCES_HPP

#include "src/TypeTraits.hpp"

namespace Space4AI
{
class Solution; // forward declaration

/** class to store the selected resources on a Solution at design-time */
class SelectedResources
{
  friend class Solution;

public:

  /** selected_edge getter */
  const std::vector<size_t>&
  get_selected_edge() const { return selected_edge; }

  /** selected_vms getter */
  const std::vector<size_t>&
  get_selected_vms() const { return selected_vms; }

  /** selected_vms_vy_cl getter */
  const SelectedVMType&
  get_selected_vms_by_cl() const { return selected_vms_by_cl; }

private:

  /** Store the selected edge resources in the solution.
  *
  *   selected[i] = num, if edge device with index i has been selected in the
  *   solution with cluster size = num:
  *
  */
  std::vector<size_t> selected_edge;

  /** Store the selected vms resources in the solution.
  *
  *   selected[i] = num, if the vm device with index i has been selected in the
  *   solution with cluster size = num;
  *
  */
  std::vector<size_t> selected_vms;

  /** Store the selected VMs by computational layer */
  SelectedVMType selected_vms_by_cl;

};

} // namespace Space4AI

#endif // SELECTED_RESOURCES_HPP
