/**
* \file RandomGreedyDT.hpp
*
* \brief Defines the Random Greedy (Design time version) algorithm to solve the
*        optimization problem
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef RANDOM_GREEDY_DT_HPP_
#define RANDOM_GREEDY_DT_HPP_

#include "src/System/System.hpp"
#include "src/Solution/Solution.hpp"
#include "src/Solution/EliteResult.hpp"

namespace Space4AI
{
/** Class to define the Random Greedy algorithm to solve the optimization problem
*/
class RandomGreedyDT
{
  public:

    /** Method to generate random greedy solutions
    *
    *   \param system Object containing all the data structures of the System
    *   \param max_it Maximum number of iterations for the algorithm
    *   \param num_top_sols Maximum admissible length for the vector of solutions
    *                       (see EliteResult.hpp)
    * 	\param reproducibility_flag Flag to make the random greedy solutions reproducible
    *   \return EliteResult class containing num_top_sols solutions ordered by cost
    */
    static
    EliteResult random_greedy(
      const System& system,
      std::size_t max_it,
      std::size_t num_top_sols,
      bool reproducibility_flag
    );

  private:

    /** Single step of the randomized greedy algorithm.
    *
    *   It randomly generates a candidate Solution, then evaluates its feasibility;
    *   if it is feasible, it evaluates its cost and updates it by reducing the
    *   ResourceType::Edge and ResourceType::VM devices cluster size.
    *
    *   \param system Object containing all the data structures of the System
    *   \return A Solution object
    */
    static
    Solution step(const System& system);

    /** Method to create the initial random Solution
    *
    *   It randomly generates a candidate Solution as a starting point to find a
    *   feasible one.
    *
    *   \param system Object containing all the data structures of the System
    *   \return Solution object
    */
    static
    Solution create_random_initial_solution(
      const System& system
    );

    /** Method reduce the number of ResourceType::Edge and ResourceType::VM resources
    *   in a cluster.
    *
    *   \param solution The initial Solution
    *   \param res_type_idx Index indicating the ResourceType of the Resource with index res_idx
    *   \param res_idx Index of the Resource whose cluster needs to be reduced
    *   \param system Object describing the System under analysis
    *   \return Updated Solution with reduced cluster
    */
    static
    Solution reduce_cluster_size(
      const Solution& solution,
      const size_t res_type_idx,
      const size_t res_idx,
      const System& system
    );

  private:

    /* Initial seed for the random device */
    inline static const size_t fixed_initial_seed = 121298;

    /* Additional factor for the random device seed */
    inline static const size_t seed_adding_factor = 1000;

    /* Seed for the random device */
    inline static size_t current_seed = 0;

    /* Flag to make the random greedy solutions reproducible */
    inline static bool reproducibility = true;

};

} // namespace Space4AI

#endif /* RANDOM_GREEDY_DT_HPP_ */
