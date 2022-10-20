#include <filesystem>
#include <fstream>
#include <iostream>


#include "external/chrono/chrono.hpp"

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
  const size_t ls_n_iterations = basic_config.at("Algorithm").at("RG_n_iterations").get<size_t>();
  const size_t max_num_sols = basic_config.at("Algorithm").at("max_num_sols").get<size_t>();
  const bool reproducibility = basic_config.at("Algorithm").at("reproducibility").get<bool>();

  // initialize pybind11
  pybind11::scoped_interpreter guard{};

  for(size_t i=0; i < basic_config.at("ConfigFiles").size(); ++i)
  {
    std::cout << std::endl;

    sp::System system;
    const std::string system_config_file = basic_config.at("ConfigFiles")[i].get<std::string>();
    system.read_configuration_file(system_config_file);

    std::cout << "Starting Random Greedy ..." << std::endl;
    sp::RandomGreedy rg;
    const auto rg_elite_result = rg.random_greedy(
      system, rg_n_iterations, max_num_sols, reproducibility);

    const auto& sols = rg_elite_result.get_solutions();

    if(sols.size() == 0 || sols.at(0).get_cost() == std::numeric_limits<sp::CostType>::infinity())
    {
      std::cout << "ERROR: No feasible solution found in " << rg_n_iterations
        << " iterations by the RG" << std::endl;

      return EXIT_FAILURE;
    }

    std::cout << "Found " << sols.size() << " solutions for file: " << system_config_file << " of best cost: " << sols.at(0).get_cost() << std::endl;
    std::cout << std::endl;

    std::cout << "Starting Local Search ..." << std::endl;

    sp::LocalSearchManager ls_manager(rg_elite_result, system, reproducibility, ls_n_iterations);
    ls_manager.run();
    const auto& ls_vec = ls_manager.get_ls_vec();

    for(size_t i=0; i<ls_vec.size(); ++i)
    {
      if(ls_vec.at(i).get_best_sol().get_cost() > sols.at(i).get_cost())
      {
        throw std::runtime_error("ERROR: Local Search returning solution of cost greater than RG!");
      }

      const auto& sol = ls_vec.at(i).get_best_sol();
      sp::Solution sol_temp = sol;
      sol_temp.set_selected_resources(system);
      bool feasibility = sol_temp.check_feasibility(system);
      if(!feasibility)
      {
        throw std::runtime_error("ERROR: LS returning non-feasible solutions!");
      }

      // check if selected_resources are the same

      const auto& selected_edge_temp = sol_temp.get_selected_resources().get_selected_edge();
      const auto& selected_edge = sol.get_selected_resources().get_selected_edge();

      const auto& selected_vms_temp = sol_temp.get_selected_resources().get_selected_vms();
      const auto& selected_vms = sol.get_selected_resources().get_selected_vms();

      const auto& selected_vms_by_cl_temp = sol_temp.get_selected_resources().get_selected_vms_by_cl();
      const auto& selected_vms_by_cl = sol.get_selected_resources().get_selected_vms_by_cl();

      if(selected_edge_temp != selected_edge)
        throw std::runtime_error("ERROR: selected_edge given by LS does not match real selected edge");
      if(selected_vms_temp != selected_vms)
        throw std::runtime_error("ERROR: selected_vms given by LS does not match real selected vms");
      if(selected_vms_by_cl_temp != selected_vms_by_cl)
        throw std::runtime_error("ERROR: selected_vms_by_cl given by LS does not match real selected vms by cl");

      // check mamory memory_slack_values
      const auto& mem_s_v_temp = sol_temp.get_memory_slack_values();
      const auto& mem_s_v = sol.get_memory_slack_values();

      for(size_t j=0; j<sp::ResIdxFromType(sp::ResourceType::Count); ++j)
      {
        for(size_t i=0; i<std::max(mem_s_v_temp.at(j).size(), mem_s_v.at(j).size()); ++i)
        {
          if(std::abs(mem_s_v.at(j).at(i) - mem_s_v_temp.at(j).at(i)) > 1e-13)
            throw std::runtime_error("ERROR: LS memory memory_slack_values do not mach general feasibility");
        }
      }

      // Check costs
      const auto& res_costs_temp = sol_temp.get_res_costs();
      const auto& res_costs = sol.get_res_costs();
      for(size_t j=0; j<sp::ResIdxFromType(sp::ResourceType::Count); ++j)
      {
        for(size_t i=0; i<std::max(res_costs_temp.at(j).size(), res_costs.at(j).size()); ++i)
        {
          if(std::abs(res_costs_temp.at(j).at(i) - res_costs.at(j).at(i)) > 1e-13)
            throw std::runtime_error("ERROR: LS res_costs do not match with general check feasibility");
        }
      }

      //  check performance
      const auto& comp_perfs_temp = sol_temp.get_time_perfs().get_comp_perfs();
      const auto& comp_perfs = sol.get_time_perfs().get_comp_perfs();

      for(size_t i=0; i<std::max(comp_perfs.size(), comp_perfs_temp.size()); ++i)
      {
        if(std::abs(comp_perfs.at(i) - comp_perfs_temp.at(i)) > 1e-13)
          throw std::runtime_error("ERROR: LS comp perfs do not match with general feasibility!");
      }

      const auto& path_perfs_temp = sol_temp.get_time_perfs().get_path_perfs();
      const auto& path_perfs = sol.get_time_perfs().get_path_perfs();

      for(size_t i=0; i<std::max(path_perfs.size(), path_perfs_temp.size()); ++i)
      {
        if(std::abs(path_perfs.at(i) - path_perfs_temp.at(i)) > 1e-13)
          throw std::runtime_error("ERROR: LS path perfs do not match with general feasibility!");
      }
    }
  }

	return EXIT_SUCCESS;

}
