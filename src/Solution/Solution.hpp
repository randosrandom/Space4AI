/**
* \file Solution.hpp
*
* \brief Defines the class to store the a solution to the system configuration provided
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef SOLUTION_HPP_
#define SOLUTION_HPP_

#include <algorithm>
#include <chrono>
#include <fstream>
#include <limits>

#include "src/System/System.hpp"
#include "src/Performance/SystemPE.hpp"

namespace Space4AI
{
  /** Solution
  *
  *   Class to store and manage the system configuration chosen as a solution
  *   for the AI application
  */
  class Solution //"Configuration"
  {

  public:

    /** Solution default constructor that initialize cost to -1, and sol_performance
    *   to (not feasible, empty, empty)
    */
    Solution():
      feasibility(false),
      total_cost(-1)
    {}

    /** Method that reads the configuration of the solution, if you want to load
    *   a previous solution saved in file.
    *
    *   \param file String containing the directory of the .json file
    *               that describes the configuration of the solution
    *
    *   \param system Object containing all the data structures of the system
    */
    void
    read_solution_from_file(
      const std::string& file,
      const System& system
    );

    /** Method to write the solution in a .json file
    *
    *   \param system Object containing all the data structures of the system
    *   \param name Path to file where you want to save the solution
    */
    void
    print_solution(const System& system, const std::string& name) const;

    /** Method to convert a solution into a json object
    *
    *   \param system Object containing all the data structures of the system
    *   \return json object that describes the solution configuration
    */
    nl::json
    to_json(const System& system) const;

    /** Method to check the feasibility of a solution
    *
    *   \param system Object containing all the data structures of the system
    *   \return true if the solution is feasible, false otherwise
    */
    bool
    check_feasibility(
      const System& system
    );

    /** Method to compute the cost of a solution
    *
    *   \param system Object containing all the data structures of the system
    *   \return cost of the solution
    */
    CostType
    objective_function(
      const System& system
    );

    /** y_hat getter */
    const YHatType&
    get_y_hat() const {return solution_data.y_hat;}

    /** used_resources getter from sol_data*/
    const UsedResourcesOrderedType&
    get_used_resources() const {return solution_data.used_resources;}

    /** n_used_resources getter from solution_data*/
    const UsedResourcesNumberType&
    get_n_used_resources() const {return solution_data.n_used_resources;}

    /** feasibility getter */
    bool
    get_feasibility() const { return feasibility; }

    /** y_hat setter */
    template <class T>
    void
    set_y_hat(T&& y_h) { solution_data.y_hat = std::forward<T>(y_h); }

    /** used_resources setter */
    template <class T>
    void
    set_used_resources(T&& u_r)
    {
      // VERY IMPORTANT
      // Used resources must be ordered by part idx!
      for(auto& vec : u_r)
      {
        std::sort(vec.begin(), vec.end());
      }

      solution_data.used_resources = std::forward<T>(u_r);
    }

    /** n_used_resources setter */
    template <class T>
    void
    set_n_used_resources(T&& n_u_r) { solution_data.n_used_resources = std::forward<T>(n_u_r); }

    /** total_cost setter */
    void
    set_total_cost(CostType cost) {total_cost = cost;}

    /** "<" operator definition:
    *   the solutions will be ordered by cost, meaning that a solution with a smaller
    *   cost is smaller than a solution with a greater cost
    */
    friend
    bool
    operator<(const Solution& s1, const Solution& s2) { return (s1.total_cost < s2.total_cost); }

  private:

    /** Method to check if each component partition is assigned to exactly one resource,
    *   if the solution is coherent with the compatibility matrix and
    *   if the number of used resources does not exceed the number of available
    *   resources
    *
    *   \param system Object containing all the data structures of the system
    *   \return true if the solution satisfies the constraints, false otherwise
    */
    bool
    preliminary_constraints_check_assignments(
      const System& system
    ) const;

    /** Method to check if the solution satisfies the memory constraints
    *
    *   \param system Object containing all the data structures of the system
    *   \return true if the solution satisfies the memory constraints, false otherwise
    */
    bool
    memory_constraints_check(
      const System& system
    ) const;

    /** Method to check that, if a component partition object is executed
    *   on a VM or a Faas, all its successors are not executed on Edge resources
    *   (assignments cannot move back from cloud/faas to edge)
    *
    *   \param system A System object
    *   \return True if the constraint is satisfied
    */
    bool
    move_backward_check(
      const System& system
    ) const;

    /** Method to verify if resources that don't allow colocation are overloaded
    *   (namely more than one partition is running on the resource)
    *
    *   \param system Object containing all the data structures of the system
    *   \return true if the solution satisfies the constraint, false otherwise
    */
    bool
    performance_assignment_check(
      const System& system
    ) const;

    /** Method to check if local constraints are satisfied by the solution
    *
    *   \param system Object containing all the data structures of the system
    *   \return true if the solution satisfies the local constraints, false otherwise
    */
    bool
    local_constraints_check(
      const System& system
    );

    /** Method to check if global constraints are satisfied by the solution
    *
    *   \param system Object containing all the data structures of the system
    *   \return true if the solution satisfies the global constraints, false otherwise
    */
    bool
    global_constraints_check(
      const System& system
    );

    std::pair<bool, TimeType>
    path_global_constraint_check(
      const std::vector<size_t>& comp_idxs,
      const System& system
    ) const;

  private:

    /** feasibility of the solution */
    bool feasibility;

    /** solution_data storing: y_hat, used_resources, and n_used_resources */
    SolutionData solution_data;

    /** Cost of the solution */
    CostType total_cost;

    /** For each component, saves difference between the constraint of max time and the actual performance time */
    std::vector<TimeType> local_slack_values;

    /** For each global path listed in the config file, saves difference between the constraint of max time and the actual performance time */
    std::vector<TimeType> global_slack_values;

    /** component performance times */
    std::vector<TimeType> comp_perfs;

    /** paths performance times */
    std::vector<TimeType> path_perfs;

  };

} //namespace Space4AI

#endif /* SOLUTION_HPP_ */
