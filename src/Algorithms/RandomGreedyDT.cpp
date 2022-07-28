/**
* \file RandomGreedyDT.cpp
*
* \brief Defines the methods of the RandomGreedyDT class
*
* \author rando98
* \author giuliamazzeellee
*/

#include "src/Algorithms/RandomGreedyDT.hpp"

#include <random>
//#include <omp.h>

namespace Space4AI
{

  EliteResult
  RandomGreedyDT::random_greedy(
    const System& system,
    size_t max_it,
    size_t num_top_sols
  )
  {
    Logger::Info("Starting Random Greedy DT algorithm");

    EliteResult elite(num_top_sols);
    Logger::Info("Elite container initialized with " + std::to_string(num_top_sols) + " spaces");

    // #pragma omp parallel default(none) shared(max_it, num_top_sols, elite, system)
  //  {

    // #pragma omp for schedule(dynamic)
    for(size_t it=0; it<max_it; ++it)
    {

      Logger::Info("**** iteration: " + std::to_string(it) +  " ****");

      Solution new_sol(std::move(step(system)));

      if(new_sol.get_feasibility())
      {
        // #pragma omp critical
        elite.add(std::move(new_sol));

        Logger::Info("******** NEW RESULT ADDED TO ELITE *******");
      }

    }

  //  } # pragma omp parallel

    Logger::Info("Finished Random Greedy DT algorithm");

    return elite;
  }

  Solution
  RandomGreedyDT::step(const System& system)
  {
    //generate a random solution
    Logger::Debug("step: Creating a random initial solution...");
    Solution sol(create_random_initial_solution(system));
    Logger::Debug("step: RANDOM INITIAL SOLUTION CREATED");

    //check feasibility and compute performance
    Logger::Debug("step: Checking feasibility and computing performance");
    bool feasible = sol.check_feasibility(system);
    Logger::Debug("step: Done!");

    // if the solution is feasible, compute the corresponding cost before and after
    // updating the cluster size
    if (feasible)
    {
      Logger::Debug("step: The solution is feasible");

      const std::vector<size_t> res_type_idxs = {ResIdxFromType(ResourceType::Edge), ResIdxFromType(ResourceType::VM)};
      const UsedResourcesNumberType& n_used_resources = sol.get_n_used_resources();

      // update the cluster size of edge and VM resources
      // loop on edge and VM types
      for(size_t res_type_idx : res_type_idxs)
      {
        //loop on resources
        for (size_t res_idx = 0; res_idx < n_used_resources[res_type_idx].size(); ++res_idx)
        {
          if(n_used_resources[res_type_idx][res_idx] > 1)
          {
            sol = std::move(reduce_cluster_size(sol, res_type_idx, res_idx, system));
          }
        }
      }

      // compute new cost of the solution
      sol.objective_function(system);
    }
    else
    {
      Logger::Debug("step: The solution is NOT feasible");
    }

    return sol;
  }

  Solution
  RandomGreedyDT::create_random_initial_solution(
    const System& system
  )
  {
    std::mt19937 rng;

    if(reproducibility)
    {
      static size_t seed_ = 121298;
      seed_ += 1000;
      rng.seed(seed_);
    }
    else
    {
      std::random_device dev;
      rng.seed(dev());
    }

    Solution solution;

    const auto& system_data = system.get_system_data();

    const auto& components = system_data.get_components();
    const auto& cls = system_data.get_cls();
    const auto& all_resources = system_data.get_all_resources();
    const auto& comp_name_to_idx = system_data.get_comp_name_to_idx();
    const auto& compatibility_matrix = system_data.get_compatibility_matrix();

    Logger::Debug("create_random_initial_solution: Initializing and resizing members ...");

    //declare solution members
    YHatType y_hat;
    UsedResourcesOrderedType used_resources; //vettore di vettore ORDINATO di tuple
    UsedResourcesNumberType n_used_resources;

    const size_t comp_num = comp_name_to_idx.size();
    const size_t res_type_idx_count = ResIdxFromType(ResourceType::Count);

    //resize n_used_resources
    n_used_resources.resize(2); // Edge, VM (don't like the two ...)
    n_used_resources[ResIdxFromType(ResourceType::Edge)].resize(all_resources.get_number_resources(ResIdxFromType(ResourceType::Edge))); // Index of Edge must be 0
    n_used_resources[ResIdxFromType(ResourceType::VM)].resize(all_resources.get_number_resources(ResIdxFromType(ResourceType::VM)));    // Index of VM must be 1

    // resize y_hat
    y_hat.resize(comp_num);
    used_resources.resize(comp_num);

    //loop on components
    for(size_t i=0; i<comp_num; ++i)
    {
      y_hat[i].resize(res_type_idx_count);

      //loop on resource types
      for(size_t j=0; j<res_type_idx_count; ++j)
      {
        y_hat[i][j].resize(components[i].get_partitions().size());

        //loop on partitions
        for(size_t k=0; k<y_hat[i][j].size(); ++k)
        {
          y_hat[i][j][k].resize(all_resources.get_number_resources(j));
        }
      }
    }

    std::vector<std::vector<bool>> candidate_resources; //per ogni tipo di risorsa, per ogni risorsa, si o no

    candidate_resources.resize(res_type_idx_count);

    // initialize candidate resources
    for(size_t i=0; i<res_type_idx_count; ++i)
    {
      const size_t n_res = all_resources.get_number_resources(i);

      if(i == ResIdxFromType(ResourceType::Faas))
        candidate_resources[i].resize(n_res, true); // if we have faas, we have to select all res
      else
        candidate_resources[i].resize(n_res, false); // other we have to select the candidate_resources
    }

    //loop on computational layers and select one resource for each cl and all resources in faas cls
    //loop on resource type
    Logger::Debug("create_random_initial_solution: Selecting candidate resources for Edge and VM...");
    for(size_t i=0; i<res_type_idx_count - 1; ++i) // Faas already selected!
    {
      {
        Logger::Trace("create_random_initial_solution: edge or VM resource");
        //loop over the cls
        for(size_t j=0; j<cls[i].size(); ++j)
        {
          //pick a random resource of the cl
          const std::vector<size_t> res_idxs = cls[i][j].get_res_idxs();
          const size_t n_res = res_idxs.size();
          std::uniform_int_distribution<decltype(rng)::result_type> dist(0, n_res-1);
          const size_t random_res_idx = res_idxs[dist(rng)];
          candidate_resources[i][random_res_idx] = true;
          Logger::Trace("create_random_initial_solution:  selecting resource " + std::to_string(random_res_idx) + " of type " + std::to_string(i));
        }
      }
    }

    //Assign components
    //loop over components
    Logger::Debug("create_random_initial_solution: Assigning the components...");
    for(size_t comp_idx=0; comp_idx<components.size(); ++comp_idx)
    {
      Logger::Trace("create_random_initial_solution:  ***** Component: " + std::to_string(comp_idx) + " ******");
      //randomly select a deployment for that component
      const auto& deployments = components[comp_idx].get_deployments();
      const size_t n_dep = deployments.size();
      std::uniform_int_distribution<decltype(rng)::result_type> distr(0,n_dep-1);
      const size_t random_dep_idx = distr(rng);
      const auto& random_dep = deployments[random_dep_idx];
      Logger::Trace("create_random_initial_solution: Selected deployment: " + std::to_string(random_dep_idx));

      //loop over all partitions in the deployment
      for(size_t part_idx : random_dep.get_partition_indices())
      {
        Logger::Trace("create_random_initial_solution: Partition: " + std::to_string(part_idx));
        //list of the resources in the intersection between candidate and compatible resources.
        //pair.first= resource type, pair.second= resource idx
        std::vector<std::pair<size_t, size_t>> resources_instersection;
        //get the indices of compatible resources and compute the
        //intersection with the selected resources in each
        //computational layer
        for(size_t res_type_idx=0; res_type_idx<res_type_idx_count; ++res_type_idx)
        {
          //loop over the resources of type j
          for(size_t res_idx=0; res_idx<candidate_resources[res_type_idx].size(); ++res_idx)
          {
            //compute the intersection
            if(compatibility_matrix[comp_idx][res_type_idx][part_idx][res_idx] && candidate_resources[res_type_idx][res_idx])
            {
              resources_instersection.emplace_back(res_type_idx,res_idx);
              Logger::Trace("create_random_initial_solution: added to the intersection resource " + std::to_string(res_idx) + " of type " + std::to_string(res_type_idx));
            }
          }
        }
        // OBS: resources_instersection is NEVER EMPTY! Indeed, all FaaS are candidate resource
        // so, at worst, we select Faas for the partition
        // THAT'S WHY I DO NOT HAVE TO CHECK Segmentation fault here
        const size_t n_inter_res = resources_instersection.size();
        std::uniform_int_distribution<decltype(rng)::result_type> dist(0,n_inter_res-1);
        const size_t random_idx = dist(rng);
        const auto& random_resource = resources_instersection[random_idx];

        //update used resources and y_hat
        used_resources[comp_idx].emplace_back(part_idx, random_resource.first, random_resource.second);
        y_hat[comp_idx][random_resource.first][part_idx][random_resource.second] = 1;

        Logger::Trace("create_random_initial_solution: Updated y, y_hat and used_resources");

      }
      //COS'è E A COSA SERVE RES_PART_RANDOM???
    }

    std::vector<std::vector<bool>> already_updated_cluster_size(2);
    already_updated_cluster_size[ResIdxFromType(ResourceType::Edge)].resize(all_resources.get_number_resources(ResIdxFromType(ResourceType::Edge)), false);
    already_updated_cluster_size[ResIdxFromType(ResourceType::VM)].resize(all_resources.get_number_resources(ResIdxFromType(ResourceType::VM)), false);

    //loop over edge and VM types
    Logger::Debug("create_random_initial_solution: Selecting number of edge and VM resources...");

    for(size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx)
    {
      for (auto [part_idx, res_type_idx, res_idx]: used_resources[comp_idx])
      {
        if(res_type_idx == ResIdxFromType(ResourceType::Edge) || res_type_idx == ResIdxFromType(ResourceType::VM))
        {
          Logger::Trace("create_random_initial_solution: resource of type: " + std::to_string(res_type_idx) + " resource index: " + std::to_string(res_idx));

          if(already_updated_cluster_size[res_type_idx][res_idx])
          {
            y_hat[comp_idx][res_type_idx][part_idx][res_idx] = n_used_resources[res_type_idx][res_idx];
            Logger::Trace("create_random_initial_solution: Updated number of resources of comp " + std::to_string(comp_idx) + \
            " part " + std::to_string(part_idx) + " to " + std::to_string(n_used_resources[res_type_idx][res_idx]));
          }
          else
          {
            already_updated_cluster_size[res_type_idx][res_idx] = true;

            const size_t number_avail = all_resources.get_number_avail(ResTypeFromIdx(res_type_idx), res_idx);
            std::uniform_int_distribution<decltype(rng)::result_type> dist(1,number_avail);
            const size_t random_number = dist(rng);

            y_hat[comp_idx][res_type_idx][part_idx][res_idx] = random_number;
            n_used_resources[res_type_idx][res_idx] = random_number;
            Logger::Trace("create_random_initial_solution: Updated number of resources of comp " + std::to_string(comp_idx) + \
            " part " + std::to_string(part_idx) + " to " + std::to_string(random_number));
          }
        }
      }
    }

    /*
    for(size_t res_type_idx = 0; res_type_idx < res_type_idx_count - 1; ++res_type_idx)
    {
      const auto res_type = ResTypeFromIdx(res_type_idx);

      const size_t n_res = all_resources.get_number_resources(res_type_idx);
      Logger::Trace("create_random_initial_solution: resource of type: " + std::to_string(res_type_idx));

      //loop over edge or VM resources
      for(size_t res_idx = 0; res_idx < n_res; ++res_idx)
      {
        Logger::Trace("create_random_initial_solution: resource: " + std::to_string(res_idx));
        //randomly generate the number of resources that can be assigned
        //to the partitions that run on that resource
        const size_t number_avail = all_resources.get_number_avail(res_type, res_idx);
        std::uniform_int_distribution<decltype(rng)::result_type> dist(1,number_avail);
        const size_t random_number = dist(rng);

        //loop over components
        for(size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx)
        {
          const auto& partitions = components[comp_idx].get_partitions();

          for(size_t part_idx = 0; part_idx < partitions.size(); ++part_idx)
          {
            //if the partition runs on the current resource,update the number
            if(y_hat[comp_idx][res_type_idx][part_idx][res_idx] > 0)
            {
              y_hat[comp_idx][res_type_idx][part_idx][res_idx] = random_number;
              n_used_resources[res_type_idx][res_idx] = random_number;
              Logger::Trace("create_random_initial_solution: Updated number of resources of comp " + std::to_string(comp_idx) + \
              " part " + std::to_string(part_idx) + " to " + std::to_string(random_number));
            }
          }
        }
      }
    }
    */

    Logger::Debug("create_random_initial_solution: Initializing new random solution...");

    solution.set_y_hat(std::move(y_hat));
    solution.set_used_resources(std::move(used_resources));
    solution.set_n_used_resources(std::move(n_used_resources));

    Logger::Debug("create_random_initial_solution: Done!");

    return solution;
  }

  Solution
  RandomGreedyDT::reduce_cluster_size(
    const Solution& solution,
    const size_t res_type_idx,
    const size_t res_idx,
    const System& system
  )
  {
    const auto& used_resources = solution.get_used_resources();

    Solution old_sol = solution;
    Solution new_sol = solution;

    auto y_hat_new = new_sol.get_y_hat();
    auto n_used_resources_new = new_sol.get_n_used_resources();

    bool feasible = true;

    Logger::Debug("reduce_cluster_size: Reducing cluster size...");

    do
    {
      n_used_resources_new[res_type_idx][res_idx]--;

      for(size_t comp_idx=0; comp_idx < used_resources.size(); ++comp_idx)
      {
        Logger::Trace("reduce_cluster_size: currently on component: " + std::to_string(comp_idx));

        for(auto [p_idx, r_type_idx, r_idx] : used_resources[comp_idx])
        {
          if(res_type_idx == r_type_idx && res_idx == r_idx)
          {
            y_hat_new[comp_idx][r_type_idx][p_idx][r_idx]--;
          }
        }
      }

      new_sol.set_y_hat(y_hat_new);
      new_sol.set_n_used_resources(n_used_resources_new);
      feasible = new_sol.check_feasibility(system);

      if(feasible)
      {
        Logger::Debug("reduce_cluster_size: The solution is still feasible");
        old_sol = new_sol;
      }
      else
      {
        Logger::Debug("reduce_cluster_size: The solution is not feasible anymore reducing cluster size of type: " + std::to_string(res_type_idx) + " and idx " + std::to_string(res_idx));
      }
    } while(feasible && (n_used_resources_new[res_type_idx][res_idx]>1) );

    Logger::Debug("reduce_cluster_size: Done reducing cluster size!");

    return old_sol;
  }

} //namespace Space4AI
