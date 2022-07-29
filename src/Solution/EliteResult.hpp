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

#include "src/Solution/Solution.hpp"
#include "src/System/System.hpp"

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
    EliteResult(size_t max_num_sols_): max_num_sols(max_num_sols_), num_threads(1)
    {
      solutions.reserve(max_num_sols_);
    }

    /** Solution getter
    *   \param rank Rank of the solution to be returned
    */
    const std::vector<Solution>&
    get_solutions() const {return solutions; }

    /** Current number of saved solutions */
    size_t
    get_size() const { return solutions.size();}

    /** Print a solution to file.
    *   \param system System to which the solutions refer
    *   \param path Path where to save the solution
    *   \param rank Rank of the solution to be printed (0 the best, 1 the second best, ...)
    */
    void
    print_solution(const System& system, const std::string& path, size_t rank = 0) const;

    /** Method to add solution to a solution to the class EliteResult
    *   \param solution Solution object to be added
    */
    template<class T>
    void
    add(T&& solution);

    /** num_threads setter */
    void
    set_num_threads(size_t num_threads_) { num_threads = num_threads_; }

    /** num_threads getter */
    size_t
    get_num_threads() const { return num_threads; }

  private:

    /** macimum number of solution to save */
    const size_t max_num_sols;

    /** Container of the solutions */
    std::vector<Solution> solutions;

    /** Number of threads used in the algorithm to find the solutions */
    size_t num_threads;

  };

  // TEMPLATE DEFINITIONS

  template<class T>
  void
  EliteResult::add(T&& solution)
  {
    solutions.push_back(std::forward<T>(solution));

    std::sort(solutions.begin(), solutions.end());

    if(solutions.size() > max_num_sols)
        solutions.pop_back();
  }

} //namespace Space4AI

#endif /* ELITE_RESULT_HPP_ */