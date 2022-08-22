/**
* \file PerformanceModels.hpp
*
* \brief Defines methods of the classes defined in PerformanceModels.hpp
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#include "src/Performance/PerformanceModels.hpp"

namespace Space4AI
{

TimeType
QTPE::predict(
  size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
  const SystemData& system_data,
  const SolutionData& solution_data
) const
{
  TimeType response_time{0.0};
  const double utilization = compute_utilization(res_type, res_idx, system_data, solution_data);

  if(utilization > 1) // This is en error! utilization cannot be bigger than -1;
  {
    response_time = -1.;  // manage error in the caller function
  }
  else
  {
    response_time =
      all_demands[comp_idx][ResIdxFromType(res_type)][part_idx][res_idx]
      /
      (1 - utilization);
  }

  return response_time;
}

double
QTPE::compute_utilization(
  ResourceType res_type, size_t res_idx,
  const SystemData& system_data,
  const SolutionData& solution_data
) const
{
  double utilization{0.0};
  const size_t type_idx = ResIdxFromType(res_type);
  const auto& components = system_data.get_components();
  const auto& used_resources = solution_data.get_used_resources();
  const auto& y_hat = solution_data.get_y_hat();

  for(std::size_t c = 0; c < components.size(); ++c)
  {
    for(const auto& [p_idx, r_type_idx, r_idx] : used_resources[c])
    {
      utilization += (type_idx == r_type_idx) && (r_idx == res_idx) ?
        all_demands[c][type_idx][p_idx][r_idx]
        * components[c].get_partition(p_idx).get_part_lambda()
        / y_hat[c][type_idx][p_idx][r_idx] // number of resources
        :
        0.;
    }
  }

  return utilization;
}


TimeType
FaasPacsltkPE::predict(
  size_t comp_idx, size_t part_idx, ResourceType, size_t res_idx,
  const SystemData& system_data,
  const SolutionData&
) const
{
  const auto& components = system_data.get_components();
  const auto& all_resources = system_data.get_all_resources();
  const auto part_lambda =
    components[comp_idx].get_partition(part_idx).get_part_lambda();
  const auto idle_time_before_kill =
    all_resources.get_resource<ResourceType::Faas>(res_idx).get_idle_time_before_kill();
  return this->predictor.predict(
      part_lambda, this->demandWarm, this->demandCold, idle_time_before_kill
    );
}


TimeType
FaasPacsltkStaticPE::predict(
  size_t, size_t, ResourceType, size_t,
  const SystemData&,
  const SolutionData&
) const
{
  return this->demand;
}

} //namespace Space4AI
