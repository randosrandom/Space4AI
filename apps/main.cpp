#include <filesystem>
#include <iostream>


#include "external/GetPot"


#include "src/System/System.hpp"
#include "src/Solution/Solution.hpp"
#include "src/Logger.hpp"
#include "src/Algorithms/RandomGreedyDT.hpp"

namespace sp = Space4AI;
namespace py = pybind11;

int
main(int argc, char** argv)
{
	GetPot command_line(argc, argv);

	const std::string getpot_file = command_line.follow("NOT_FOUND", 2, "-f", "--file");

	if(getpot_file == "NOT_FOUND")
	{
		Logger::Error("Configuration file not provided");
		throw std::invalid_argument("Configuration file not provided");
	}
	else if(!std::filesystem::exists(std::filesystem::path(getpot_file)))
	{
		Logger::Error("Configuration file does not exist");
		throw std::invalid_argument("Configuration file does not exist");
	}

	GetPot ifile(getpot_file.c_str());

	const std::string sec_config = "configuration_file/";
	const std::string sec_algo = "algorithm/";
	const std::string sec_logg = "logger/";

	const size_t log_priority = ifile((sec_logg + "priority").data(), 3);
	Logger::SetPriority(static_cast<LogPriority>(log_priority));

	const bool enable_terminal_ouput = ifile((sec_logg + "terminal_stream").data(), true);
	Logger::EnableTerminalOutput(enable_terminal_ouput);

	const bool enable_file_output = ifile((sec_logg + "file_stream").data(), false);
	const std::string logger_file_output_name = ifile((sec_logg + "file_name").data(), "");

	Logger::EnableFileOutput(enable_file_output, logger_file_output_name);


	const std::string system_file_name = ifile((sec_config + "file_name").data(), "NOT_FOUND");

	const size_t n_iterations = ifile((sec_algo + "n_iterations").data(), 1000);
	const size_t n_max_solutions = ifile((sec_algo + "n_max_solutions").data(), 3);
	const std::string output_file_name_prefix = ifile((sec_algo + "output_file_name_prefix").data(), "../OutputFiles/SolRandomGreedyDT");

	// -------------------------------------------------------------------------------

	sp::System system;

	// py::scoped_interpreter guard{};
	//	py::gil_scoped_release release; // add this to release the GIL

	//--------------------------------------------------------------------------------

	system.read_configuration_file(system_file_name);

	auto elite = std::move(sp::RandomGreedyDT::random_greedy(system, n_iterations, n_max_solutions));

	for(size_t i = 0; i < elite.get_size(); ++i)
	{
		elite.print_solution(
			system,
			output_file_name_prefix + "_" + std::to_string(i) + ".json",
			i
		);
	}


/*
	for (size_t i = 0; i < 100; ++i)
	{
		auto sol = sp::RandomGreedyDT::step(system);
		sol.check_feasibility(system);
		sol.objective_function(system);
		sol.print_solution(system, "../OutputFiles/DebugSol" + std::to_string(i) + ".json");
	}
*/

	/*
	const std::string file_run = "../ConfigFiles/Lambda_0.15_output_json.json";

	sp::Solution sol;
	sol.read_solution_from_file(file_run, system);

	sol.check_feasibility(system);

	sol.objective_function(system);

	sol.print_solution(system);
	*/

	// for(auto c = 0; c < performance.size(); ++c)
	// {
	// 	std::cout << "\ncomp " << c << std::endl;
	//
	// 	for(auto t=0; t<performance[c].size(); ++t)
	// 	{
	// 		for(auto p=0; p < performance[c][t].size(); ++p )
	// 		{
	// 			for(auto r=0; r < performance[c][t][p].size(); ++r)
	// 			{
	//
	// 				if(system.get_system_data()get_system_data().get_compatibility_matrix()[c][t][p][r])
	// 					std::cout << "\t" << performance[c][t][p][r]->get_keyword();
	// 			}
	// 		}
	// 	}
	// }

	// const auto& comp2 = system.get_system_data()get_components()[2];
	//
	// for(const auto& d : comp2.get_deployments())
	// {
	// 	std::cout << std::endl;
	// 	std::cout << d.get_name() << std::endl;
	// 	const auto& part_idxs = d.get_partition_indices();
	// 	for(auto i: part_idxs)
	// 		std::cout << i << "\t";
	// }
	// std::cout<<"\n";
	//
	// std::cout << "*************"<<std::endl;
	//
	// std::cout << system.get_system_data().get_dag().size() << std::endl;
	//
	// const auto& graph_matrix=system.get_system_data().get_dag().get_dag_matrix();
	//
	// for(std::size_t i = 0; i < system.get_system_data().get_dag().size(); ++i)
	// {
	//   for(std::size_t j = 0; j < system.get_system_data().get_dag().size(); ++j)
	//   {
	//     std::cout << graph_matrix[i][j] << "\t";
	//   }
	//
	//   std::cout << std::endl;
	// }
	// //
	// // std::cout << std::endl;
	// //
	// // std::cout << system.get_system_data()get_compatibility_matrix()[0][2][0][0] << std::endl;
	// // std::cout << system.get_system_data()get_compatibility_matrix()[0][0][0][0] << std::endl;
	// // std::cout << "**********" << std::endl;
	// //
	// //
	// // for(auto c: system.get_system_data().get_components())
	// // {
	// //   auto partitions=c.get_partitions();
	// //   std::cout << partitions.size() << std::endl;
	// // }
	// //
	// // std::cout << std::endl;
	// // std::cout << "**********" << std::endl;
	// //
	// const auto& resources = system.get_system_data().get_resources();
	// const auto& n_VM = resources.get_number_resources(1);
	//
	// for(std::size_t i=0; i<n_VM; ++i)
	// {
	// 	std::cout << resources.get_number_avail(sp::resource_type(1),i) <<std::endl;
	// }
	// // for(const auto& c: resources.get_resources<sp::ResourceType::Faas>())
	// //   std::cout << c.get_description() << std::endl;
	//
	// std::cout << "**********" << std::endl;
	// std::cout<<"cl of edge resources: "<<std::endl;
	//
	// const auto& n_edge = resources.get_number_resources(0);
	// for(std::size_t i=0; i<n_edge; ++i)
	// {
	// 	std::cout << resources.get_cl_name(sp::resource_type(0),i) <<std::endl;
	// }
	//
	// std::cout << "**********" << std::endl;
	// std::cout<<"cl of VM resources: "<<std::endl;
	// for(std::size_t i=0; i<n_VM; ++i)
	// {
	// 	std::cout << resources.get_cl_name(sp::resource_type(1),i) <<std::endl;
	// }
	//
	// std::cout << "**********" << std::endl;
	// std::cout<<"cl of Faas resources: "<<std::endl;
	// const auto& n_Faas = resources.get_number_resources(2);
	//
	// for(std::size_t i=0; i<n_Faas; ++i)
	// {
	// 	std::cout << resources.get_cl_name(sp::resource_type(2),i) <<std::endl;
	// }
	//
	// //RESOURCE DATA HAS BEEN CHECKED AND IT IS CORRECT!
	//
	// // check components
	//
	// const auto& components = system.get_system_data().get_components();
	// std::cout << "**********" << std::endl;
	// //check partitions
	// for (std::size_t i=0; i<components.size(); ++i)
	// {
	// 	std::cout << "COMPONENT "<< components[i].get_name() << std::endl;
	// 	for (const auto& part : components[i].get_partitions())
	// 	{
	// 		std::cout<<"***"<<std::endl;
	// 		std::cout << "part name: "<<part.get_name()<<std::endl;
	// 		std::cout << "part memory: "<<part.get_memory()<<std::endl;
	// 		std::cout << "part early exit prob: "<<part.get_early_exit_probability()<<std::endl;
	// 		std::cout << "part next: "<<part.get_next()<<std::endl;
	// 		std::cout << "part data size: "<<part.get_data_size()<<std::endl;
	// 	}
	// }

	// std::cout << "**********************************************\n" << std::endl;
	//
	// auto idx_to_comp_name = system.get_system_data().get_idx_to_comp_name();
	//
	// for(auto [i, n]: idx_to_comp_name)
	// {
	// 	std::cout << "Name n: " << n << "\t idx: " << i << std::endl;
	// }
	//
	//
	// //COMPONENTS AND PARTITIONS HAVE BEEN CHECKED AND THEY ARE CORRECT
	//
	// //check comp_name_to_idx
	// std::cout << "**********" << std::endl;
	// const auto& comp_name_to_idx = system.get_system_data().get_comp_name_to_idx();
	//
	// for(const auto& elem : comp_name_to_idx)
	// {
	// 	std::cout<<"comp name: "<<elem.first<<"  comp idx: "<<elem.second<<std::endl;
	// }
	//
	// //correct!
	//
	// //check part_name_to_part_idx
	// std::cout << "**********" << std::endl;
	// const auto& part_name_to_part_idx = system.get_system_data().get_part_name_to_part_idx();
	//
	// for(const auto& elem : part_name_to_part_idx)
	// {
	// 	std::cout<<"part name: "<<elem.first<<"  part idx: "<<elem.second<<std::endl;
	// }
	//
	// // correct!
	// //
	// // check cls
	// std::cout << "**********" << std::endl;
	// const auto& cls = system.get_system_data().get_cls();
	//
	// for(std::size_t res_type_idx=0; res_type_idx<3; ++res_type_idx)
	// {
	// 	std::cout << "*** cls of type: "<< res_type_idx <<" ***"<<std::endl;
	// 	for(const auto& cl : cls[res_type_idx])
	// 	{
	// 		std::cout << "cl name: "<<cl.get_name()<<std::endl;
	// 		//get resource idxs
	// 		const auto& res_idxs = cl.get_res_idxs();
	// 		std::cout << "resources: "<<std::endl;
	// 		for(std::size_t i=0; i<res_idxs.size(); ++i)
	// 		{
	// 			std::cout << res_idxs[i] <<std::endl;
	// 		}
	// 	}
	// }

	// correct!
	//
	// // check res_name_to_type_and_idx
	// std::cout << "**********" << std::endl;
	// const auto& res_name_to_type_and_idx = system.get_system_data().get_res_name_to_type_and_idx();
	//
	// for(const auto& elem : res_name_to_type_and_idx)
	// {
	// 	std::cout << "*** Resource name: " << elem.first <<" ***" << std::endl;
	// 	std::cout << "res type: " << index(elem.second.first)<<std::endl;
	// 	std::cout << "res idx: "<<elem.second.second<<std::endl;
	// }
	// //correct!
	//
	//
	// //check network domains
	// std::cout << "**********" << std::endl;
	// const auto& network_domains = system.get_system_data().get_network_domains();
	//
	// for(std::size_t i=0; i<network_domains.size(); ++i)
	// {
	// 	std::cout << "*** network domain name: "<<network_domains[i].get_name()<<" ***"<<std::endl;
	// 	const auto& cls = network_domains[i].get_cls();
	// 	std::cout << "cls:"<<std::endl;
	// 	for(std::size_t j=0; j<cls.size(); ++j)
	// 	{
	// 		std::cout << cls[j] <<std::endl;
	// 	}
	// 	std::cout<<"access delay: "<<network_domains[i].get_access_delay()<<std::endl;
	// 	std::cout<<"Bandwidth: "<<network_domains[i].get_bandwidth()<<std::endl;
	// }
	//
	// //correct
	//
	// //check cl_to_network_domains
	// std::cout << "**********" << std::endl;
	// const auto& cl_to_network_domains = system.get_system_data().get_cl_to_network_domains();
	//
	// for(const auto& elem : cl_to_network_domains)
	// {
	// 	std::cout << "*** cl name: "<<elem.first<<" ***"<<std::endl;
	// 	for(const auto& i : elem.second)
	// 	{
	// 		std::cout << i <<std::endl;
	// 	}
	// }
	// //correct!
	//
	// //check local constraints
	// std::cout << "**********" << std::endl;
	// const auto& local_constraints = system.get_system_data().get_local_constraints();
	//
	// for(std::size_t i=0; i< local_constraints.size(); ++i)
	// {
	// 	std::cout << "*** comp idx: "<<local_constraints[i].get_comp_idx()<<" ***"<<std::endl;
	// 	std::cout <<"max res time: "<<local_constraints[i].get_max_res_time()<<std::endl;
	// }
	//
	// //correct
	//
	// //check global constraints
	// std::cout << "**********" << std::endl;
	// const auto& global_constraints = system.get_system_data().get_global_constraints();
	//
	// for(std::size_t i=0; i< global_constraints.size(); ++i)
	// {
	// 	std::cout << "path name: "<<global_constraints[i].get_path_name()<<" ***"<<std::endl;
	// 	const auto& comp_idxs = global_constraints[i].get_comp_idxs();
	// 	std::cout << "comp idxs: "<<std::endl;
	// 	for(std::size_t j=0; j<comp_idxs.size(); ++j)
	// 	{
	// 		std::cout << comp_idxs[j] <<std::endl;
	// 	}
	// 	std::cout <<"max res time: "<<global_constraints[i].get_max_res_time()<<std::endl;
	// }
	// //correct
	//
	// //check gc_name_to_idx
	// std::cout << "**********" << std::endl;
	// const auto& gc_name_to_idx = system.get_system_data().get_gc_name_to_idx();
	//
	// for(const auto& elem : gc_name_to_idx)
	// {
	// 	std::cout<<"gc name: "<<elem.first<<"   gc idx: "<<elem.second<<std::endl;
	// }
	// //correct

	return 0;
}
