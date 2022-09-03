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

  std::string output_dir = "OutputFiles/";
  fs::create_directory(output_dir);

  Timings::Chrono my_chrono;

  const size_t n_iterations = basic_config.at("Algorithm").at("n_iterations").get<size_t>();
  const size_t max_num_sols = basic_config.at("Algorithm").at("max_num_sols").get<size_t>();
  const bool reproducibility = basic_config.at("Algorithm").at("reproducibility").get<bool>();

  Logger::SetPriority(static_cast<LogPriority>(basic_config.at("Logger").at("priority").get<int>()));
  Logger::EnableTerminalOutput(basic_config.at("Logger").at("terminal_stream").get<bool>());
  const bool enable_file_output = basic_config.at("Logger").at("file_stream").get<bool>();

  // initialize pybind11
  pybind11::scoped_interpreter guard{};

  for(size_t i=0; i < basic_config.at("ConfigFiles").size(); ++i)
  {
    sp::System system;

    my_chrono.start();
    const std::string system_config_file = basic_config.at("ConfigFiles")[i].get<std::string>();

    if(enable_file_output)
    {
      std::string filename_ = system_config_file;
      eraseAllSubStr(filename_, "config");
      eraseAllSubStr(filename_, ".json");
      std::replace(filename_.begin(), filename_.end(), '/', '_');
      Logger::EnableFileOutput(true, filename_);
    }

    system.read_configuration_file(system_config_file);
    const double system_read_time = my_chrono.wallTimeNow() * 1e-6;

    my_chrono.start();
    const auto elite_result = sp::RandomGreedyDT::random_greedy(
      system, n_iterations, max_num_sols, reproducibility
    );
    const double algorithm_run_time = my_chrono.wallTimeNow() * 1e-6;

    std::string suffix_sol = system_config_file;
    eraseAllSubStr(suffix_sol, "config");
    std::replace(suffix_sol.begin(), suffix_sol.end(), '/', '_');

    const auto& sols = elite_result.get_solutions();
    for(size_t rk = 0; rk < sols.size(); ++rk)
    {
      const std::string sol_output_file = output_dir + "Sol_rk" + std::to_string(rk) + suffix_sol;
      elite_result.print_solution(system, sol_output_file, rk);

      nl::json InfoSol;

      InfoSol["System"]["name"] = system_config_file;
      InfoSol["System"]["readTime"] = system_read_time;

      InfoSol["Algorithm"]["name"] = "Random Greedy";
      InfoSol["Algorithm"]["time"] = algorithm_run_time;
      InfoSol["Algorithm"]["n_iterations"] = n_iterations;
      InfoSol["Algorithm"]["num_threads"] = elite_result.get_num_threads();

      InfoSol["Rank"] = rk;
      InfoSol["SolCost"] = sols[rk].get_cost();

      const std::string infoSol_output_file = output_dir + "InfoSol_rk" + std::to_string(rk) + suffix_sol;

      std::ofstream o(infoSol_output_file);
      o << std::setw(4) << InfoSol << std::endl;
    }

    if(sols.size() > 0)
    {
      std::cout << "Found feasible solution to: " << system_config_file << " of best cost: " << sols[0].get_cost() << std::endl;
    }
    else
    {
      std::cout << "No feasible solution found to: "<< system_config_file << std::endl;
    }

    std::cout << std::endl;
    std::cout << "System Reading time (in seconds): " << system_read_time << std::endl;
    std::cout << "Random Greedy running time (in seconds): " << algorithm_run_time << std::endl;
    std::cout << std::endl;

  }

  return 0;
}
