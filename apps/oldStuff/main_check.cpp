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

  sp::DAG dg;
  dg.read_from_file(basic_config.at("DirectedAcyclicGraph"), basic_config.at("Components"));

  const auto& dag_matrix = dg.get_dag_matrix();
  const auto& idx_to_comp_name = dg.get_idx_to_comp_name();

  std::cout << "DAG MATRIX" << std::endl;
  for(size_t i=0; i<dag_matrix.size(); ++i)
  {
    std::cout << std::endl;
    for(size_t j=0; j<dag_matrix.size(); ++j)
      std::cout << std::fixed << std::setprecision(3) << dag_matrix[i][j] << " ";
  }

  std::cout << "\nIndex to Comp name" << std::endl;
  for(const auto& [idx, name] : idx_to_comp_name)
  {
    std::cout << idx << ": " << name << std::endl;
  }
}
