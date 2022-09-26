#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "src/s4ai.hpp"

namespace sp = Space4AI;
namespace py = pybind11;
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

  Logger::SetPriority(static_cast<LogPriority>(basic_config.at("Logger").at("priority").get<int>()));
  Logger::EnableTerminalOutput(basic_config.at("Logger").at("terminal_stream").get<bool>());

  // initialize pybind11
  pybind11::scoped_interpreter guard{};

  for(size_t i=0; i < basic_config.at("ConfigFiles").size(); ++i)
  {
    std::cout << std::endl;

    sp::System system;
    const std::string system_config_file = basic_config.at("ConfigFiles")[i].get<std::string>();

    std::string filename_ = system_config_file;
    eraseAllSubStr(filename_, "config_tests");
    eraseAllSubStr(filename_, ".json");
    std::replace(filename_.begin(), filename_.end(), '/', '_');
    Logger::EnableFileOutput(true, filename_);

    system.read_configuration_file(system_config_file);

    sp::RandomGreedyDT rg;

    const auto elite_result = rg.random_greedy(
      system,
      basic_config.at("Algorithm").at("n_iterations").get<size_t>(),
      basic_config.at("Algorithm").at("max_num_sols").get<size_t>(),
      true
    );

    std::cout << std::endl;

  }

	return EXIT_SUCCESS;
}
