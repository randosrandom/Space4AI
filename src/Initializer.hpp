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
/**
* Class to initialize and finalize pybind11.
*/
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

    ~Initializer() { pybind11::finalize_interpreter(); std::cout << "Finalized pybind11 objects!" << std::endl; }

    Initializer(const Initializer&) = delete;
    Initializer& operator=(const Initializer) = delete;

  private:

    Initializer() { pybind11::initialize_interpreter(); std::cout << "Initialized pybind11 objects!" << std::endl;  }

};

} // namespace Space4AI

#endif // INITIALIZER_HPP_
