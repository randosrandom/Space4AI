#include <filesystem>
#include <fstream>
#include <iostream>


#include "external/chrono/chrono.hpp"

#include "src/s4ai.hpp"

namespace sp = Space4AI;
namespace fs = std::filesystem;
namespace nl = nlohmann;


/*
 * Erase all Occurrences of given substring from main string.
 */
void eraseAllSubStr(std::string & mainStr, const std::string & toErase)
{
    size_t pos = std::string::npos;
    // Search for the substring in string in a loop untill nothing is found
    while ((pos  = mainStr.find(toErase) )!= std::string::npos)
    {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
}

std::tuple<double, double, double>
get_cost_frac(const sp::Solution& sol)
{
  const auto total_cost = sol.get_cost();
  const auto& res_costs = sol.get_res_costs();
  const auto edge_total_cost = std::accumulate(res_costs[0].begin(), res_costs[0].end(), 0.0);
  const auto vm_total_cost = std::accumulate(res_costs[1].begin(), res_costs[1].end(), 0.0);
  const auto faas_total_cost = std::accumulate(res_costs[2].begin(), res_costs[2].end(), 0.0);

  return std::make_tuple(edge_total_cost/total_cost, vm_total_cost/total_cost, faas_total_cost/total_cost);
}

std::tuple<size_t, size_t,size_t>
get_num_res(const sp::Solution& sol)
{
  const auto& sel_edge = sol.get_selected_resources().get_selected_edge();
  const auto& sel_vms = sol.get_selected_resources().get_selected_vms();

  const size_t n_used_edge = std::accumulate(sel_edge.begin(), sel_edge.end(), 0);
  const size_t n_used_vms = std::accumulate(sel_vms.begin(), sel_vms.end(), 0);

  size_t n_used_faas(0);
  const auto& used_resources = sol.get_used_resources();
  for(const auto& used_res_comp : used_resources)
  {
    for(const auto& [ignore1, res_type_idx, ignore2]: used_res_comp)
    {
      if(res_type_idx==2)
      {
        n_used_faas++;
      }
    }
  }
  return std::make_tuple(n_used_edge, n_used_vms, n_used_faas);
}

std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<std::string>>
get_res_names(
  const sp::Solution& sol,
  const sp::System& sys
)
{
  std::vector<std::string> edge_names;
  std::vector<std::string> vm_names;
  std::vector<std::string> faas_names;

  const auto& all_resources = sys.get_system_data().get_all_resources();

  const auto& sel_edge = sol.get_selected_resources().get_selected_edge();
  const auto& sel_vms = sol.get_selected_resources().get_selected_vms();

  for(size_t res_idx=0; res_idx<sel_edge.size(); ++res_idx)
  {
    if(sel_edge[res_idx]>0)
    {
      edge_names.push_back(all_resources.get_name(sp::ResTypeFromIdx(0), res_idx));
    }
  }

  for(size_t res_idx=0; res_idx<sel_vms.size(); ++res_idx)
  {
    if(sel_vms[res_idx]>0)
    {
      vm_names.push_back(all_resources.get_name(sp::ResTypeFromIdx(1), res_idx));
    }
  }

  const auto& used_resources = sol.get_used_resources();
  for(const auto& used_res_comp : used_resources)
  {
    for(const auto& [ignore1, res_type_idx, res_idx]: used_res_comp)
    {
      if(res_type_idx==2)
      {
        faas_names.push_back(all_resources.get_name(sp::ResTypeFromIdx(2), res_idx));
      }
    }
  }

  return std::make_tuple(edge_names, vm_names, faas_names);
}

int
main(int argc, char** argv)
{
  if(argc != 3)
  {
    throw std::invalid_argument("Wrong number of arguments provided. Please provide the path of the basic json configuration file and the lambda profile json");
  }

  const fs::path basic_config_filepath = argv[1];
  const fs::path lambda_config_filepath = argv[2];
  std::ifstream basic_config_file(basic_config_filepath);
  std::ifstream lambda_config_file(lambda_config_filepath);
  nl::json basic_config;
  nl::json lambda_config;

  if(basic_config_file)
  {
    basic_config = nl::json::parse(basic_config_file);
  }
  else
  {
    throw std::runtime_error("Can't open " + basic_config_filepath.string() + " file. Make sure that the path is correct, and the format is json");
  }
  if(lambda_config_file)
  {
    lambda_config = nl::json::parse(lambda_config_file);
  }
  else
  {
    throw std::runtime_error("Can't open " + lambda_config_filepath.string() + " file. Make sure that the path is correct, and the format is json");
  }

  std::string output_dir = "LambdaProfileOut/GeneralWorkflows/";
  fs::create_directories(output_dir);

  const size_t rg_n_iterations = basic_config.at("Algorithm").at("RG_n_iterations").get<size_t>();
  const size_t ls_n_iterations = basic_config.at("Algorithm").at("LS_n_iterations").get<size_t>();
  const size_t rg_max_num_sols = basic_config.at("Algorithm").at("RG_max_num_sols").get<size_t>();
  const size_t ls_max_num_sols = basic_config.at("Algorithm").at("LS_max_num_sols").get<size_t>();
  const bool reproducibility = basic_config.at("Algorithm").at("reproducibility").get<bool>();
  const double energy_cost_pct = basic_config.at("EnergyCostPct").get<double>();

  auto lambda_vec = lambda_config.at("LambdaVec").get<std::vector<sp::LoadType>>();

  Logger::SetPriority(static_cast<LogPriority>(basic_config.at("Logger").at("priority").get<int>()));
  Logger::EnableTerminalOutput(basic_config.at("Logger").at("terminal_stream").get<bool>());
  const bool enable_file_output = basic_config.at("Logger").at("file_stream").get<bool>();

  if(basic_config.at("ConfigFiles").size() != basic_config.at("DTSolutions").size())
  {
    throw std::length_error(
      "Error in configuration input file: Number of Scenarios in *ConfigFiles* dimension does not match"
      " number of scenarios in *OutputFilesDT*"
    );
  }
  for(const auto& [scenario_name, instances]: basic_config.at("ConfigFiles").items())
  {
    if(basic_config.at("DTSolutions").at(scenario_name).size() != instances.size())
    {
      throw std::length_error(
        "Number of instances in Scenario" + scenario_name + " do not coincide in *Config Files* and *DTSolutions*");
    }
  }

  // initialize pybind11
  pybind11::scoped_interpreter guard{};

  Timings::Chrono my_chrono;
  for(const auto& [scenario_name, instances]: basic_config.at("ConfigFiles").items())
  {
    const size_t num_instances = instances.size();
    const auto& sol_instances = basic_config.at("DTSolutions").at(scenario_name);

    std::vector<sp::CostType> costs_by_lambda(lambda_vec.size());

    std::vector<sp::CostType> edge_cost_by_lambda(lambda_vec.size(), 0.0);
    std::vector<sp::CostType> vm_cost_by_lambda(lambda_vec.size(), 0.0);
    std::vector<sp::CostType> faas_cost_by_lambda(lambda_vec.size(), 0.0);

    std::vector<double> num_edge_by_lambda(lambda_vec.size(), 0.0);
    std::vector<double> num_vm_by_lambda(lambda_vec.size(), 0.0);
    std::vector<double> num_faas_by_lambda(lambda_vec.size(), 0.0);

    std::vector<double> ts(lambda_vec.size(), 0.0);

    for(size_t ins_idx=0; ins_idx<num_instances; ++ins_idx)
    {
      const std::string system_config_file = instances[ins_idx].get<std::string>();
      const std::string solution_config_file = sol_instances[ins_idx].get<std::string>();

      sp::System init_system;
      init_system.read_configuration_file(system_config_file, lambda_vec[0]);

      sp::Solution initial_deployment(init_system);
      initial_deployment.read_solution_from_file(solution_config_file, init_system);
      if(initial_deployment.check_feasibility(init_system))
      {
        costs_by_lambda[0] += initial_deployment.objective_function(init_system);
      }
      else
      {
        throw std::runtime_error("Initial solution not feasible");
      }

      const auto [ec, vc, fc] = get_cost_frac(initial_deployment);
      edge_cost_by_lambda[0] += ec;
      vm_cost_by_lambda[0] += vc;
      faas_cost_by_lambda[0] += fc;

      const auto [en, vn, fn] = get_num_res(initial_deployment);
      num_edge_by_lambda[0] += en;
      num_vm_by_lambda[0] += vn;
      num_faas_by_lambda[0] += fn;

      sp::Solution curr_rt_sol(initial_deployment);

      const auto& fixed_edge = initial_deployment.get_selected_resources().get_selected_edge();

      for(size_t j=1; j<lambda_vec.size(); ++j)
      {
        sp::System system;
        system.read_configuration_file(system_config_file, lambda_vec[j], energy_cost_pct);

        const auto& selected_vms = curr_rt_sol.get_selected_resources().get_selected_vms();
        const auto& selected_vms_by_cl = curr_rt_sol.get_selected_resources().get_selected_vms_by_cl();

        sp::SelectedResources sel_res(fixed_edge, selected_vms, selected_vms_by_cl);

        my_chrono.start();
        sp::RandomGreedy rg(sel_res);
        const auto rg_elite_result = rg.random_greedy(
          system, rg_n_iterations, rg_max_num_sols, reproducibility);
        sp::LocalSearchManager ls_man(rg_elite_result, system, reproducibility, ls_n_iterations, ls_max_num_sols, sel_res);
        ls_man.run();
        const double algorithm_run_time = my_chrono.wallTimeNow() * 1e-6;

        ts[j] += algorithm_run_time;

        const auto& ls_sols = ls_man.get_ls_elite_result().get_solutions();
        if(ls_sols.size()>0)
        {
          curr_rt_sol = ls_sols[0];
        }
        else
        {
          throw std::runtime_error("Can't find feasible solution...");
        }

        costs_by_lambda[j] += curr_rt_sol.get_cost();

        const auto [ec, vc, fc] = get_cost_frac(curr_rt_sol);
        edge_cost_by_lambda[j] += ec;
        vm_cost_by_lambda[j] += vc;
        faas_cost_by_lambda[j] += fc;

        const auto [en, vn, fn] = get_num_res(curr_rt_sol);
        num_edge_by_lambda[j] += en;
        num_vm_by_lambda[j] += vn;
        num_faas_by_lambda[j] += fn;
      }
    }

    // mean of the things for the instances
    auto lam_mean_double = [&num_instances](double c){return c/num_instances;};
    std::transform(
      costs_by_lambda.begin(), costs_by_lambda.end(), costs_by_lambda.begin(), lam_mean_double);
    std::transform(
      edge_cost_by_lambda.begin(), edge_cost_by_lambda.end(), edge_cost_by_lambda.begin(), lam_mean_double);
    std::transform(
      vm_cost_by_lambda.begin(), vm_cost_by_lambda.end(), vm_cost_by_lambda.begin(), lam_mean_double);
    std::transform(
      faas_cost_by_lambda.begin(), faas_cost_by_lambda.end(), faas_cost_by_lambda.begin(), lam_mean_double);
    std::transform(
      ts.begin(), ts.end(), ts.begin(), lam_mean_double);
    std::transform(
      num_edge_by_lambda.begin(), num_edge_by_lambda.end(), num_edge_by_lambda.begin(), lam_mean_double);
    std::transform(
      num_vm_by_lambda.begin(), num_vm_by_lambda.end(), num_vm_by_lambda.begin(), lam_mean_double);
    std::transform(
      num_faas_by_lambda.begin(), num_faas_by_lambda.end(), num_faas_by_lambda.begin(), lam_mean_double);

    nl::json output_json;

    output_json["Costs"] = costs_by_lambda;
    output_json["CostByRes"]["Edge"] = edge_cost_by_lambda;
    output_json["CostByRes"]["VM"] = vm_cost_by_lambda;
    output_json["CostByRes"]["Faas"] = faas_cost_by_lambda;
    output_json["Timings"] = ts;

    output_json["NumberResources"]["Edge"] = num_edge_by_lambda;
    output_json["NumberResources"]["VM"] = num_vm_by_lambda;
    output_json["NumberResources"]["Faas"] = num_faas_by_lambda;

    std::string output_name = output_dir + "Sol_" + scenario_name + ".json";
    std::ofstream o(output_name);
    o << std::setw(4) << output_json << std::endl;
  }
  return 0;
}
