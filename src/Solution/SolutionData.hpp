/**
* \file SolutionData.hpp
*
* \brief Defines the class to store the main data of the solution
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef SOLUTION_DATA_HPP_
#define SOLUTION_DATA_HPP_

#include "src/TypeTraits.hpp"

namespace Space4AI
{

  class Solution; // forward declaration

  class SolutionData
  {
    friend class Solution;

  private:

    /** default constructor */
    SolutionData() = default;

    /** default copy constructor */
    SolutionData(const SolutionData&) = default;

    /** default move constructor */
    SolutionData(SolutionData&&) = default;

    /** default copy operator */
    SolutionData& operator=(const SolutionData&) = default;

    /** default move copy operator */
    SolutionData& operator=(SolutionData&&) = default;

  public:

    /** y_hat getter */
    const YHatType&
    get_y_hat() const
    {
      return y_hat;
    }

    /** used_resources getter */
    const UsedResourcesOrderedType&
    get_used_resources() const
    {
      return used_resources;
    }

    /** n_used_resources getter */
    const UsedResourcesNumberType&
    get_n_used_resources() const
    {
      return n_used_resources;
    }

    ~SolutionData() = default;

  private:

    /** Hyper_Matrix of dimension 4, storing the number of resource of a specific type
    *   used to run a specific partition of a specific component.
    *
    *   See TypeTraits.hpp to understand the type structure
    */
    YHatType y_hat;

    /** Save only the deployed resources.
    *   For each component (indexing the first vector) we save a vector of tuples,
    *   (partition index, ResourceType index, Resource index) used on that component.
    *   Actually it could be sufficient to use just YHat, but since the number of
    *   chosen resources can be much smaller than available resources, it is better
    *   to have a data structure that keeps track only of the used resources.
    *
    *   There is Ordered in name, since the inner vector must be ordered by part_idx at some point.
    *   Using a std::set would not be efficient since order it's not necessary to
    *   keep the order for each insert (See report for details)
    */
    UsedResourcesOrderedType used_resources;

    /** For each [Resource Type, Resource idx] save the number of instances of
    *   such resource deployed in the system.
    */
    UsedResourcesNumberType n_used_resources;

  };

} //namespace Space4AI

#endif /* SOLUTION_DATA_HPP_ */
