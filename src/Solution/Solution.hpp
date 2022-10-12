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
* \file Solution.hpp
*
* \brief Defines the class to store the a solution to the System configuration provided.
*
* \author Randeep Singh
*/

#ifndef SOLUTION_HPP_
#define SOLUTION_HPP_

#include <algorithm>
#include <chrono>
#include <fstream>

#include "src/Solution/SelectedResources.hpp"
#include "src/System/System.hpp"
#include "src/Performance/SystemPE.hpp"

namespace Space4AI
{
/** Class to store and manage the a solution
*   for the AI application represented by a System object.
*/
class Solution
{
  friend class LocalSearch;

  public:

    /** Solution default constructor */
    // Solution(): feasibility(false), total_cost(-1) {}

    /** Solution constructor that initializes feasibility to false
    *   and total_cost to -1, and resize all the data scturctures according
    *   to data contained in system;
    */
    Solution(const System& system);

    /** Method that reads the configuration of the Solution, if you want to load
    *   a previous Solution saved in file.
    *
    *   \param file String containing the directory of the .json file
    *               that describes the configuration of the Solution
    *   \param system Object containing all the data structures of the System
    *
    */
    void
    read_solution_from_file( const std::string& file, const System& system);

    /** Method to write the Solution in a .json file.
    *
    *   \param system Object containing all the data structures of the System
    *   \param name Path to file where you want to save the Solution
    */
    void print_solution(const System& system, const std::string& name) const;

    /** Method to convert a Solution into a nl::json object.
    *
    *   \param system Object containing all the data structures of the System
    *   \return nl::json object that describes the Solution configuration
    */
    nl::json to_json(const System& system) const;

    /** Method to check the feasibility of a Solution.
    *
    *   \param system Object containing all the data structures of the System
    *   \return true if the Solution is feasible, false otherwise
    */
    bool check_feasibility(const System& system);

    /** Method to compute the cost of a Solution.
    *
    *   \param system Object containing all the data structures of the System
    *   \return cost of the Solution
    */
    CostType objective_function(const System& system);

    /** feasibility getter */
    bool
    get_feasibility() const { return feasibility; }

    /** total_cost getter */
    CostType
    get_cost() const {return total_cost; }

    /** y_hat getter */
    const YHatType&
    get_y_hat() const {return solution_data.y_hat;}

    /** used_resources getter from sol_data */
    const UsedResourcesOrderedType&
    get_used_resources() const {return solution_data.used_resources;}

    /** n_used_resources getter from solution_data */
    const UsedResourcesNumberType&
    get_n_used_resources() const {return solution_data.n_used_resources;}

    /** selected resource getter */
    const SelectedResources&
    get_selected_resources() const {return selected_resources;}

    /** memory occupations getter */
    const MemoryOccupationType&
    get_memory_slack_values() const {return memory_slack_values;}

    /** time_perfs getter */
    const SystemPE&
    get_time_perfs() const {return time_perfs;}

    /** y_hat setter */
    template <class T>
    void
    set_y_hat(T&& y_h) {solution_data.y_hat = std::forward<T>(y_h);}

    /** used_resources setter
    *
    *   Note: this method preliminarily sorts the input structure of used resources by
    *   partition index (see TypeTraits.hpp to better understand the type structure).
    */
    template <class T>
    void
    set_used_resources(T&& u_r)
    {
      for(auto& vec : u_r) {std::sort(vec.begin(), vec.end());}
      solution_data.used_resources = std::forward<T>(u_r);
    }

    /** n_used_resources setter */
    template <class T>
    void
    set_n_used_resources(T&& n_u_r) {solution_data.n_used_resources = std::forward<T>(n_u_r);}

    void
    set_selected_resources(const System& system);

    /** "<" operator definition:
    *   the solutions will be ordered by cost, meaning that a Solution with a smaller
    *   cost is smaller than a Solution with a greater cost.
    */
    friend
    bool
    operator<(const Solution& s1, const Solution& s2) { return (s1.total_cost < s2.total_cost); }

  private:

    /** Method to check if each Component Partition is assigned to exactly one Resource,
    *   if the Solution is coherent with the SystemData.compatibility_matrix and
    *   if the number of used resources does not exceed the number of available
    *   resources.
    *
    *   \param system Object containing all the data structures of the System
    *   \return true if the Solution satisfies the constraints, false otherwise
    */
    bool preliminary_constraints_check_assignments(const System& system) const; // It should not be needed to check it after RG. The solution is built satisfying preliminary constraints!

    /** Method to check if each Partition of the passed component is assigned to exactly one Resource,
    *   if the Solution is coherent with the SystemData.compatibility_matrix and
    *   if the number of used resources does not exceed the number of available
    *   resources.
    *
    *   \param system Object containing all the data structures of the System
    *   \param comp_idx Component index to check
    *   \return true if the Solution satisfies the constraints, false otherwise
    */
    bool preliminary_constraints_check_assignments(size_t comp_idx,
      const System& system) const;

    /** Method to check if the Solution satisfies the memory constraints.
    *
    *   \param system Object containing all the data structures of the System
    *   \return true if the Solution satisfies the memory constraints, false otherwise
    */
    bool memory_constraints_check(const System& system);

    /** Method to change memory of resources during local search.
    *
    *   \param local_info local information on the modified resources and partitions
    *   \return true if the Solution satisfies the memory constraints
    */
    bool
    memory_constraints_check(const System& system, const LocalInfo& local_info);

    /** Method to check that, if a Component Partition object is executed
    *   on a ResourceType::VM or a ResourceType::Faas, all its successors
    *   are not executed on ResourceType::Edge resources
    *   (assignments cannot move back from cloud/faas to edge).
    *
    *   \param system Object containing all the data structures of the System
    *   \return True if the constraint is satisfied
    */
    bool
    move_backward_check(const System& system) const;

    bool
    move_backward_check(size_t comp_idx) const;

    /** Method to verify if resources that don't allow colocation are overloaded
    *   (namely more than one Partition is running on the Resource).
    *
    *   \param system Object containing all the data structures of the System
    *   \return true if the Solution satisfies the constraint, false otherwise
    */
    bool performance_assignment_check(
      const System& system,
      const LocalInfo& local_info = LocalInfo());

    /** Method to check if LocalConstraint constraints are satisfied by the Solution.
    *
    *   \param system Object containing all the data structures of the System
    *   \return true if the Solution satisfies the LocalConstraint constraints,
    *           false otherwise
    */
    bool local_constraints_check(
      const System& system,
      const LocalInfo& local_info=LocalInfo());

    /** Method to check if GlobalConstraint constraints are satisfied by the Solution.
    *
    *   \param system Object containing all the data structures of the System
    *   \return true if the Solution satisfies the GlobalConstraint constraints,
    *           false otherwise
    */
    bool global_constraints_check(
      const System& system,
      const LocalInfo& local_info = LocalInfo());

  private:

    /** feasibility of the Solution: true if feasible, false otherwise */
    bool feasibility;

    /** Cost of the Solution */
    CostType total_cost;

    /** SolutionData object storing y_hat, used_resources, n_used_resources */
    SolutionData solution_data;

    /** SelectedResources object.
    *   Store the selected edge and vm resources, if the solution is feasible.
    */
    SelectedResources selected_resources;

    /* For each resource save the slack memory remaining in the solution */
    MemoryOccupationType memory_slack_values;

    /** Local and Global performance */
    SystemPE time_perfs;

};

} //namespace Space4AI

#endif /* SOLUTION_HPP_ */
