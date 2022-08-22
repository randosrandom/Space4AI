/**
* \file SystemPE.cpp
*
* \brief Defines the methods of the classes defined in SystemPE.hpp
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#include "src/Performance/SystemPE.hpp"

namespace Space4AI
{
std::vector<TimeType>
SystemPerformanceEvaluator::compute_performance(
  const System& system,
  const SolutionData& solution_data
)
{
  const auto& components = system.get_system_data().get_components();
  std::vector<TimeType> response_times(components.size(), std::numeric_limits<TimeType>::quiet_NaN());

  for(size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx)
  {
    response_times[comp_idx] = get_perf_evaluation(comp_idx, system, solution_data);
  }

  return response_times;
}

TimeType
SystemPerformanceEvaluator::get_perf_evaluation(
  size_t comp_idx,
  const System& system,
  const SolutionData& solution_data
)
{
  // logger messages in Debug ...
  TimeType perf_evaluation = 0.;
  const auto& performance_comp = system.get_performance()[comp_idx];
  const auto& partitions_comp = system.get_system_data().get_components()[comp_idx].get_partitions();
  const auto& used_resources_comp = solution_data.get_used_resources()[comp_idx];

  // logger messages in Debug ...

  // logger messages in Debug...

  for(const auto& [p_idx, r_type_idx, r_idx] : used_resources_comp)
  {
    perf_evaluation +=
      performance_comp[r_type_idx][p_idx][r_idx]->predict(
        comp_idx, p_idx, ResTypeFromIdx(r_type_idx), r_idx,
        system.get_system_data(),
        solution_data
      );

    if(perf_evaluation < 0)
    {
      // logger message
      return std::numeric_limits<TimeType>::quiet_NaN();
    }

    if(used_resources_comp.size() > 1) // I have to compute network delay
    {
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
          perf_evaluation += get_network_delay(
              ResTypeFromIdx(res1_type_idx), res1_idx, ResTypeFromIdx(res2_type_idx), res2_idx, data_size,
              system
            );
        }
      }
    }

    // logger messages
  }

  return perf_evaluation;
}

TimeType
SystemPerformanceEvaluator::get_network_delay(
  ResourceType res1_type, size_t res1_idx, ResourceType res2_type, size_t res2_idx,
  DataType data_size,
  const System& system
)
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
    network_delay = NetworkPerformanceEvaluator::predict(access_delay, bandwidth, data_size);
  }
  else // more than one network_domain
  {
    TimeType new_network_delay = std::numeric_limits<TimeType>::quiet_NaN();

    for(size_t idx : network_domains_intersect)
    {
      const auto access_delay = network_domains[idx].get_access_delay();
      const auto bandwidth = network_domains[idx].get_bandwidth();
      new_network_delay = NetworkPerformanceEvaluator::predict(access_delay, bandwidth, data_size);
      network_delay = new_network_delay < network_delay ?
        new_network_delay : network_delay;
    }
  }

  return network_delay;
}

} //namespace Space4AI
