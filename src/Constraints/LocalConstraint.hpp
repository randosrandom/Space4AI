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
* \file LocalConstraint.hpp
*
* \brief Defines the class to store the Local constraints on the single components.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef LOCALCONSTRAINT_HPP
#define LOCALCONSTRAINT_HPP

#include "src/TypeTraits.hpp"

namespace Space4AI
{
/** Class to represent a performance constraint for
*   a single Component.
*/
class LocalConstraint
{
  public:

    /** LocalConstraint class constructor.
    *
    *   \param comp_idx_ Index of the Component in the SystemData.components vector
    *   \param res_time_ Response time constraint
    */
    LocalConstraint(
      std::size_t comp_idx_,
      TimeType res_time_
    ): comp_idx(comp_idx_), max_res_time(res_time_)
    {}

    /** Bring back default move constructor */
    LocalConstraint(LocalConstraint&&) = default;

    /** Bring back move assignment operator */
    LocalConstraint& operator=(LocalConstraint&&) = default;

    /** comp_idx getter */
    size_t
    get_comp_idx() const { return comp_idx; }

    /** max_res_time getter */
    TimeType
    get_max_res_time() const { return max_res_time; }

  private:

    /** Index of the Component in the SystemData.components vector */
    std::size_t comp_idx;

    /** Response time constraint */
    TimeType max_res_time;

};

} // namespace Space4AI

#endif /* LOCALCONSTRAINT_HPP */
