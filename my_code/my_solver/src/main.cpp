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
#include "SharedData.h"
#include "Visualizer.h"

// create program options namespace
namespace po = boost::program_options;


int main(int argc, char** argv)
{
  // Define command-line options
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "Show help message")("map,m", po::value<std::string>()->required(), "file with the map")(
      "agents,a", po::value<std::string>()->required(), "input file for agents")(
      "agentNum,k", po::value<int>()->default_value(0), "number of agents")("GUI,G", po::value<bool>()->default_value(1));

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
    if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return 0;
    }
    // Handle missing arguments
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }

  std::string map_name   = vm["map"].as<std::string>();
  std::string scene_name = vm["agents"].as<std::string>();
  int         agent_num  = vm["agentNum"].as<int>();
  bool        GUI        = vm["GUI"].as<bool>();

  // create the instance
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(map_name, scene_name, agent_num);

  // calculate the optimal paths neglecting the other agents
  // instance->calculate_optimal_paths_parallel(); // old, dont use
  instance->calculate_optimal_paths();

  // create the shared datastructure
  std::unique_ptr<SharedData> shared_data = std::make_unique<SharedData>();

  // create the computation object
  Computation computation(*instance, *shared_data, GUI);

  // start the computation thread
  computation.start();

  // create the visualization object
  if (GUI)
  {
    Visualizer visualizer(*instance, *shared_data);
    visualizer.start();
    // wait for the visualization thread to finish
    visualizer.join_thread();
  }

  // wait for the computation thread to finish
  computation.join_thread();

  return EXIT_SUCCESS;
}
