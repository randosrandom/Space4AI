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
* \file Dag.cpp
*
* \brief Definition of the methods of the class Graph.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#include <iostream>
#include <numeric>

#include "src/Graph/Dag.hpp"

namespace Space4AI
{
void
DAG::read_from_file(const nlohmann::json& dag_dict,
  const nlohmann::json& components_json)
{
  // Read Components name (Reading from the field DirectedAcyclicGraph is not sufficient since I don't have the leaf nodes saved ... )
  for(const auto& [name, value] : components_json.items())
  {
    this->comp_name_to_idx.emplace(name, comp_name_to_idx.size());
    this->idx_to_comp_name.emplace(idx_to_comp_name.size(), name);
    std::string debug_message = "Component " + name + " added to comp_name_to_idx";
    Logger::Debug(debug_message);
  }

  const size_t num_nodes = this->comp_name_to_idx.size();
  this->dag_matrix = DagMatrixType(num_nodes, DagMatrixType::value_type(num_nodes, 0.0));

  for(const auto& [key, value] : dag_dict.items())
  {
    if(comp_name_to_idx.count(key))
    {
      for(size_t i = 0; i < value.at("next").size(); ++i)
      {
        this->dag_matrix[comp_name_to_idx.at(value.at("next")[i].get<std::string>())]
                        [comp_name_to_idx.at(key)]
          = value.at("transition_probability")[i].get<double>();
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

  for(std::size_t i = 0; i < size(); i++)
  {
    this->idx_to_comp_name.at(i) = idx_to_comp_name_temp[ordered_index[i]];
    this->comp_name_to_idx.at(idx_to_comp_name.at(i)) = i;
  }

  // reset the dag_matrix
  this->dag_matrix = DagMatrixType(num_nodes, DagMatrixType::value_type(num_nodes, 0.0));

  // re-populate the dag matrix reading the json in order!
  for(const auto& [idx, name] : idx_to_comp_name)
  {
    if(dag_dict.contains(name))
    {
      for(size_t i = 0; i < dag_dict.at(name).at("next").size(); ++i)
      {
        dag_matrix[comp_name_to_idx.at(dag_dict.at(name).at("next")[i].get<std::string>())]
                  [idx]
          = dag_dict.at(name).at("transition_probability")[i].get<double>();
      }
    }
  }
}

std::vector<size_t>
DAG::find_graph_order() const
{
  const size_t num_nodes = comp_name_to_idx.size();
  std::vector<size_t> permutation_for_order;
  permutation_for_order.reserve(num_nodes);
  std::vector<bool> index_already_permuted(num_nodes, false);

  // should be wise to add check to avoid infinite while loop
  while(permutation_for_order.size() < num_nodes)
  {
    find_next_root(permutation_for_order, index_already_permuted);
  }
  return permutation_for_order;
}

void
DAG::find_next_root(
  std::vector<size_t>& permutation_for_order,
  std::vector<bool>& index_already_permuted) const
{
  std::vector<size_t> new_added_indexes;
  new_added_indexes.reserve(index_already_permuted.size());

  for(std::size_t i = 0; i < index_already_permuted.size(); ++i)
  {
    if(!index_already_permuted[i])
    {
      bool next = true;
      for(size_t j=0; j<dag_matrix[i].size() && next; ++j)
      {
        if(!index_already_permuted[j] && dag_matrix[i][j] != 0)
          next = false;
      }
      if(next)
        new_added_indexes.push_back(i);
    }
  }
  for(size_t i : new_added_indexes)
  {
    permutation_for_order.push_back(i);
    index_already_permuted[i] = true;
  }
}

const DagMatrixType::value_type&
DAG::input_edges(size_t node) const
{
  if(node < this->size())
  {
    return dag_matrix[node];
  }
  else
  {
    const std::string err_message = "Trying to access non-existent node: " + std::to_string(node) + " in dag_matrix";
    Logger::Error(err_message);
    throw std::out_of_range("Accessing invalid position in dag_matrix");
  }
}

} // namespace Space4AI
