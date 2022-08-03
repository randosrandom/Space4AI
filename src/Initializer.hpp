#ifndef INITIALIZER_HPP_
#define INITIALIZER_HPP_

#include <pybind11/embed.h>
#include <iostream>

namespace Space4AI
{
class Initializer final
{

public:

  Initializer()
  {
    if(!already_initialized)
    {
      pybind11::initialize_interpreter();
      already_initialized = true;
    }
    else
    {
      std::cout << "It's forbidden to Initialize more than once; pybind11 is already initialized" << std::endl;
    }
  }

  ~Initializer()
  {
    if(already_initialized)
    {
      pybind11::finalize_interpreter();
      already_initialized = false;
    }
  }

private:

  inline static bool already_initialized = false;

};

} // namespace Space4AI

#endif // INITIALIZER_HPP_
