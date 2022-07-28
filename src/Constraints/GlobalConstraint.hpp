/**
* \file GlobalConstraint.hpp
*
* \brief Defines the class to store the Global constraints on a given path of components
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef GLOBALCONSTRAINT_HPP
#define GLOBALCONSTRAINT_HPP

#include "src/TypeTraits.hpp"

namespace Space4AI
{
  /** GlobalConstraint
  *
  *   Class to represent a global constraint, i.e. a performance constraint
  *   for a specific path (set of components)
  *
  */
  class GlobalConstraint
  {

  public:

    /** GlobalConstraint class constructor
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
