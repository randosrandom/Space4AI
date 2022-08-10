/**
* \file Solution.hpp
*
* \brief Defines the class to initialize pybind11.
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
/** Class to intialize pybind11. */
class Initializer final
{

	public:

		/** Initializer class constructor.
		*
		*   Initializes pybind11 only if it was not initialized previously
		*   by using the boolean member already_initialized.
		*/
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

		/** Initializer class destructor.
		*
		*  Finalizes pybind11 and sets already_initialized back to false.
		*/
		~Initializer()
		{
			if(already_initialized)
			{
				pybind11::finalize_interpreter();
				already_initialized = false;
			}
		}

	private:

		/** Boolean variable to store the status of pybind11:
		*   true = already initialized,
		*   false = not initialized.
		*/
		inline static bool already_initialized = false;

};

} // namespace Space4AI

#endif // INITIALIZER_HPP_
