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
#include "indicators/indicators.hpp"

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

auto main(int argc, char** argv) -> int
{
  assertm(true, "Experiments should be compiled in Release mode to have reliable results!");
  const std::vector<std::string> maps(MAPS);
  const std::vector<int>         agent_nums(NUMS_AGENTS);

  // nondeterministic random generator for generating seeds
  std::random_device                 rd;
  auto                               nondeterministic_rnd_generator = std::mt19937(rd());
  std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());

  // Create a MultiProgress object to handle multiple progress bars
  indicators::ProgressBar bar1{indicators::option::BarWidth{100},
                               indicators::option::Start{"["},
                               indicators::option::Fill{"■"},
                               indicators::option::Lead{"■"},
                               indicators::option::Remainder{" "},
                               indicators::option::End{" ]"},
                               indicators::option::ForegroundColor{indicators::Color::white},
                               indicators::option::ShowElapsedTime{true},
                               indicators::option::MaxProgress{maps.size()},
                               indicators::option::ShowRemainingTime{true},
                               indicators::option::PrefixText{"Maps:\t\t"},
                               indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

  indicators::ProgressBar bar2{indicators::option::BarWidth{100},
                               indicators::option::Start{"["},
                               indicators::option::Fill{"■"},
                               indicators::option::Lead{"■"},
                               indicators::option::Remainder{" "},
                               indicators::option::End{" ]"},
                               indicators::option::ForegroundColor{indicators::Color::white},
                               indicators::option::ShowElapsedTime{true},
                               indicators::option::MaxProgress{agent_nums.size()},
                               indicators::option::ShowRemainingTime{true},
                               indicators::option::PrefixText{"Agent nums:\t"},
                               indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

  indicators::ProgressBar                               bar3{indicators::option::BarWidth{100},
                               indicators::option::Start{"["},
                               indicators::option::Fill{"■"},
                               indicators::option::Lead{"■"},
                               indicators::option::Remainder{" "},
                               indicators::option::End{" ]"},
                               indicators::option::ForegroundColor{indicators::Color::white},
                               indicators::option::ShowElapsedTime{true},
                               indicators::option::MaxProgress{RUNS_PER_MAP},
                               indicators::option::ShowRemainingTime{true},
                               indicators::option::PrefixText{"Scene:\t\t"},
                               indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};
  indicators::MultiProgress<indicators::ProgressBar, 3> bars(bar1, bar2, bar3);


  // iterate over all maps
  for (int i = 0; i < (int)maps.size(); i++)
  {
    // bars.set_progress<0>(((float)i / maps.size() * 100));
    bars.set_option<0>(indicators::option::PostfixText{std::to_string(i) + "/" + std::to_string(maps.size())});
    const std::string map_name = maps[i];
    // iterate over agent numbers
    for (int j = 0; j < (int)agent_nums.size(); j++)
    {
      // bars.set_progress<1>(((float)j / agent_nums.size() * 100));
      bars.set_option<1>(indicators::option::PostfixText{std::to_string(j) + "/" + std::to_string(agent_nums.size())});
      int                            agent_num = agent_nums[j];
      const std::vector<std::string> scenes    = get_scene_names(map_name);
      // make sure every map gets enough runs
      for (int k = 0; k < RUNS_PER_MAP; k++)
      {
        // bars.set_progress<2>(((float)k / RUNS_PER_MAP * 100));
        bars.set_option<2>(indicators::option::PostfixText{std::to_string(k) + "/" + std::to_string(RUNS_PER_MAP)});
        // choose scene
        const std::string& scene_name = scenes[k % scenes.size()];
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
          throw std::runtime_error("Loaded " + std::to_string(instance.num_of_agents) + " agents instead of " + std::to_string(agent_num));
        }

        // find the solution
        start_time           = Time::now();
        Solution sol         = solver.PrioritizedPlanning();
        double   pp_duration = ((dsec)(Time::now() - start_time)).count();
        solver.solution      = sol;

        // if feasible, calculate the cost
        if (solver.solution.feasible)
        {
          solver.solution.calculate_cost(instance);
        }


        // write the results of the experiment
        const std::string experiment_name = "sipp_pp";
        const std::string algorithm_name  = "sipp_mine";
        nlohmann::json    experiment_res  = {{"experiment_name", experiment_name},
                                         {"map_name", map_name},
                                         {"scene_name", scene_name},
                                         {"num_agents", agent_num},
                                         {"algo_name", algorithm_name},
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
        // choose a file name that does not exist yet
        int               num                 = 0;
        const std::string scene_name_stripped = scene_name.substr(map_name.size() + 1, scene_name.find('.') - map_name.size() - 1);
        std::string       filename =
            experiment_name + "_" + map_name + "_" + scene_name_stripped + "_" + algorithm_name + "_" + std::to_string(num) + ".json";
        while (std::filesystem::exists(filename))
        {
          num++;
          filename =
              experiment_name + "_" + map_name + "_" + scene_name_stripped + "_" + algorithm_name + "_" + std::to_string(num) + ".json";
        }

        std::ofstream file(filename);
        file << experiment_res.dump(4);  // Pretty print
        file.close();
        bars.tick<2>();
      }
      bars.set_option<2>(indicators::option::PostfixText{std::to_string(RUNS_PER_MAP) + "/" + std::to_string(RUNS_PER_MAP)});
      bars.tick<1>();
    }
    bars.set_option<1>(indicators::option::PostfixText{std::to_string(agent_nums.size()) + "/" + std::to_string(agent_nums.size())});
    bars.tick<0>();
  }
  bars.set_option<0>(indicators::option::PostfixText{std::to_string(maps.size()) + "/" + std::to_string(maps.size())});
}

