/**
* \file RandomGreedyDT.hpp
*
* \brief Defines the Random Greedy (Design time version) algorithm to solve the problem
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef RANDOM_GREEDY_DT_HPP_
#define RANDOM_GREEDY_DT_HPP_

#include "src/System/System.hpp"
#include "src/Solution/Solution.hpp"
#include "src/Solution/EliteResult.hpp"

namespace Space4AI
{
  /** Class to define the Random Greedy algorithm to solve the problem
  */
  class RandomGreedyDT
  {
  public:

    /** Method to generate a random greedy solutions
    *
    *   \param system A System object
    *   \param max_it Maximum number of iterations for the algorithm
    *   \param num_top_sols maximum length admissible for the vector (see EliteResult.hpp)
    *
    *   \return EliteResult class containing num_top_sols solution ordered by cost
    */
    static
    EliteResult
    random_greedy(
      const System& system,
      std::size_t max_it,
      std::size_t num_top_sols
    );

    /** Reproducibility setter.
    *   \param reproducibility_
    */
    static
    void
    set_reproducibility(bool reproducibility_) {
      reproducibility = reproducibility_;
    }

  private:

    /** Single step of the randomized greedy algorithm.
    *   It randomly generates a candidate solution, then evaluates its feasibility;
    *   if it is feasible, it evaluates its cost and updates it by reducing the cluster size.
    *
    *   \param system The system object under analysis
    *   \return A Solution object
    *
    *
    */
    static
    Solution
    step(const System& system);

    /** Method to create the initial random solution
    *   \param system
    *   \return Solution object
    */
    static
    Solution
    create_random_initial_solution(
      const System& system
    );

    /** Method reduce the number of VM resources in a cluster.
    *   \param Solution The original solution
    *   \param res_type_idx Index indicating the resource type of the resource with index res_idx
    *   \param res_idx Resource index to which to perform the reduce_cluster
    *   \param system The system under analysis
    *
    *   \return Updated Solution with reduced cluster
    */
    static
    Solution
    reduce_cluster_size(
      const Solution& solution,
      const size_t res_type_idx,
      const size_t res_idx,
      const System& system
    );

  private:

    /** reproducibility flag.
    *   If it's true a fixed initial seed it's used, if false a random initial seed is generated
    */
    inline static bool reproducibility = false;

  };

} // namespace Space4AI

#endif /* RANDOM_GREEDY_DT_HPP_ */
