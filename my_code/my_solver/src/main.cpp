#include <boost/program_options.hpp>
#include <iostream>

#include "Instance.h"
#include "Visualizer.h"
#include "Computation.h"

// create program options namespace
namespace po = boost::program_options;


int main(int argc, char** argv)
{
    // Define command-line options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show help message")
		    ("map,m", po::value<std::string>()->required(), "file with the map")
		    ("agents,a", po::value<std::string>()->required(), "input file for agents")
		    ("agentNum,k", po::value<int>()->default_value(0), "number of agents");

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
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    std::string map_name = vm["map"].as<std::string>();
    std::string scene_name = vm["agents"].as<std::string>();
    int agent_num = vm["agentNum"].as<int>();

    // create the instance
	  Instance instance(map_name, scene_name, agent_num);

    // print the map
    std::cout << "The map is:" << std::endl;
    // instance.print_map();
    instance.print_agents();

    // create the visualization object
    Visualizer visualizer(instance);
    
    // create the computation object
    Computation computation(instance);

    // start the threads
    visualizer.start();
    computation.start();

    // wait for the threads to finish
    visualizer.join_thread();
    computation.join_thread();
    
    return EXIT_SUCCESS;
}
