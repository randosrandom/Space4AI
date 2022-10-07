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
* \file SystemPE.cpp
*
* \brief Defines the methods of the classes defined in SystemPE.hpp
*
* \author Randeep Singh
*/

#include "src/Performance/SystemPE.hpp"

namespace Space4AI
{
/*
void
SystemPE::compute_local_perfs(
  const System& system,
  const SolutionData& solution_data)
{
  const auto& components = system.get_system_data().get_components();
  local_parts_perf.resize(components.size(), NaN);
  local_parts_delays.resize(components.size(), NaN);
  comp_perfs.resize(components.size(), NaN);

  for(size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx)
  {
    comp_eval(comp_idx, system, solution_data);
  }
}
*/
void
SystemPE::compute_global_perf(
  size_t path_idx,
  const System& system,
  const SolutionData& solution_data)
{
  const auto& global_constraint = system.get_system_data().get_global_constraints()[path_idx];
  path_perfs[path_idx] = 0.0;
  const auto& comp_idxs = global_constraint.get_comp_idxs();
  for(size_t i=0; i<comp_idxs.size()-1; ++i)
  {
    path_perfs[path_idx] += comp_perfs[comp_idxs[i]];

    //DELAYS
    // get the indices of the current and the next component
    const auto curr_comp_idx = comp_idxs[i];
    const auto next_comp_idx = comp_idxs[i+1];
    // used resources
    const auto& used_resources = solution_data.get_used_resources();
    // resource index of the last partition
    const auto [curr_comp_last_part_idx, res1_type_idx, res1_idx] =
      *(used_resources[curr_comp_idx].crbegin());
    //resource index of the first partition
    const auto [next_comp_first_part_idx, res2_type_idx, res2_idx] =
      *(used_resources[next_comp_idx].cbegin());
      // different resources.
    if(res1_type_idx != res2_type_idx || res1_idx != res2_idx)
    {
      const auto& curr_comp = system.get_system_data().get_component(curr_comp_idx);
      const auto data_size = curr_comp.get_partition(curr_comp_last_part_idx).get_data_size();
      comp_delays[curr_comp_idx] = compute_network_delay(
          ResTypeFromIdx(res1_type_idx), res1_idx,
          ResTypeFromIdx(res2_type_idx), res2_idx,
          data_size,
          system);
    }
    else
    {
      comp_delays[curr_comp_idx] = 0.0;
    }
    path_perfs[path_idx] += comp_delays[curr_comp_idx];
  }
  // last component
  path_perfs[path_idx] += comp_perfs[comp_idxs.back()];
}

void
SystemPE::compute_local_perf(
  size_t comp_idx,
  const System& system,
  const SolutionData& solution_data)
{
  const auto& used_resources_comp = solution_data.get_used_resources()[comp_idx];
  local_parts_perfs[comp_idx].resize(used_resources_comp.size(), 0.0);

  const auto& performance_comp = system.get_performance()[comp_idx];
  const auto& partitions_comp = system.get_system_data().get_components()[comp_idx].get_partitions();

  Logger::Debug("Evaluating performance of component" + std::to_string(comp_idx));

  for(size_t i=0; i<used_resources_comp.size(); ++i)
  {
    const auto& [p_idx, r_type_idx, r_idx] = used_resources_comp[i];

    const TimeType perf_p_idx = performance_comp[r_type_idx][p_idx][r_idx]->predict(
      comp_idx, p_idx, ResTypeFromIdx(r_type_idx), r_idx,
      system.get_system_data(),
      solution_data);
    // this takes into account the compute_utilization.
    // If utilization > 1, response_time is negative- (get_perf_evaluation return -1)
    // Check lables of response_time (when is negative, NaN, +inf, max ...)
    local_parts_perfs[comp_idx][i] = perf_p_idx !=-1 ? perf_p_idx : NaN;

    if(used_resources_comp.size() > 1) // I have to compute network delay
    {
      local_parts_delays[comp_idx].clear();
      local_parts_delays[comp_idx].reserve(used_resources_comp.size()-1);

      for(auto it = used_resources_comp.cbegin(); it != std::prev(used_resources_comp.cend()); ++it)
      {
        const auto data_size = partitions_comp[std::get<0>(*it)].get_data_size();
        const auto res1_type_idx = std::get<1>(*it);
        const auto res1_idx = std::get<2>(*it);
        const auto res2_type_idx = std::get<1>(*std::next(it));
        const auto res2_idx = std::get<2>(*std::next(it));

        // I have to compute the network delay only if the two resources running
        // the two partitions are different!
        if(res1_type_idx != res2_type_idx || res1_idx != res2_idx)
        {
          // logger messages
          local_parts_delays[comp_idx].push_back(compute_network_delay(
              ResTypeFromIdx(res1_type_idx), res1_idx,
              ResTypeFromIdx(res2_type_idx), res2_idx, data_size,
              system));
        }
        else
        {
          local_parts_delays[comp_idx].push_back(0.0);
        }
      }
    }
    // logger messages
  }
  const TimeType parts_total_time = std::accumulate(
    local_parts_perfs[comp_idx].cbegin(), local_parts_perfs[comp_idx].cend(), 0.0);
  const TimeType parts_total_delay_time = std::accumulate(
    local_parts_delays[comp_idx].cbegin(), local_parts_delays[comp_idx].cend(), 0.0);

  comp_perfs[comp_idx] = parts_total_time + parts_total_delay_time;
}

TimeType
SystemPE::compute_network_delay(
  ResourceType res1_type, size_t res1_idx, ResourceType res2_type, size_t res2_idx,
  DataType data_size,
  const System& system)
{
  TimeType network_delay = std::numeric_limits<TimeType>::infinity();
  const auto& system_data = system.get_system_data();
  const auto& all_resources = system_data.get_all_resources();
  const auto& network_domains = system_data.get_network_domains();
  const auto& cl_to_network_domains = system_data.get_cl_to_network_domains();
  const auto& cl1_name = all_resources.get_cl_name(res1_type, res1_idx);
  const auto& cl2_name = all_resources.get_cl_name(res2_type, res2_idx);
  const auto& network_domains_1 = cl_to_network_domains.at(cl1_name);
  const auto& network_domains_2 = cl_to_network_domains.at(cl2_name);
  std::set<size_t> network_domains_intersect;
  std::set_intersection(
    network_domains_1.cbegin(), network_domains_1.cend(),
    network_domains_2.cbegin(), network_domains_2.cend(),
    std::inserter(network_domains_intersect, network_domains_intersect.begin())
  );

  if(network_domains_intersect.empty())
  {
    const std::string err =   "ERROR: no network domain available between\n" +
      std::to_string(res1_idx) + " of type " + std::to_string(ResIdxFromType(res1_type)) + "\n" +
      std::to_string(res2_idx) + " of type " + std::to_string(ResIdxFromType(res2_type));
    // logger message
    throw std::runtime_error(err);
  }
  else if(network_domains_intersect.size() == 1)
  {
    const auto access_delay = network_domains[*network_domains_intersect.begin()].get_access_delay();
    const auto bandwidth = network_domains[*network_domains_intersect.begin()].get_bandwidth();
    network_delay = get_network_delay(access_delay, bandwidth, data_size);
  }
  else // more than one network_domain
  {
    TimeType new_network_delay = std::numeric_limits<TimeType>::quiet_NaN();

    for(size_t idx : network_domains_intersect)
    {
      const auto access_delay = network_domains[idx].get_access_delay();
      const auto bandwidth = network_domains[idx].get_bandwidth();
      new_network_delay = get_network_delay(access_delay, bandwidth, data_size);
      network_delay = new_network_delay < network_delay ?
        new_network_delay : network_delay;
    }
  }
  return network_delay;
}

} //namespace Space4AI
