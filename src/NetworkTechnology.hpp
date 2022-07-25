/**
* \file NetworkTechnology.hpp
*
* \brief Defines the class that represents the Network domains
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef NETWORKTECHNOLOGY_HPP_
#define NETWORKTECHNOLOGY_HPP_

#include <string>
#include <vector>

namespace Space4AI
{
  /** NetworkDomain
  *
  *   Class used to represent a network domain used for data transfer operations
  *   among different Component or Partition objects
  */
  class NetworkDomain
  {
  public:

    /** NetworkDomain class constructor
    *
    *   \param name_ Name of the network domain
    *   \param cls_names_ Vector of names of the ComputationalLayer objects
    *                     located in the network domain
    *   \param access_delay_ Access delay characterizing the network domain
    *   \param bandwidth_ Bandwidth characterizing the network domain
    */
    NetworkDomain(
      const std::string& name_,
      const std::vector<std::string>& cls_names_,
      TimeType access_delay_,
      double bandwidth_
    ):
    name(name_), cls_names(cls_names_), access_delay(access_delay_),
    bandwidth(bandwidth_)
    {}

    /** name getter */
    std::string get_name() const{
      return name;
    }

    /** cls getter */
    const std::vector<std::string>& get_cls() const{
      return cls_names;
    }

    /** access_delay getter */
    TimeType get_access_delay() const{
      return access_delay;
    }

    /** bandwidth getter */
    double get_bandwidth() const{
      return bandwidth;
    }

  private:

    /** Name of the network domain */
    const std::string name;

    /** Vector of names of the ComputationalLayer objects located in the network domain */
    const std::vector<std::string> cls_names;

    /** Access delay characterizing the network domain */
    const TimeType access_delay;

    /** Bandwidth characterizing the network domain */
    const double bandwidth;
  };

} //namespace Space4AI


#endif // NETWORKTECHNOLOGY_HPP_
