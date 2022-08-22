/**
* \file Initializer.hpp
*
* \brief Defines the class to initialize and finalize pybind11.
*        Implemented as Singleton
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef INITIALIZER_HPP_
#define INITIALIZER_HPP_

#include <pybind11/embed.h>
#include <iostream>

namespace Space4AI
{
class Initializer final
{
public:

  /** Initializer.
  *
  *   Initializes pybind11 only if it was not initialized previously.
  */
  static
  Initializer&
  Instance()
  {
    static Initializer InitializerSingleton;
    return InitializerSingleton;
  }

  ~Initializer() { pybind11::finalize_interpreter(); std::cout << "Finalizing pybind11 objects..." << std::endl; }

  Initializer(const Initializer&) = delete;
  Initializer& operator=(const Initializer) = delete;

private:

  Initializer() { pybind11::initialize_interpreter(); std::cout << "Initializing pybind11 objects..." << std::endl;  }

};

} // namespace Space4AI

#endif // INITIALIZER_HPP_
