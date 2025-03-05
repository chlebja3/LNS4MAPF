/*
 * Author: Jan Chleboun
 * Date: 04-03-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <random>

#include "Instance.h"
#include "LNS.h"

#define RUNS_PER_MAP 1

#define MAPS  \
  {           \
    "den520d" \
  }

#define NUMS_AGENTS \
  {                 \
    500             \
  }

auto get_mapf_benchmark_path() -> std::string
{
  return std::filesystem::canonical("/proc/self/exe").parent_path().parent_path().parent_path().parent_path().string() +
         "/MAPF-benchmark";  // Linux only
}

auto get_scene_names(const std::string& map_name) -> std::vector<std::string>
{
  std::vector<std::string> scen_files;  // will be returned

  // find path to the scene directory
  std::string scen_dir_path = get_mapf_benchmark_path() + "/mapf-scen-random/scen-random/";

  // iterate over all files in the scene directory
  for (const auto& entry : std::filesystem::directory_iterator(scen_dir_path))
  {
    std::string filename = entry.path().filename().string();
    // Check if the filename starts with map name
    if (filename.rfind(map_name, 0) == 0)
    {
      // add to the scene files
      scen_files.push_back(filename);
    }
  }
  return scen_files;
}

auto get_max_agent_num(const std::string& scene_name) -> int
{
  // convert scene name to path
  const std::string scene_path = get_mapf_benchmark_path() + "/mapf-scen-random/scen-random/" + scene_name;

  // open the scene
  std::ifstream scene_file(scene_path);
  if (!scene_file.is_open())
  {
    throw std::runtime_error("Cannot open file '" + scene_path + "'");
  }

  // find the number of lines and subtract 1 to get the number of agents
  return (int)std::count(std::istreambuf_iterator<char>(scene_file), std::istreambuf_iterator<char>(), '\n') - 1;
}


auto main(int argc, char** argv) -> int
{
  assertm(true, "Experiments should be compiled in Release mode to have reliable results!");
  const std::vector<std::string> maps(MAPS);
  const std::vector<int>         agent_nums(NUMS_AGENTS);

  // nondeterministic random generator for generating seeds
  std::random_device                 rd;
  auto                               nondeterministic_rnd_generator = std::mt19937(rd());
  std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());

  // iterate over all maps
  for (const auto& map_name : maps)
  {
    // iterate over agent numbers
    for (int agent_num : agent_nums)
    {
      int                            count  = 0;
      const std::vector<std::string> scenes = get_scene_names(map_name);
      // make sure every map gets enough runs
      while (count < RUNS_PER_MAP)
      {
        // iterate over scenes
        for (const auto& scene_name : scenes)
        {
          // generate a random seed
          int seed = dist(nondeterministic_rnd_generator);

          // create the seeded random generator (for experiment reproducibility)
          auto rnd_generator = std::mt19937(seed);

          Time::time_point start_time = Time::now();
          // create the instance
          const std::string map_path   = get_mapf_benchmark_path() + "/mapf-map/" + map_name + ".map";
          const std::string scene_path = get_mapf_benchmark_path() + "/mapf-scen-random/scen-random/" + scene_name;
          Instance          instance(map_path, scene_path, agent_num);

          // create solver
          LNS    solver(instance, rnd_generator, INFO_type::experiment, 1.0);
          double preprocessing_time = ((dsec)(Time::now() - start_time)).count();

          // check whether enough agents loaded
          if (instance.num_of_agents != agent_num)
          {
            throw std::runtime_error("Loaded " + std::to_string(instance.num_of_agents) + " agents instead of " +
                                     std::to_string(agent_num));
          }

          // find the solution
          start_time           = Time::now();
          Solution sol         = solver.PrioritizedPlanning();
          double   pp_duration = ((dsec)(Time::now() - start_time)).count();
          solver.solution      = sol;

          std::cout << "Solved the problem." << std::endl;
          // if feasible, calculate the cost
          if (solver.solution.feasible)
          {
            solver.solution.calculate_cost(instance);
          }

          std::cout << "Calculated cost." << std::endl;

          // write the results of the experiment
          nlohmann::json experiment_res = {{"experiment_name", "sipp_pp"},
                                           {"map_name", map_name},
                                           {"scene_name", scene_name},
                                           {"num_agents", agent_num},
                                           {"algo_name", "sipp_mine"},
                                           {"algo_parameters", {}},
                                           {"feasible", solver.solution.feasible},
                                           {"sum_of_dist", instance.get_sum_of_distances()},
                                           {"sum_of_cost", solver.solution.sum_of_costs},
                                           {"makespan", solver.solution.makespan},
                                           {"preprocessing_time", preprocessing_time},
                                           {"computation_time", pp_duration},
                                           {"expanded", solver.get_num_of_expanded_nodes()},
                                           {"generated", solver.get_num_of_generated_nodes()},
                                           {"seed", seed}};
          std::cout << "Writing to file. " << std::endl;
          std::ofstream file("experiment.json");
          file << experiment_res.dump(4);  // Pretty print
          file.close();
          count++;
          if (count >= RUNS_PER_MAP)
          {
            break;
          }
        }
      }
    }
  }
}

