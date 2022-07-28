#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "external/chrono/chrono.hpp"

#include "src/Algorithms/RandomGreedyDT.hpp"
#include "src/Logger.hpp"
#include "src/Solution/Solution.hpp"
#include "src/System/System.hpp"

namespace sp = Space4AI;
namespace py = pybind11;
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

  Timings::Chrono my_chrono;

  sp::System system;
  sp::EliteResult elite_results(basic_config.at("Algorithm").at("max_num_sols").get<size_t>());

  if(basic_config.at("FolderAutoReadFlag"))
  {
    return 0;
  }
  else
  {
    for(size_t i=0; i < basic_config.at("ConfigFiles").size(); ++i)
    {
      const std::string system_config_file = basic_config.at("ConfigFiles")[i].get<std::string>();
      my_chrono.start();
      system.read_configuration_file(system_config_file);
      double system_read_time = my_chrono.wallTimeNow() * 1e-6;

      my_chrono.start();
      auto elite_results = sp::RandomGreedyDT::random_greedy(
        system,
        basic_config.at("Algorithm").at("n_iterations").get<size_t>(),
        basic_config.at("Algorithm").at("max_num_sols").get<size_t>()
      );
      double algorithm_run_time = my_chrono.wallTimeNow() * 1e-6;

      if(elite_results.get_size() == 0)
      {
        std::cout << "No feasible solution found in "
          << basic_config.at("Algorithm").at("n_iterations").get<size_t>()
          << " iterations." << std::endl;

        return EXIT_FAILURE;
      }

      std::cout << "Found feasible solution for file: " << system_config_file << std::endl;
      std::cout << "System Reading time (in seconds): " << system_read_time << std::endl;
      std::cout << "Random Greedy running time (in seconds): " << algorithm_run_time << std::endl;

      std::cout << std::endl;

    }
  }

	return EXIT_SUCCESS;
}
