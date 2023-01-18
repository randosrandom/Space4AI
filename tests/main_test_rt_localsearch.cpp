#include <filesystem>
#include <fstream>
#include <iostream>

#include "src/s4ai.hpp"

namespace sp = Space4AI;
namespace fs = std::filesystem;
namespace nl = nlohmann;

int
main(int argc, char** argv)
{
  if(argc != 2)
  {
    throw std::invalid_argument("Wrong number of arguments provided. Plese provide just the path of the basic json configuration file");
  }

  const fs::path basic_config_filepath = argv[1];
  std::ifstream basic_config_file(basic_config_filepath);
  nl::json basic_config;

  if(basic_config_file)
  {
    basic_config = nl::json::parse(basic_config_file);
  }
  else
  {
    throw std::runtime_error("Can't open " + basic_config_filepath.string() + " file. Make sure that the path is correct, and the format is json");
  }

  const size_t rg_n_iterations = basic_config.at("Algorithm").at("RG_n_iterations").get<size_t>();
  const size_t ls_n_iterations = basic_config.at("Algorithm").at("LS_n_iterations").get<size_t>();
  const size_t max_num_sols = basic_config.at("Algorithm").at("max_num_sols").get<size_t>();
  const bool reproducibility = basic_config.at("Algorithm").at("reproducibility").get<bool>();
  const auto lambda = basic_config.at("Lambda").get<sp::LoadType>();

  #warning you should pass it in the json
  const double energy_cost_pct = 0.2;

  Logger::SetPriority(static_cast<LogPriority>(basic_config.at("Logger").at("priority").get<int>()));
  Logger::EnableTerminalOutput(basic_config.at("Logger").at("terminal_stream").get<bool>());

  if(basic_config.at("ConfigFiles").size() != basic_config.at("DTSolutions").size())
  {
    throw std::length_error(
      "Error in configuration input file: *ConfigFiles* dimension does not match"
      " *OutputFilesDT* dimension. If no DT solution is present please use the dt_solver");
  }

  for(size_t i=0; i < basic_config.at("ConfigFiles").size(); ++i)
  {
    sp::System system;
    const std::string system_config_file = basic_config.at("ConfigFiles")[i].get<std::string>();
    const std::string solution_config_file = basic_config.at("DTSolutions")[i].get<std::string>();
    system.read_configuration_file(system_config_file, lambda, energy_cost_pct);

    sp::Solution current_sol(system);
    current_sol.read_solution_from_file(solution_config_file, system);

    sp::RandomGreedy rg(current_sol.get_selected_resources());
    const auto new_rg_elite_result = rg.random_greedy(
      system, rg_n_iterations, max_num_sols, reproducibility);

    const auto& new_rg_sols = new_rg_elite_result.get_solutions();

    // check same selected resources on edge and vms at RG
    const auto& curr_sel_res = current_sol.get_selected_resources();

    for(const auto& sol_: new_rg_sols)
    {
      const auto& selected_resources_ = sol_.get_selected_resources();

      if(curr_sel_res.get_selected_edge() < selected_resources_.get_selected_edge())
        throw std::logic_error("ERROR: RT-RG selected unavailable Edge resources wrt the given solution");

      const auto& selected_vms_by_cl_ = selected_resources_.get_selected_vms_by_cl();
      const auto& curr_sel_vms_by_cl = curr_sel_res.get_selected_vms_by_cl();

      for(size_t i=0; i<std::max(selected_vms_by_cl_.size(), curr_sel_vms_by_cl.size()); ++i)
      {
        if(curr_sel_vms_by_cl.at(i).first)
        {
          if(selected_vms_by_cl_.at(i).first && (selected_vms_by_cl_.at(i).second != curr_sel_vms_by_cl.at(i).second))
            throw std::logic_error("ERROR: RT-RG selected different VM at an already chosen layer in the given solution");
        }
      }
    }

    sp::LocalSearchManager ls_manager(new_rg_elite_result, system, reproducibility, ls_n_iterations, max_num_sols, curr_sel_res);
    ls_manager.run();
    const auto& ls_vec = ls_manager.get_ls_vec();

    for(const auto& ls: ls_vec)
    {
      const auto& sol_ = ls.get_best_sol();
      const auto& selected_resources_ = sol_.get_selected_resources();

      if(curr_sel_res.get_selected_edge() < selected_resources_.get_selected_edge())
        throw std::logic_error("ERROR: RT-LS selected different Edge resources wrt the given solution");

      const auto& selected_vms_by_cl_ = selected_resources_.get_selected_vms_by_cl();
      const auto& curr_sel_vms_by_cl = curr_sel_res.get_selected_vms_by_cl();

      for(size_t i=0; i<std::max(selected_vms_by_cl_.size(), curr_sel_vms_by_cl.size()); ++i)
      {
        if(curr_sel_vms_by_cl.at(i).first)
        {
          if(selected_vms_by_cl_.at(i).first && (selected_vms_by_cl_.at(i).second != curr_sel_vms_by_cl.at(i).second))
            throw std::logic_error("ERROR: RT-LS selected different VM at an already chosen layer in the given solution");
        }
      }
    }
  }
}
