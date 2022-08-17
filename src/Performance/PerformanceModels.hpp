/**
* \file PerformanceModels.hpp
*
* \brief Defines the classes for the performance evaluators.
*
* Polymorphism is used. Note that some Performance Models constructors
* could accept parameters that will not be used by the class. This trick is
* required to provide the same interface to the object factory builder
* (see PerformanceFactory.hpp). a-ML library predictors are used as pybind11 classes,
* implemented with the Meyer's singleton pattern in the file PerformancePredictors.hpp.
*
* \author Randeep Singh
* \author Giulia Mazzilli
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
*   Abstract class used to represent a performance model for predicting the
*   response time of a Partition object deployed onto a given Resource.
*/
class BasePerformanceModel
{
  	public:

    		/** BasePerformanceModel constructor.
    		*
    		*   \param keyword_ Keyword (name) to identify the model to use
    		*   \param allows_colocation_ true if colocation is allowed, false otherwise
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

    		/** Method to evaluate the performance of a specific Partition object executed
    		*   onto a specific Resource.
    		*
    		*   \param comp_idx Component index
    		*   \param part_idx Partition index (related to component identified by comp_idx)
    		*   \param res_type Type of the Resource
    		*   \param res_idx Resource index
    		*   \param system_data Reference to all the SystemData read from the .json configuration file
    		*   \param solution_data Reference to the SolutionData, namely SolutionData.y_hat
    		*                        and SolutionData.used_resources
    		*   \return response time of the execution
    		*
    		*   The method will be overridden by the performance classes
    		*/
    		virtual
    		TimeType predict(
    		    size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
    		    const SystemData& system_data,
    		    const SolutionData& solution_data
    		) const = 0;

    		/** virtual destructor */
    		virtual ~BasePerformanceModel() = default;

  	protected:

    		/** Name of the model */
    		const std::string keyword;

    		/** Boolean variable to determine whether colocation is allowed or not */
    		const bool allows_colocation;
};

/** Class to define queue-servers performance models.
*
*   Available for ResourceType::Edge and ResourceType::VM.
*/
class QTPE: public BasePerformanceModel
{

  	public:

    		/** QTPE constructor.
    		*
    		*   \param keyword_ Keyword (name) to identify the model to use
    		*   \param allows_colocation_ true if colocation is allowed, false otherwise
    		*   \param demand_ Demand time
    		*/
    		QTPE(
    		    const std::string& keyword_,
    		    bool allows_colocation_,
    		    TimeType demand_
    		):
    			BasePerformanceModel(keyword_, allows_colocation_),
    			demand(demand_)
    		{}

    		/** Abstract method of BasePerformanceModel overridden. */
    		virtual
    		TimeType predict(
    		    size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
    		    const SystemData& system_data,
    		    const SolutionData& solution_data
    		) const override;


    		/** Method to compute the utilization of a specific Resource object.
    		*
    		*   \param res_type Type of the Resource
    		*   \param res_idx Resource index
    		*   \param system_data Reference to all the SystemData read from the .json configuration file
    		*   \param solution_data Reference to the SolutionData, namely SolutionData.y_hat
    		*                        and SolutionData.used_resources
    		*   \return utilization of the Resource
    		*/
    		static
    		double compute_utilization(
    		    ResourceType res_type, size_t res_idx,
    		    const SystemData& system_data,
    		    const SolutionData& solution_data
    		);

    		/** all_demands setter */
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

    		/** Demand time */
    		const TimeType demand;

    		/** Demand times of the other Component and Partition objects that could be run on
    		*   the specific Resource
    		*/
    		inline
    		static
    		DemandEdgeVMType all_demands = {};

};

/** Abstract class inherited from BasePerformanceModel, to represent
*   ResourceType::Faas resources performance models.
*/
class FaasPE: public BasePerformanceModel
{
  	public:

    		/** FaasPE constructor.
    		*
    		*   \param keyword_ Keyword (name) to identify the model to use
    		*   \param allows_colocation_ true if colocation is allowed, false otherwise
    		*   \param demandWarm_ Demand time when there is an active server available on the platform
    		*   \param demandCold_ Demand time when all the servers on the platform are down and a new one
    		*                      needs to be activated
    		*/
    		FaasPE(
    		    const std::string& keyword_,
    		    bool allows_colocation_,
    		    TimeType demandWarm_,
    		    TimeType demandCold_
    		):
    			BasePerformanceModel(keyword_, allows_colocation_),
    			demandWarm(demandWarm_), demandCold(demandCold_)
    		{}

    		/** demandWarm getter */
    		virtual
    		TimeType
    		get_demandWarm() const {return demandWarm; }

    		/** demandCold getter */
    		virtual
    		TimeType
    		get_demandCold() const {return demandCold; }

    		/** virtual destructor */
    		virtual ~FaasPE() = default;

  	protected:

    		/** Demand time when there is at least one active server available on the platform */
    		const TimeType demandWarm;

    		/** Demand time when all the servers on the platform are down and a new one needs to be activated */
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
    		double predict(
    		    size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
    		    const SystemData& system_data,
    		    const SolutionData& solution_data
    		) const override;

    		virtual ~FaasPacsltkPE() = default;

  	private:

  		  Pacsltk& predictor;

};

/** Class to define AWS Lambda FaaS performance models.
*
*   Available only for ResourceType::Faas resources.
*/
class FaasPacsltkStaticPE: public FaasPE
{

  	public:

    		/** FaasPacsltkStaticPE constructor.
    		*
    		*   \param keyword_ Keyword (name) to identify the model to use
    		*   \param allows_colocation_ true if colocation is allowed, false otherwise
    		*   \param demandWarm_ Demand time when there is an active server available on the platform
    		*   \param demandCold_ Demand time when all the servers on the platform are down and a new one
    		*                      needs to be activated
    		*   \param idle_time_before_kill How long does the platform keep the servers up after being idle
    		*   \param part_lambda Load factor of the Partition
    		*/
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

    		/** Abstract method of BasePerformanceModel overridden. */
    		virtual
    		double predict(
    		    size_t comp_idx, size_t part_idx, ResourceType res_type, size_t res_idx,
    		    const SystemData& system_data,
    		    const SolutionData& solution_data
    		) const override;

    		/** virtual destructor */
    		virtual ~FaasPacsltkStaticPE() = default;

  	private:

    		/** Demand time */
    		const TimeType demand;

};

} //namespace Space4AI

#endif /* PERFORMANCEMODELS_HPP_ */
