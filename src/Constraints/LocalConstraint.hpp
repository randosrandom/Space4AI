/**
* \file LocalConstraint.hpp
*
* \brief Defines the class to store the Local constraints on the single components
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef LOCALCONSTRAINT_HPP
#define LOCALCONSTRAINT_HPP

#include "src/TypeTraits.hpp"

namespace Space4AI
{
  /** LocalConstraint
  *
  *   class to represent a local constraint, i.e. a performance constraint for
  *   a single component.
  */
  class LocalConstraint
  {
  public:

    /** LocalConstraint class constructor
    *
    *   \param comp_idx_ Index of the component in the SystemData.components vector
    *   \param res_time_ Response time constraint
    */
    LocalConstraint(
      std::size_t comp_idx_,
      TimeType res_time_
    ): comp_idx(comp_idx_), max_res_time(res_time_)
    {}

    #warning I commented part of code of LocalConstraint. If there are issues check here

    // LocalConstraint() = default;

    // LocalConstraint(const LocalConstraint&) = default;
    // LocalConstraint& operator=(const LocalConstraint&) = default;

    /** Bring back default move constructor and move assignment operator */
    LocalConstraint(LocalConstraint&&) = default;
    LocalConstraint& operator=(LocalConstraint&&) = default;


    /** comp_idx getter */
    size_t
    get_comp_idx() const { return comp_idx; }

    /** max_res_time getter */
    TimeType
    get_max_res_time() const { return max_res_time; }

  private:

    /** Index of the component in the SystemData.components vector */
    std::size_t comp_idx;

    /** Response time constraint */
    TimeType max_res_time;

  };

} // namespace Space4AI



#endif /* LOCALCONSTRAINT_HPP */
