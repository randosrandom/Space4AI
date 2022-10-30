#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "src/s4ai.hpp"

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

  // initialize pybind11
  pybind11::scoped_interpreter guard{};

  for(size_t i=0; i < basic_config.at("ConfigFiles").size(); ++i)
  {
    std::cout << std::endl;

    sp::System system;
    const std::string system_config_file = basic_config.at("ConfigFiles")[i].get<std::string>();
    system.read_configuration_file(system_config_file);

    sp::RandomGreedy rg;
    const auto elite_result = rg.random_greedy(
      system,
      basic_config.at("Algorithm").at("RG_n_iterations").get<size_t>(),
      basic_config.at("Algorithm").at("RG_max_num_sols").get<size_t>(),
      true
    );

    const auto ls_n_iterations = basic_config.at("Algorithm").at("LS_n_iterations").get<size_t>();
    const auto ls_max_num_sols = basic_config.at("Algorithm").at("LS_max_num_sols").get<size_t>();
    sp::LocalSearchManager ls_manager(elite_result, system, true, ls_n_iterations, ls_max_num_sols);
    ls_manager.run();

    if(!ls_manager.get_ls_elite_result().get_solutions().at(0).get_feasibility())
      return EXIT_FAILURE;

  }

	return EXIT_SUCCESS;
}
