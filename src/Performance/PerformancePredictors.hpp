/**
* \file PerformancePredictors.hpp
*
* \brief Defines the classes that provide the interface between the Performance
*        Models (C++) and the (Python) functions of the a-ML library.
*
* C++/Python interface is provided by pybind11. For efficiency classes are implemented
* through the Meyer's singleton pattern.
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef PERFORMANCEPREDICTORS_HPP_
#define PERFORMANCEPREDICTORS_HPP_

#include <pybind11/embed.h>

#include "TypeTraits.hpp"

namespace Space4AI
{
  namespace py = pybind11;

  // deprecate in PACS ...
  /*
  class CoreBased
  {
  public:

    static
    CoreBased&
    Instance(const std::string& regressor_file)
    {
      // We use the Meyer's trick to istantiate the factory as Singleton
      static CoreBased single_coreBased(regressor_file);
      return single_coreBased;
    }

    TimeType
    predict(
      std::string regressor_file, size_t n_cores, double log_n_cores
    ) const
    {
      return core_based_predictor(
        regressor_file, n_cores, log_n_cores
      ).cast<TimeType>();
    }

  private:

    CoreBased(const std::string& regressor_file):
    core_based_predictor(
      py::module_::import("CoreBased").attr("create_regressor")(regressor_file).attr("predict_from_df")
    )
    {};

    CoreBased(const CoreBased&) = delete;

    CoreBased& operator=(const CoreBased&) = delete;

    const py::object core_based_predictor;

  };
  */

  class Pacsltk
  {
  public:

    static
    Pacsltk&
    Instance()
    {
      // We use the Meyer's trick to istantiate the factory as Singleton
      static Pacsltk single_pacsltk;
      return single_pacsltk;
    }

    TimeType
    predict(
      LoadType part_lambda,
      TimeType demandWarm, TimeType demandCold, TimeType idle_time_before_kill
    ) const
    {
      return pacsltk_predictor(
        part_lambda, demandWarm, demandCold, idle_time_before_kill
      ).cast<TimeType>();
    }

  private:

    Pacsltk(): pacsltk_predictor(py::module_::import("PACSLTK").attr("predictor")) {};

    Pacsltk(const Pacsltk&) = delete;

    Pacsltk& operator=(const Pacsltk&) = delete;

    const py::object pacsltk_predictor;
  };

  // deprecate in PACS ...
  /*
  class MLlibFaas
  {
  public:

    static
    MLlibFaas&
    Instance()
    {
      // We use the Meyer's trick to istantiate the factory as Singleton
      static MLlibFaas single_mllibFaas;
      return single_mllibFaas;
    }

    TimeType
    predict(
      std::string regressor_file,
      LoadType part_lambda,
      TimeType demandWarm, TimeType demandCold, TimeType idle_time_before_kill
    ) const
    {
      return mlibFaas_predictor(regressor_file,
        part_lambda, demandWarm, demandCold, idle_time_before_kill
      ).cast<TimeType>();
    }

  private:

    MLlibFaas(): mlibFaas_predictor(py::module_::import("MLLIBfaas").attr("predictor")) {};

    MLlibFaas(const MLlibFaas&) = delete;

    MLlibFaas& operator=(const MLlibFaas&) = delete;

    const py::object mlibFaas_predictor;

  };
  */

} //namespace Space4AI

#endif /* PERFORMANCEPREDICTORS_HPP_ */
