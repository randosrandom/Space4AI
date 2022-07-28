/**
* \file PerformanceModels.hpp
*
* \brief Defines the classes for the performance evaluators.
*
* Polymorphism is used. Note that some Performance Models constructor
* could accept parameters that will not be used by the class. This trick is
* required to provide the same interface to the object factory builder.
* a-ML library predictor are used as pybind11 classes, implemented with
* the Meyer's singleton pattern in the file PerformancePredictors.hpp
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef PERFORMANCEMODELS_HPP_
#define PERFORMANCEMODELS_HPP_

#include "src/Performance/PerformancePredictors.hpp"
#include "src/Solution/SolutionData.hpp"
#include "src/System/SystemData.hpp"
#include "src/TypeTraits.hpp"

namespace Space4AI
{
  /** Abstract class to represent the parent of all performance models that will
  *   inherit from it.
  *
  * Abstract class used to represent a performance model for predicting the
  * response time of a Partition object deployed onto a given Resource.
  */
  class BasePerformanceModel
  {
  public:

    /*
    * BasePerformanceModel constructor.
    *
    * \param keyword_ Keyword (name) to identify the model to use
    * \param allows_colocation_ true if colocation is allowed
    */
    BasePerformanceModel(
      const std::string& keyword_,
      bool allows_colocation_
    ): keyword(keyword_), allows_colocation(allows_colocation_)
    {}

    /** keyword getter */
    std::string
    get_keyword() const {return keyword;}

    /** allows_colocation getter */
    bool
    get_allows_colocation() const {return allows_colocation;}

    /**
    * Method to evaluate the performance of a specific Partition object executed onto a specific Resource.
    * \param comp_idx Component index
    * \param part_idx Partition index (related to component identified by comp_idx)
    * \param res_type Type of the resource
    * \param res_idx Resource index
    * \param system_data Reference to all the system data read from configuration_file
    * \param solution_data Reference to the solution data, namely y_hat and used_resources
    *
    * The method will be overridden by the performance classes
    */
    virtual
    TimeType
    predict(
      size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
      const SystemData& system_data,
      const SolutionData& solution_data
    ) const = 0;

    /** virtual destructor */
    virtual ~BasePerformanceModel() = default;

  protected:

    /** name of the model */
    const std::string keyword;

    /** Determine whether colocation is allowed or not */
    const bool allows_colocation;
  };

  /**
  * Class to define queue-servers performance models.
  *
  * Available for Edge and VM.
  */
  class QTPE: public BasePerformanceModel
  {

  public:

    /**
    * QTPE constructor.
    * \param keyword_
    * \param allows_colocation
    * \param demandWarm_ Demand time
    */
    QTPE(
      const std::string& keyword_,
      bool allows_colocation_,
      TimeType demand_
    ):
    BasePerformanceModel(keyword_, allows_colocation_),
    demand(demand_)
    {}

    /**
    * Abstract method predict() overridden.
    */
    virtual
    TimeType
    predict(
      size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
      const SystemData& system_data,
      const SolutionData& solution_data
    ) const override;


    /**
    * Method to compute the utilization of a specific Resource object.
    */
    static
    double
    compute_utilization(
      ResourceType res_type, size_t res_idx,
      const SystemData& system_data,
      const SolutionData& solution_data
    );


    template<class T>
    static
    void
    set_all_demands(T&& all_demands_)
    {
      QTPE::all_demands = std::forward<T>(all_demands_);
    }

    /** virtual destructor */
    virtual ~QTPE() = default;

  private:

    /** demand time */
    const TimeType demand;

    /** demand times of the other Components and Partition that could be run on the specific resource */
    inline
    static
    DemandEdgeVMType all_demands = {};

  };

  // deprecate in PACS
  /*
  class CoreBasedPE: public BasePerformanceModel
  {

  public:

    CoreBasedPE(
      const std::string& keyword_,
      bool allows_colocation_,
      const std::string& regressor_file_
    ):
    BasePerformanceModel(keyword_, allows_colocation_),
    regressor_file(regressor_file_),
    predictor(CoreBased::Instance(regressor_file))
    {}

    virtual
    double
    predict(
      size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
      const SystemData& system_data,
      const SolutionData& solution_data
    ) const override;

    virtual ~CoreBasedPE() = default;

  private:

    const std::string regressor_file;

    CoreBased& predictor;

  };
  /**
  * Abstract class inherited from BasePerformanceModel, to represent
  * FaaS resources performance models.
  */
  class FaasPE: public BasePerformanceModel
  {
  public:

    FaasPE(
      const std::string& keyword_,
      bool allows_colocation_,
      TimeType demandWarm_,
      TimeType demandCold_
    ):
    BasePerformanceModel(keyword_, allows_colocation_),
    demandWarm(demandWarm_), demandCold(demandCold_)
    {}

    virtual
    TimeType
    get_demandWarm() const {return demandWarm; }

    virtual
    TimeType
    get_demandCold() const {return demandCold; }

    virtual ~FaasPE() = default;

  protected:

    const TimeType demandWarm;

    const TimeType demandCold;

  };

  class __attribute__((__visibility__("hidden"))) FaasPacsltkPE: public FaasPE
  {

  public:

    FaasPacsltkPE(
      const std::string& keyword_,
      bool allows_colocation_,
      TimeType demandWarm_,
      TimeType demandCold_
    ):
    FaasPE(keyword_, allows_colocation_, demandWarm_, demandCold_),
    predictor(Pacsltk::Instance())
    {}

    virtual
    double
    predict(
      size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
      const SystemData& system_data,
      const SolutionData& solution_data
    ) const override;

    virtual ~FaasPacsltkPE() = default;

  private:

    Pacsltk& predictor;

  };

  class FaasPacsltkStaticPE: public FaasPE
  {

  public:

    FaasPacsltkStaticPE(
      const std::string& keyword_,
      bool allows_colocation_,
      TimeType demandWarm_,
      TimeType demandCold_,
      TimeType idle_time_before_kill,
      LoadType part_lambda
    ):
    FaasPE(keyword_, allows_colocation_, demandWarm_, demandCold_),
    demand(Pacsltk::Instance().predict(part_lambda, demandCold, demandWarm, idle_time_before_kill))
    {}

    virtual
    double
    predict(
      size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
      const SystemData& system_data,
      const SolutionData& solution_data
    ) const override;

    virtual ~FaasPacsltkStaticPE() = default;

  private:

    const TimeType demand;

  };

  // deprecate in PACS
  /*
  class FaasMLlibPE: public FaasPE
  {

  public:

    FaasMLlibPE(
      const std::string& keyword_,
      bool allows_colocation_,
      TimeType demandWarm_,
      TimeType demandCold_,
      std::string regressor_file_
    ):
    FaasPE(keyword_, allows_colocation_, demandWarm_, demandCold_),
    regressor_file(regressor_file_),
    predictor(MLlibFaas::Instance())
    {}

    virtual
    double
    predict(
      size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
      const SystemData& system_data,
      const SolutionData& solution_data
    ) const override;


    virtual ~FaasMLlibPE() = default;

  private:

    const std::string regressor_file;

    MLlibFaas& predictor;

  };
  */

} //namespace Space4AI

#endif /* PERFORMANCEMODELS_HPP_ */
