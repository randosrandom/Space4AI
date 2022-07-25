/**
* \file Dag.cpp
*
* \brief Definition of the methods of class Graph
*
* \author rando98
* \author giuliamazzeellee
*/

#include <iostream>
#include <numeric>

#include "Dag.hpp"

namespace Space4AI
{
  void
  DAG::read_from_file(  const nlohmann::json& dag_dict,
                      const nlohmann::json& components_json)
  {

    // Read Components name (Reading from the field DirectedAcyclicGraph is not sufficient since I don't have the leaf nodes saved ... )
    for(const auto& [name, value]: components_json.items())
    {
      this->comp_name_to_idx.emplace(name, comp_name_to_idx.size());
      this->idx_to_comp_name.emplace(idx_to_comp_name.size(), name);

      std::string debug_message = "Component " + name + " added to comp_name_to_idx";
      Logger::Debug(debug_message);
    }

    const size_t num_nodes = this->comp_name_to_idx.size();

    this->dag_matrix = DagMatrixType(num_nodes, DagMatrixType::value_type(num_nodes, 0.0));

    for(const auto& [key, value]: dag_dict.items())
    {
      if(comp_name_to_idx.count(key))
      {
        for(size_t i=0; i<value.at("next").size(); ++i)
        {
          this->dag_matrix  [ comp_name_to_idx.at( value.at("next")[i] )   ]
                            [ comp_name_to_idx.at( key )                   ]
                            = value.at("transition_probability")[i];
        }
      }
      else
      {
        const std::string err_message = "Component: " + key + " not found in *DirectedAcyclicGraph* in json";
        Logger::Error(err_message);
        throw std::logic_error("Incoherence between *DirectedAcyclicGraph* and *Component* in Design time json\n" + err_message);
      }
    }

    // get ordered indexes by component position in DAG
    std::vector<size_t> ordered_index = find_graph_order();

    // assign the new indexes, so that they are ordered
    // E.g., if i<j => component i comes not after than component j!
    auto idx_to_comp_name_temp = this->idx_to_comp_name;
    for(std::size_t i=0; i<size(); i++)
    {
      this->idx_to_comp_name.at(i) = idx_to_comp_name_temp[ordered_index[i]];
      this->comp_name_to_idx.at(idx_to_comp_name.at(i)) = i;
    }

    // reset the dag_matrix
    this->dag_matrix = DagMatrixType(num_nodes, DagMatrixType::value_type(num_nodes, 0.0));

    // re-populate the dag matrix reading the json in order!
    for(const auto& [idx, name]: idx_to_comp_name)
    {
      if(dag_dict.contains(name))
      {
        for(size_t i=0; i<dag_dict.at(name).at("next").size(); ++i)
        {
          dag_matrix  [ comp_name_to_idx.at(dag_dict.at(name).at("next")[i] )]
                      [ idx ]
                      = dag_dict.at(name).at("transition_probability")[i];
        }
      }
    }

  }

  std::vector<size_t>
  DAG::find_graph_order() const // Think about the recursive version!
  {
    const size_t num_nodes = comp_name_to_idx.size();

    std::vector<size_t> permutation_for_order;
    permutation_for_order.reserve(num_nodes);

    std::vector<bool> index_already_permuted(num_nodes, false);

    // find origin of graph
    auto zero_vec = DagMatrixType::value_type(num_nodes, 0.);
    for(std::size_t i = 0; i < num_nodes; ++i)
    {
      if(dag_matrix[i] == zero_vec) // no nodes enters in node i => i root node
      {
        permutation_for_order.push_back(i);
      }
    }

    index_already_permuted[permutation_for_order.back()] = true;

    std::vector<size_t> old_added_indexes = {permutation_for_order.back()};

    // start reordering
    while(permutation_for_order.size() < num_nodes)
    {
      std::vector<size_t> new_added_indexes;

      for(size_t root_idx: old_added_indexes)
      {
        for(std::size_t i = 0; i < num_nodes; i++)
        {
          if(!index_already_permuted[i] && input_edges(i)[root_idx] > 0)
          {
            index_already_permuted[i] = true;
            permutation_for_order.push_back(i);
            new_added_indexes.push_back(i);
          }
        }
      }
      old_added_indexes = new_added_indexes;
    }

    return permutation_for_order;
  }

  const DagMatrixType::value_type&
  DAG::input_edges(size_t node) const
  {
    if(node < this->size())
      return dag_matrix[node];
    else
    {
      const std::string err_message = "Trying to access non-existent node: " + std::to_string(node) + " in dag_matrix";
      Logger::Error(err_message);
      throw std::out_of_range("Accessing invalid position in dag_matrix");
    }
  }

} // namespace Space4AI
