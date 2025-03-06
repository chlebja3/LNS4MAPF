/*
 * Author: Jan Chleboun
 * Date: 04-03-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <nlohmann/json.hpp>
#include <random>

#include "Instance.h"
#include "LNS.h"
#include "experiment_utils.h"
#include "indicators/indicators.hpp"
#include "magic_enum/magic_enum.hpp"
#include "utils.h"

#define RUNS_PER_MAP 1

#define MAPS  \
  {           \
    "den520d" \
  }

#define NUMS_AGENTS \
  {                 \
    500             \
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
  indicators::ProgressBar bar1 = create_progress_bar("Maps:\t\t", maps.size());
  indicators::ProgressBar bar2 = create_progress_bar("Agent nums:\t", agent_nums.size());
  indicators::ProgressBar bar3 = create_progress_bar("Scene:\t\t", RUNS_PER_MAP);
  indicators::ProgressBar bar4 = create_progress_bar("Algo:\t\t", magic_enum::enum_count<SIPP_implementation>());
  indicators::MultiProgress<indicators::ProgressBar, 3> bars(bar1, bar2, bar3);


  // iterate over all maps
  for (int i = 0; i < (int)maps.size(); i++)
  {
    // update the progress bar iteration number display
    bars.set_option<0>(indicators::option::PostfixText{std::to_string(i) + "/" + std::to_string(maps.size())});
    const std::string map_name = maps[i];

    // iterate over agent numbers
    for (int j = 0; j < (int)agent_nums.size(); j++)
    {
      // update the progress bar iteration number display
      bars.set_option<1>(indicators::option::PostfixText{std::to_string(j) + "/" + std::to_string(agent_nums.size())});
      int                            agent_num = agent_nums[j];
      const std::vector<std::string> scenes    = get_scene_names(map_name, RUNS_PER_MAP);

      // load all instances
      auto [instances, preprocessing_times] = load_instances(scenes, map_name, agent_num);

      // make sure every map gets enough runs
      for (int k = 0; k < RUNS_PER_MAP; k++)
      {
        // update the progress bar iteration number display
        bars.set_option<2>(indicators::option::PostfixText{std::to_string(k) + "/" + std::to_string(RUNS_PER_MAP)});

        // choose instance
        assertm(scenes.size() == instances.size(), "The size of scenes and instances must be the same.");
        const int          idx        = k % (int)scenes.size();
        const std::string& scene_name = scenes[idx];
        const Instance&    instance   = instances[idx];
        // check whether enough agents loaded
        if (instance.num_of_agents != agent_num)
        {
          throw std::runtime_error("Loaded " + std::to_string(instance.num_of_agents) + " agents instead of " + std::to_string(agent_num));
        }

        // iterate over all implementations
        for (auto algo : magic_enum::enum_values<SIPP_implementation>())
        {
          // generate a random seed
          int seed = dist(nondeterministic_rnd_generator);

          // create the seeded random generator (for experiment reproducibility)
          auto rnd_generator = std::mt19937(seed);

          // create solver TODO iterate over implementations
          LNS solver(instance, rnd_generator, INFO_type::experiment, 1.0);

          // find the solution
          Clock clock;
          clock.start();
          Solution sol                             = solver.PrioritizedPlanning(SIPP_implementation::SIPP_mine);
          auto [pp_duration_wall, pp_duration_cpu] = clock.end();
          solver.solution                          = sol;

          // if feasible, calculate the cost
          if (solver.solution.feasible)
          {
            solver.solution.calculate_cost(instance);
          }

          // write the results of the experiment
          const std::string experiment_name = "sipp_pp";
          const std::string algorithm_name  = (std::string)magic_enum::enum_name(algo);
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
                                           {"preprocessing_time_wall", preprocessing_times[idx].first},
                                           {"preprocessing_time_cpu", preprocessing_times[idx].second},
                                           {"computation_time_wall", pp_duration_wall},
                                           {"computation_time_cpu", pp_duration_cpu},
                                           {"expanded", solver.get_num_of_expanded_nodes()},
                                           {"generated", solver.get_num_of_generated_nodes()},
                                           {"seed", seed}};

          // save the experiment
          save_experiment(experiment_res, map_name, scene_name, experiment_name, algorithm_name);
        }
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

