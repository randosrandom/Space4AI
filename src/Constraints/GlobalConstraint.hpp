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
* \file GlobalConstraint.hpp
*
* \brief Defines the class to store the Global constraints on a given path of components.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef GLOBALCONSTRAINT_HPP
#define GLOBALCONSTRAINT_HPP

#include "src/TypeTraits.hpp"

namespace Space4AI
{
/** Class to represent a performance constraint for
*   a specific path (set of Component objects).
*/
class GlobalConstraint
{
  public:

    /** GlobalConstraint class constructor.
    *
    *   \param path_name_ Name of the path
    *   \param comp_idxs_ Vector storing the indexes of the components (in the SystemData.components
    *                     vector) in the path
    *   \param res_time_ Response time constraint
    */
    GlobalConstraint(
      const std::string& path_name_,
      const std::vector<std::size_t>& comp_idxs_,
      TimeType res_time_
    ): path_name(path_name_), comp_idxs(comp_idxs_), max_res_time(res_time_)
    {}

    /** path_name getter */
    std::string
    get_path_name() const {return path_name;}

    /** comp_idxs getter */
    const std::vector<size_t>&
    get_comp_idxs() const { return comp_idxs; }

    /** max_res_time getter */
    TimeType
    get_max_res_time() const {return max_res_time; }

  private:

    /** Name of the path */
    const std::string path_name;

    /** Vector storing the indexes of the components (in the SystemData.components vector)
    *   in the path
    */
    const std::vector<std::size_t> comp_idxs;

    /** Response time constraint */
    const TimeType max_res_time;

};

} //  namespace Space4AI

#endif /* GLOBALCONSTRAINT_HPP */
