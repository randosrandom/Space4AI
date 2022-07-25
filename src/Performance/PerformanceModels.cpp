#include "PerformanceModels.hpp"

#include <cmath>

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

    const auto& y_hat = solution_data.get_y_hat();

    #warning "double check if the following if is needed..."
    if(y_hat[comp_idx][ResIdxFromType(res_type)][part_idx][res_idx] > 0)
    {
        double utilization = compute_utilization(res_type, res_idx, system_data, solution_data);
        if(utilization > 1) // This is en error! utilization cannot be bigger than -1;
          response_time = -1.;  // manage error in the caller function
        else
          response_time = this->demand / (1-utilization);
    }

    return response_time;
  }

  double
  QTPE::compute_utilization(
    ResourceType res_type, size_t res_idx,
    const SystemData& system_data,
    const SolutionData& solution_data
  )
  {
    double utilization{0.0};

    const size_t type_idx = ResIdxFromType(res_type);

    const auto& components = system_data.get_components();
    const auto& used_resources = solution_data.get_used_resources();
    const auto& y_hat = solution_data.get_y_hat();

    for(std::size_t c=0; c < components.size(); ++c)
    {
      for(const auto& [p_idx, r_type_idx, r_idx]: used_resources[c])
      {
        utilization += (type_idx == r_type_idx) && (r_idx == res_idx) ?
            QTPE::all_demands[c][type_idx][p_idx][r_idx]
            * components[c].get_partition(p_idx).get_part_lambda()
            / y_hat[c][type_idx][p_idx][r_idx] // number of resources
          :
            0.;
      }
    }

    return utilization;
  }

  /* deprecating in PACS ...
  TimeType
  CoreBasedPE::predict(
    size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
    const SystemData& system_data,
    const SolutionData& solution_data
  ) const
  {
    const auto& y_hat = solution_data.get_y_hat();

    if(y_hat[comp_idx][ResIdxFromType(res_type)][part_idx][res_idx] > 0)
    {
      const auto& resources = system_data.get_resources();

      size_t cores = 0;

      if(res_type == ResourceType::Edge)
        cores = resources.get_resources<ResourceType::Edge>(res_idx).get_n_cores();
      else if (res_type == ResourceType::VM)
        cores = resources.get_resources<ResourceType::VM>(res_idx).get_n_cores();
      else
      {
        // Logger message
        throw std::logic_error("Cannot apply CoreBasedPE on Faas Resources");
      }

      return this->predictor.predict(this->regressor_file, cores, std::log10(cores));
    }
    else
    {
      #warning "double-check what to do in this case"
      return 0.;
    }
  }
  */

  TimeType
  FaasPacsltkPE::predict(
    size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
    const SystemData& system_data,
    const SolutionData& solution_data
  ) const
  {
    const auto& y_hat = solution_data.get_y_hat();

    if(y_hat[comp_idx][ResIdxFromType(res_type)][part_idx][res_idx] > 0)
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
    else
    {
      #warning "double-check what to do in this case"
      return 0.;
    }
  }

  TimeType
  FaasPacsltkStaticPE::predict(
    size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
    const SystemData& system_data,
    const SolutionData& solution_data
  ) const
  {
    return this->demand;
  }

  /* Deprecatin in Pacs ...
  TimeType
  FaasMLlibPE::predict(
    size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
    const SystemData& system_data,
    const SolutionData& solution_data
  ) const
  {
    const auto& y_hat = solution_data.get_y_hat();

    if(y_hat[comp_idx][ResIdxFromType(res_type)][part_idx][res_idx] > 0)
    {
      const auto& components = system_data.get_components();
      const auto& resources = system_data.get_resources();

      const auto part_lambda =
        components[comp_idx].get_partitions(part_idx).get_part_lambda();

      const auto idle_time_before_kill =
        resources.get_resources<ResourceType::Faas>(res_idx).get_idle_time_before_kill();

      return this->predictor.predict(this->regressor_file,
        part_lambda, this->demandWarm, this->demandCold,
        idle_time_before_kill
      );
    }
    else
    {
      #warning "double-check what to do in this case"
      return 0.;
    }
  }
  */

} //namespace Space4AI
