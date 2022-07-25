/**
* \file EliteResult.hpp
*
* \brief Defines the class to store to store a fixed-size vector of Solution objects, sorted by minimum cost
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef ELITE_RESULT_HPP_
#define ELITE_RESULT_HPP_

#include "Solution/Solution.hpp"
#include "System/System.hpp"

#include <algorithm>

namespace Space4AI
{
  /** Class to store to store a fixed-size list of Solution objects, sorted by minimum cost */
  class EliteResult
  {
  public:

    /** EliteResult constructor
    *   \param max_num_sols_ maximum length admissible for the vector
    */
    EliteResult(size_t max_num_sols_): max_num_sols(max_num_sols_)
    {
      elite_results.reserve(max_num_sols_);
    }

    /** Print a solution to file.
    *   \param system System to which the solutions refer
    *   \param path Path where to save the solution
    *   \param rank Rank of the solution to be printed (0 the best, 1 the second best, ...)
    */
    void
    print_solution(const System& system, const std::string& path, size_t rank = 0) const;

    /** Solution getter
    *   \param rank Rank of the solution to be returned
    */
    const Solution&
    get_solution(size_t rank = 0) const;

    /** Current number of saved solutions */
    size_t
    get_size() const { return elite_results.size();}

    /** Method to add solution to a solution to the class EliteResult
    *   \param solution Solution object to be added
    */
    template<class T>
    void
    add(T&& solution);


  private:

    const size_t max_num_sols;

    std::vector<Solution> elite_results;

  };

  // TEMPLATE DEFINITIONS

  template<class T>
  void
  EliteResult::add(T&& solution)
  {
    elite_results.push_back(std::forward<T>(solution));

    std::sort(elite_results.begin(), elite_results.end());

    if(elite_results.size() > max_num_sols)
        elite_results.pop_back();
  }

} //namespace Space4AI

#endif /* ELITE_RESULT_HPP_ */
