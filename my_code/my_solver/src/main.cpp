/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <boost/program_options.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

#include "Computation.h"
#include "Instance.h"
#include "LNS.h"
#include "SIPP.h"
#include "SharedData.h"
#include "Visualizer.h"
#include "magic_enum/magic_enum.hpp"

// create program options namespace
namespace po = boost::program_options;

constexpr int DEFAULT_MAX_ITER = 10;
constexpr double DEFAULT_TIME_LIMIT = 30.0;
constexpr int DEFAULT_NEIGHBORHOOD_SIZE = 10;
constexpr int DEFAULT_MAX_PUSHES = 1;

auto main(int argc, char** argv) -> int
{
  // Define command-line options
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "Show help message")("map,m", po::value<std::string>()->required(), "file with the map")(
      "agents,a", po::value<std::string>()->required(), "input file for agents")(
      "agentNum,k", po::value<int>()->default_value(0), "number of agents")(
      "GUI,G", po::value<bool>()->default_value(true), "whether to turn on the graphical user interface")(
      "sipp_suboptimality,w", po::value<double>()->default_value(1.0), "suboptimality factor for suboptimal sipp")(
      "maxIterations,i", po::value<int>()->default_value(DEFAULT_MAX_ITER), "maximal number of iterations of LNS")(
      "timeLimit,t", po::value<double>()->default_value(DEFAULT_TIME_LIMIT),
      "time limit to find the solution, in seconds")(
      "sipp_implementation", po::value<std::string>()->default_value("SIPP_mine"),
      "implementation of SIPP (SIPP_mine, SIPP_mapf_lns, SIPP_suboptimal)")(
      "psipp_implementation", po::value<std::string>()->default_value("PSIPP_mine"),
      "implementation of PSIPP (PSIPP_mine, PSIPP_mine_ap, PSIPP_suboptimal, PSIPP_suboptimal_ap)")(
      "use_psipp", po::value<bool>()->default_value(true), "Use PSIPP instead of SIPP")(
      "maxPushes", po::value<int>()->default_value(DEFAULT_MAX_PUSHES), "maximal number of pushes in PSIPP")(
      "pushCost", po::value<double>()->default_value(std::numeric_limits<double>::infinity(), "inf"),
      "cost of a single push in PSIPP node ordering (inf = pushes are ordered lexicographically)")(
      "closedListPerPushCount", po::value<bool>()->default_value(false),
      "keep a separate closed list for each push count in PSIPP")(
      "sippFirst", po::value<bool>()->default_value(true), "try SIPP first and use PSIPP only if SIPP fails")

      ("Restarts,r", po::value<bool>()->default_value(true),
       "restart the search if no feasible initial solution was found")(
          "destroy_operator", po::value<std::string>()->default_value("ADAPTIVE"),
          "Destroy operator to be used in LNS (RANDOM, RANDOMWALK, INTERSECTION, ADAPTIVE, RANDOM_CHOOSE, BLOCKED)")(
          "neighborhood_size,n", po::value<int>()->default_value(DEFAULT_NEIGHBORHOOD_SIZE),
          "Size of the neighborhood used by the destroy operator (number of paths to be destroyed)")(
          "seed,s", po::value<int>()->default_value(-1),
          "seed of the random generators for reproducability, to achieve non reproducible random behavior, use "
          "negative "
          "value")(
          "output_paths", po::value<std::string>()->default_value(""),
          "Output file for the paths of the generated solution. If not used, the solution will not be exported.");

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

  const std::string map_name = vm["map"].as<std::string>();
  const std::string scene_name = vm["agents"].as<std::string>();
  const std::string sipp_algo = vm["sipp_implementation"].as<std::string>();
  const std::string psipp_algo = vm["psipp_implementation"].as<std::string>();
  const std::string destroy_name = vm["destroy_operator"].as<std::string>();
  const std::string output_paths_file = vm["output_paths"].as<std::string>();
  const int neighborhood_size = vm["neighborhood_size"].as<int>();
  const int agent_num = vm["agentNum"].as<int>();
  const int max_iter = vm["maxIterations"].as<int>();
  const double time_limit = vm["timeLimit"].as<double>();
  const bool GUI = vm["GUI"].as<bool>();
  const bool restarts = vm["Restarts"].as<bool>();
  const double w = vm["sipp_suboptimality"].as<double>();
  const int seed = vm["seed"].as<int>();
  const bool use_psipp = vm["use_psipp"].as<bool>();
  const int max_pushes = vm["maxPushes"].as<int>();
  const double push_cost = vm["pushCost"].as<double>();
  const bool closed_list_per_push_count = vm["closedListPerPushCount"].as<bool>();
  const bool sipp_first = vm["sippFirst"].as<bool>();

  if (push_cost < 0)
  {
    throw std::invalid_argument("pushCost must be non-negative");
  }
  if (max_pushes < 0)
  {
    throw std::invalid_argument("maxPushes must be non-negative");
  }

  // create the instance
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(map_name, scene_name, agent_num);

  // calculate the optimal paths neglecting the other agents
  // instance->calculate_optimal_paths_parallel(); // old, dont use
  // instance->calculate_optimal_paths();

  // create the shared datastructure
  std::unique_ptr<SharedData> shared_data = std::make_unique<SharedData>();

  // create the info type
  INFO_type info_type = INFO_type::no_info;
  if (GUI)
  {
    info_type = INFO_type::visualisation;
  }

  // read sipp implementation
  SIPP_implementation sipp_implementation = SIPP_implementation::SIPP_mine;
  auto sipp_implementation_opt = magic_enum::enum_cast<SIPP_implementation>(sipp_algo, magic_enum::case_insensitive);
  if (sipp_implementation_opt.has_value())
  {
    sipp_implementation = sipp_implementation_opt.value();
  }
  else
  {
    std::cout << "WARNING: Unknown sipp implementation: '" << sipp_algo << "', using default option "
              << magic_enum::enum_name(sipp_implementation) << std::endl;
  }

  // read psipp implementation
  PSIPP_implementation psipp_implementation = PSIPP_implementation::PSIPP_mine;
  auto psipp_implementation_opt = magic_enum::enum_cast<PSIPP_implementation>(psipp_algo, magic_enum::case_insensitive);
  if (psipp_implementation_opt.has_value())
  {
    psipp_implementation = psipp_implementation_opt.value();
  }
  else
  {
    std::cout << "WARNING: Unknown psipp implementation: '" << psipp_algo << "', using default option "
              << magic_enum::enum_name(psipp_implementation) << std::endl;
  }

  // read destroy operator
  DESTROY_TYPE destroy_type = DESTROY_TYPE::RANDOM;
  auto destroy_type_opt = magic_enum::enum_cast<DESTROY_TYPE>(destroy_name, magic_enum::case_insensitive);
  if (destroy_type_opt.has_value())
  {
    destroy_type = destroy_type_opt.value();
  }
  else
  {
    std::cout << "WARNING: Unknown destroy type: '" << destroy_name << "', using default option "
              << magic_enum::enum_name(destroy_type) << std::endl;
  }

  if (neighborhood_size < 1 || neighborhood_size > agent_num)
  {
    throw std::runtime_error("Invalid neighborhood size");
  }

  // warn about options that have no effect
  const bool is_mine_ap = psipp_implementation == PSIPP_implementation::PSIPP_mine_ap;
  const bool is_mine = psipp_implementation == PSIPP_implementation::PSIPP_mine || is_mine_ap;
  if (closed_list_per_push_count && !is_mine_ap)
  {
    std::cerr << "WARNING: --closedListPerPushCount only has an effect with PSIPP_mine_ap" << std::endl;
  }
  if (std::isfinite(push_cost) && !is_mine)
  {
    std::cerr << "WARNING: --pushCost only has an effect with PSIPP_mine and PSIPP_mine_ap" << std::endl;
  }
  if (!sipp_first && !use_psipp)
  {
    std::cerr << "WARNING: --sippFirst=false has no effect without PSIPP (--use_psipp)" << std::endl;
  }

  // create SIPP settings
  SIPP_settings sipp_settings = SIPP_settings(sipp_implementation, info_type, w);

  // create PSIPP settings
  PSIPP_settings psipp_settings =
      PSIPP_settings(psipp_implementation, info_type, w, 1, max_pushes, push_cost, closed_list_per_push_count);

  // create destroy settings
  Destroy_settings destroy_settings = Destroy_settings(destroy_type, neighborhood_size);

  // create LNS settings
  LNS_settings lns_settings = LNS_settings(max_iter, time_limit, destroy_settings, sipp_settings, psipp_settings,
                                           restarts, use_psipp, sipp_first);

  // create the computation object
  Computation computation(*instance, shared_data.get(), lns_settings, seed);

  // start the computation thread
  computation.start();

  // create the visualization object
  if (GUI)
  {
    Visualizer visualizer(*instance, computation, *shared_data,
                          seed + 1);  // use different seed for visualizer than for computation
    visualizer.start();
    // wait for the visualization thread to finish
    visualizer.join_thread();
  }

  // wait for the computation thread to finish
  computation.join_thread();

  // export the solution
  if (!output_paths_file.empty())
  {
    const Solution& sol = computation.get_solution();
    sol.save(output_paths_file, *instance);
  }

  return EXIT_SUCCESS;
}
