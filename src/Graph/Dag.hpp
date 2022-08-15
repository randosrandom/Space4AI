/**
* \file Dag.hpp
*
* \brief Defines the class to represent the DAG, using a transition probabilities matrix.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef DAG_HPP_
#define DAG_HPP_

#include <map>
#include <unordered_map>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "external/nlohmann/json.hpp"

#include "src/Logger.hpp"
#include "src/TypeTraits.hpp"

namespace Space4AI
{
/** Class to represent the transition probabilities of a Directed Acyclic Graph.
*
*   The number of rows (equal to the number of columns) represents the number of components.
*   So, in an object of type DAG, dag[i][j] stores the probability to move
*   from component j to component i.
*   The class has methods that automatically find the order of the components
*   of the graph from the .json configuration file, and has data members that assign
*   to each component a representative index.
*/
class DAG
{
	public:

		/** Method to generate a DAG object starting from
		*   the description provided in a nl::json object
		*
		*   \param dag_dict Object of type nl::json extracted from the .json configuration
		*                   file containing data for the DAG
		*
		*   \param components_json Object of type nl::json containing data about components
		*                          extracted from the .json configuration file.
		*/
		void read_from_file(const nlohmann::json& dag_dict,
		    const nlohmann::json& components_json);

		/** Method that returns the size of the DAG, namely the number of components */
		size_t
		size() const { return dag_matrix.size(); };

		/** Method that returns the transition probabilities to a certain node from all
		*   the other components.
		*
		*   \param node Index of the input node
		*   \return Vector of the transition probabilities from all components to the
		*           input Component indexed by node
		*/
		const DagMatrixType::value_type& input_edges(size_t node) const;

		/** dag matrix getter */
		const DagMatrixType&
		get_dag_matrix() const {return dag_matrix;};

		/** getter of the hash map from Component name to Component index */
		const std::unordered_map<std::string, size_t>&
		get_comp_name_to_idx() const {return comp_name_to_idx;};

		/** getter of the ordered map from Component index to Component name */
		const std::map<size_t, std::string>&
		get_idx_to_comp_name() const {return idx_to_comp_name;};

	private:

		/** Method used to find the right ordering of the components.
		*
		*   RFC 7159 standard states that JSON is an unordered collection of elements,
		*   and typical parsers scan the elements using aplhabetical order. However,
		*   this is quite dangerous since names should be assigned taking care of the
		*   order. This method instead finds the exact order (of course, exploiting appropriate
		*   fields of the .json configuration file) of the components in the DAG,
		*   regardless of their names.
		*
		*   \return Vector of permuted indexes (wrt to the canonical indexes assigned with
		*           alphabetical ordering)
		*/
		std::vector<size_t> find_graph_order() const;

	private:

		/** Matrix of transition probabilities.
		*
		*   dag_matrix[i][j] stores the transition probability from node j to node i.
		*/
		DagMatrixType dag_matrix;

		/** Hash map from Component name to the assigned index */
		std::unordered_map<std::string, size_t> comp_name_to_idx;

		/** Ordered map from Conponent index to Component name */
		std::map<size_t, std::string> idx_to_comp_name;

};

} // namespace Space4AI

#endif /* DAG_HPP_ */
