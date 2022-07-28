/**
* \file Dag.hpp
*
* \brief Defines the class to represent the DAG, using a transition matrix.
*
* \author rando98
* \author giuliamazzeellee
*/
#ifndef DAG_HPP_
#define DAG_HPP_

#include <map>
#include <unordered_map>

#include "external/nlohmann/json.hpp"

#include "src/Logger.hpp"
#include "src/TypeTraits.hpp"

/** Disable implicit conversions FROM a JSON value */
#ifndef JSON_USE_IMPLICIT_CONVERSIONS
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#endif

namespace Space4AI
{
  /**
  *   Class to represent the transition probabilities of a Directed Acyclic Graph.
  *
  *   Number of rows (equal to number of columns) represents the number of components.
  *   So, an object of type DAG, dag[i][j] of stores the probability to move
  *   from component j to component i.
  *   The class have methods that automatically find the order fo the components
  *   of the graph from the json, and have data members that assign to each component
  *   an representative index.
  */
  class DAG
  {
  public:

    /** Method to generate a DAG object starting from
    *   the description provided in a json object
    *
    *   \param dag_dict Object of type json extracted from configuration_file
    *                   containing data for the DAG
    *
    *   \param components_json Object of type json containing data about components
    *                          extracted from the original configuration file.
    */
    void
    read_from_file( const nlohmann::json& dag_dict,
                    const nlohmann::json& components_json);

    /** Method that returns the size of the DAG, namely the number of components */
    size_t
    size() const { return dag_matrix.size(); };

    /**
    *  Method that returns the transition probabilities of the
    *  input edges of a node.
    *
    *  \param node Index of the input node node
    *  \return Vector of the transition probabilies from all components to the
    *          component indexed by node
    */
    const DagMatrixType::value_type&
    input_edges(size_t node) const;

    /** dag matrix getter */
    const DagMatrixType&
    get_dag_matrix() const {return dag_matrix;};

    /** getter of the hash map from component name to component index */
    const std::unordered_map<std::string, size_t>&
    get_comp_name_to_idx() const {return comp_name_to_idx;};

    /** getter of the ordered map from component index to component name */
    const std::map<size_t, std::string>&
    get_idx_to_comp_name() const {return idx_to_comp_name;};

  private:

    /**
    * Method used to find the right ordering of the component.
    * RFC 7159 standard states that JSON is an unordered collection of elements,
    * and typical parsers travers the elements using aplhabetical order. However,
    * this is quite dangerous since, naming should be assigned taking care of the
    * order. This method instead find the exact order (of course, expoliting appropriate fields
    * of the configuration file) the right order of the components in the DAG,
    * regardless their names.
    *
    * \return Vector of permuted index (wrt to the canonical indexes assigned with
    *         alphabetical ordering)
    */
    std::vector<size_t>
    find_graph_order() const;

  private:

    /** Matrix of transition probabilities.
    *
    *   dag_matrix[i][j] stores the transition probability from node j to node i.
    */
    DagMatrixType dag_matrix;

    /** Hash map from component name to the assigned index */
    std::unordered_map<std::string, size_t> comp_name_to_idx;

    /** Ordered map from index to component name.
    *   It is useful when there is the need to sometihing in order
    */
    std::map<size_t, std::string> idx_to_comp_name;

  };

} // namespace Space4AI

#endif /* DAG_HPP_ */
