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
  const auto init = sp::Initializer();

  for(size_t i=0; i < basic_config.at("ConfigFiles").size(); ++i)
  {
    std::cout << std::endl;

    sp::System system;
    const std::string system_config_file = basic_config.at("ConfigFiles")[i].get<std::string>();
    system.read_configuration_file(system_config_file);

    const auto elite_result = sp::RandomGreedyDT::random_greedy(
      system,
      basic_config.at("Algorithm").at("n_iterations").get<size_t>(),
      basic_config.at("Algorithm").at("max_num_sols").get<size_t>()
    );

    if(system.get_dynamicPerfModels() && elite_result.get_num_threads() > 1)
    {
        std::cout << "Threads with dynamic: " << elite_result.get_num_threads() << std::endl;
        return EXIT_FAILURE;
    }
    if(!system.get_dynamicPerfModels() && elite_result.get_num_threads() < 2)
    {
        std::cout << "Threads with static: " << elite_result.get_num_threads() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << std::endl;
  }

	return EXIT_SUCCESS;
}
