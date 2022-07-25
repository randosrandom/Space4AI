/**
* \file System.hpp
*
* \brief Define the class to store the general system object
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <limits>

#include "Performance/PerformanceModels.hpp"
#include "Performance/PerformanceFactory.hpp"
#include "SystemData.hpp"

namespace Space4AI
{
  namespace nl = nlohmann;

  /** System
  *
  *   Class to store all the data structures included in SystemData plus
  *   the performance models of each device in each component. These are
  *   extracted from the "Performance" section of the .json configuration file.
  *
  */
  class System
  {

  public:

    /** Method to read the .json file that describes
    *   the system and convert it to a nl::json object.
    *
    *   \param system_file String containing the directory of the .json file
    *                      that describes the system
    */
    void
    read_configuration_file(std::string system_file);

    /** performance getter */
    const PerformanceType&
    get_performance() const {return performance;};

    /** system_data getter */
    const SystemData&
    get_system_data() const {return system_data; };

  private:

    /** Method to populate the performance evaluators
    *
    *   \param performance_json json object extracted from configuration_file containing
    *                           the description of the performance models
    */
    void
    initialize_performance(const nl::json& performance_json);

    /** Method to populate the performance evaluators
    *
    *   \param emand_matrix_json  json object extracted from configuration_file containing
    *                             the description of the static performance evaluators.
    *                             (needed for the old style configuration files.)
    */
    void
    initialize_demand_matrix(const nl::json& demand_matrix_json);

  private:
    /** object containing the data structures that define the system configuration
    *   except for the performance models
    */
    SystemData system_data;

    /** Object used to store uniqe ptrs to the performance models used by each resource
    *   in each component.
    *
    *   Indexed by: [comp_idx][res_type_idx][part_idx][res_idx]
    */
    PerformanceType performance;

  };

} // namespace Space4AI

#endif //SYSTEM_HPP
