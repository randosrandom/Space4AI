/*
Copyright 2021 AI-SPRINT

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

/**
* \file System.hpp
*
* \brief Defines the class to store the general system object.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <limits>

#include "src/Performance/PerformanceModels.hpp"
#include "src/Performance/PerformanceFactory.hpp"
#include "src/System/SystemData.hpp"

namespace Space4AI
{

namespace nl = nlohmann;

class Solution;

/** Class to store the general System object.
*
*   Class to store all the data structures included in SystemData plus
*   the performance models of each Resource in each Component. These are
*   extracted from the "Performance" section of the .json configuration file.
*
*/
class System
{
  public:

    /** Method to read the .json file that describes
    *   the System and convert it to a nl::json object.
    *
    *   \param system_file String containing the directory of the .json file
    *                      that describes the System
    *   \paramm config_file_type Identufy the configuration file version
    */
    void read_configuration_file(const std::string& system_file);

    /** performance getter */
    const PerformanceType&
    get_performance() const {return performance;};

    /** system_data getter */
    const SystemData&
    get_system_data() const {return system_data; };

    /** dynamicPerfModels getter */
    bool
    get_dynamicPerfModels() const {return dynamicPerfModels; }

  private:

    /** Method to populate the performance evaluators.
    *
    *   \param performance_json json object extracted from configuration_file containing
    *                           the description of the performance models
    */
    void initialize_performance(const nl::json& performance_json);

    /** Method to populate the demand matrix.
    *
    *   \param demand_matrix_json  nl::json object extracted from configuration_file containing
    *                              the description of the static performance evaluators.
    */
    void initialize_demand_matrix(const nl::json& demand_matrix_json);

  private:
    /** Object containing the data structures that define the System configuration
    *   except for the performance models.
    */
    SystemData system_data;

    /** Object used to store uniqe ptrs to the performance models used by each Resource
    *   in each Component.
    *
    *   Indexed by: [comp_idx][res_type_idx][part_idx][res_idx]
    */
    PerformanceType performance;

    /** Flag indicating type of Performance Models read (only needed for ResourceType::Faas since
    *   ResourceType::Edge and ResourceType::VM default to queue theory).
    *
    *   False: Performace Models are all static, so a demand_matrix has been built.
    *   True:  There is at least one dynamic Performance Model, pybind11 must be called during
    *          the construction of the Solution to evaluate response times.
    *
    *   This flag is important, since Parallelization is not possible for dynamic models (see GIL issue).
    */
    bool dynamicPerfModels = false;

};

} // namespace Space4AI

#endif //SYSTEM_HPP
