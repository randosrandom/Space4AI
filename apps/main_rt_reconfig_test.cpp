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

int
main(int argc, char** argv)
{
  if(argc != 3)
  {
    throw std::invalid_argument("Wrong number of arguments provided. Plese provide just the path of the basic json configuration file");
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
    throw std::runtime_error("Can't open: " + lambda_config_filepath.string() + " file.");
  }

  std::string output_dir = "LambdaProfileOut/";
  fs::create_directories(output_dir);

  const size_t rg_n_iterations = basic_config.at("Algorithm").at("RG_n_iterations").get<size_t>();
  const size_t ls_n_iterations = basic_config.at("Algorithm").at("LS_n_iterations").get<size_t>();
  const size_t rg_max_num_sols = basic_config.at("Algorithm").at("RG_max_num_sols").get<size_t>();
  const size_t ls_max_num_sols = basic_config.at("Algorithm").at("LS_max_num_sols").get<size_t>();
  const bool reproducibility = basic_config.at("Algorithm").at("reproducibility").get<bool>();

  const auto lambda_vec = lambda_config.at("lambda").get<std::vector<sp::LoadType>>();

  Logger::SetPriority(static_cast<LogPriority>(basic_config.at("Logger").at("priority").get<int>()));
  Logger::EnableTerminalOutput(basic_config.at("Logger").at("terminal_stream").get<bool>());
  const bool enable_file_output = basic_config.at("Logger").at("file_stream").get<bool>();

  // initialize pybind11
  pybind11::scoped_interpreter guard{};

  if(basic_config.at("ConfigFiles").size()>1)
  {
    throw std::runtime_error("Please provide single system file for this test");
  }

  // first lambda
  sp::Solution best_dt_sol(nullptr);
  const std::string system_config_file = basic_config.at("ConfigFiles")[0].get<std::string>();
  sp::System system;
  system.read_configuration_file(system_config_file, lambda_vec.front());
  {
  sp::RandomGreedy rg;

  const auto rg_elite_result = rg.random_greedy(
    system, rg_n_iterations, rg_max_num_sols, reproducibility);

  sp::LocalSearchManager ls_man(rg_elite_result, system, reproducibility, ls_n_iterations, ls_max_num_sols);
  ls_man.run();
  const auto& ls_sols = ls_man.get_ls_elite_result().get_solutions();
  best_dt_sol = ls_sols.at(0);
  // best_dt_sol = rg_elite_result.get_solutions().at(0);
  }

  std::string suffix_sol = system_config_file;
  eraseAllSubStr(suffix_sol, "config");
  eraseAllSubStr(suffix_sol, ".json");
  std::replace(suffix_sol.begin(), suffix_sol.end(), '/', '_');
  suffix_sol += "_lambda_" + std::to_string(lambda_vec.front()) + ".json";
  std::string sol_output_file = output_dir + "Sol" + suffix_sol;
  best_dt_sol.print_solution(system, sol_output_file);

  sp::Solution last_rt_sol(best_dt_sol);

  for(size_t i=1; i<lambda_vec.size(); ++i)
  {
    sp::System system;
    system.read_configuration_file(system_config_file, lambda_vec[i]);

    sp::RandomGreedy rg_run(last_rt_sol.get_selected_resources());
    const auto rg_run_elite_result = rg_run.random_greedy(
      system, rg_n_iterations, rg_max_num_sols, reproducibility);
    sp::LocalSearchManager ls_run_man(rg_run_elite_result, system, reproducibility, ls_n_iterations, ls_max_num_sols, last_rt_sol.get_selected_resources());
    ls_run_man.run();

    suffix_sol = system_config_file;
    eraseAllSubStr(suffix_sol, "config");
    eraseAllSubStr(suffix_sol, ".json");
    std::replace(suffix_sol.begin(), suffix_sol.end(), '/', '_');
    suffix_sol += "_lambda_" + std::to_string(lambda_vec[i]) + ".json";
    sol_output_file = output_dir + "Sol" + suffix_sol;

    const auto& new_rt_sols = ls_run_man.get_ls_elite_result().get_solutions();
    //const auto& new_rt_sols = rg_run_elite_result.get_solutions();
    if(new_rt_sols.size() > 0)
    {
      std::cout << "Find new sol for lambda= " << lambda_vec[i] << " of cost: " << new_rt_sols.at(0).get_cost() << std::endl;
      new_rt_sols.at(0).print_solution(system, sol_output_file);
    }
    else
    {
      throw std::runtime_error("No feasible solution found for lambda= " + std::to_string(lambda_vec[i]));
    }
    last_rt_sol = new_rt_sols.at(0);

    if(best_dt_sol.get_selected_resources().get_selected_edge() != last_rt_sol.get_selected_resources().get_selected_edge())
    {
      std::cout << "UCCISO DEGLI EDGES" << std::endl;      
    }

  }
  return 0;
}
