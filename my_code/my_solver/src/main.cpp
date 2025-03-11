/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <boost/program_options.hpp>
#include <iostream>
#include <memory>

#include "Computation.h"
#include "Instance.h"
#include "LNS.h"
#include "SIPP.h"
#include "SharedData.h"
#include "Visualizer.h"
#include "magic_enum/magic_enum.hpp"

// create program options namespace
namespace po = boost::program_options;

constexpr int    DEFAULT_MAX_ITER   = 100;
constexpr double DEFAULT_TIME_LIMIT = 30.0;


auto main(int argc, char** argv) -> int
{
  // Define command-line options
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "Show help message")("map,m", po::value<std::string>()->required(), "file with the map")(
      "agents,a", po::value<std::string>()->required(), "input file for agents")(
      "agentNum,k", po::value<int>()->default_value(0), "number of agents")("GUI,G", po::value<bool>()->default_value(true),
                                                                            "whether to tun the graphical user interface")(
      "sipp_suboptimality,w", po::value<double>()->default_value(1.0), "suboptimality factor for suboptimal sipp")(
      "maxIterations,i", po::value<int>()->default_value(DEFAULT_MAX_ITER), "maximal number of iterations of LNS")(
      "timeLimit,t", po::value<double>()->default_value(DEFAULT_TIME_LIMIT), "time limit to find the solution, in seconds")(
      "sipp_implementation,si", po::value<std::string>()->default_value("SIPP_mine"),
      "implementation of SIPP (SIPP_mine, SIPP_mapf_lns, SIPP_suboptimal)")(
      "seed,s", po::value<int>()->default_value(-1),
      "seed of the random generators for reproducability, to achieve non reproducible random behavior, use negative value");

  // Parse command-line arguments
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch (const po::error& ex)
  {
    // Handle --help
    if (vm.count("help") != 0U)
    {
      std::cout << desc << std::endl;
      return 0;
    }
    // Handle missing arguments
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }

  const std::string map_name   = vm["map"].as<std::string>();
  const std::string scene_name = vm["agents"].as<std::string>();
  const std::string sipp_algo  = vm["sipp_implementation"].as<std::string>();
  const int         agent_num  = vm["agentNum"].as<int>();
  const int         max_iter   = vm["maxIterations"].as<int>();
  const double      time_limit = vm["timeLimit"].as<double>();
  const bool        GUI        = vm["GUI"].as<bool>();
  const double      w          = vm["sipp_suboptimality"].as<double>();
  const int         seed       = vm["seed"].as<int>();

  // create the instance
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(map_name, scene_name, agent_num);

  // calculate the optimal paths neglecting the other agents
  // instance->calculate_optimal_paths_parallel(); // old, dont use
  // instance->calculate_optimal_paths();

  // create the shared datastructure
  std::unique_ptr<SharedData> shared_data = std::make_unique<SharedData>();

  // create the info type
  INFO_type info_type = no_info;
  if (GUI)
  {
    info_type = INFO_type::visualisation;
  }

  SIPP_implementation sipp_implementation     = SIPP_implementation::SIPP_mine;
  auto                sipp_implementation_opt = magic_enum::enum_cast<SIPP_implementation>(sipp_algo, magic_enum::case_insensitive);
  if (sipp_implementation_opt.has_value())
  {
    sipp_implementation = sipp_implementation_opt.value();
  }
  else
  {
    std::cout << "WARNING: Unknown sipp implementation: '" << sipp_algo << "', using default option "
              << magic_enum::enum_name(sipp_implementation) << std::endl;
  }

  // create SIPP settings
  SIPP_settings sipp_settings = SIPP_settings(sipp_implementation, info_type, w);

  // create LNS settings
  LNS_settings lns_settings = LNS_settings(max_iter, time_limit, sipp_settings);


  // create the computation object
  Computation computation(*instance, shared_data.get(), lns_settings, seed);

  // start the computation thread
  computation.start();

  // create the visualization object
  if (GUI)
  {
    Visualizer visualizer(*instance, *shared_data, seed + 1);  // use different seed for visualizer than for computation
    visualizer.start();
    // wait for the visualization thread to finish
    visualizer.join_thread();
  }

  // wait for the computation thread to finish
  computation.join_thread();

  return EXIT_SUCCESS;
}
