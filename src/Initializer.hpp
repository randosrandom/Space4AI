#ifndef INITIALIZER_HPP_
#define INITIALIZER_HPP_

#include <pybind11/embed.h>

namespace Space4AI
{
class Initializer final
{
public:

  static
  Initializer&
  Instance()
  {
    static Initializer InitializerSingleton;
    return InitializerSingleton;
  }

private:

  Initializer() { pybind11::initialize_interpreter(); }

  ~Initializer() { pybind11::finalize_interpreter(); }

};

} // namespace Space4AI

#endif // INITIALIZER_HPP_
