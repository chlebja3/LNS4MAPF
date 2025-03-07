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
#define EXPERIMENT_NAME "sipp_pp"

#define MAPS  \
  {           \
    "den520d" \
  }

#define NUMS_AGENTS \
  {                 \
    500             \
  }

#define SUBOPTIMALITIES       \
  {                           \
    1.0, 1.01, 1.05, 1.1, 1.5 \
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

  // precalculate what can be skipped
  std::vector<bool> skips                   = what_can_be_skipped(EXPERIMENT_NAME, maps, agent_nums, RUNS_PER_MAP);
  int               skipped_maps            = 0;
  int               skipped_agent_nums      = 0;
  int               skipped_runs            = 0;
  int               skipped_implementations = 0;

  // prepare the algorithms
  std::vector<Algorithm> algos;
  for (const auto& algo_type : magic_enum::enum_values<SIPP_implementation>())
  {
    if (algo_type == SIPP_implementation::SIPP_suboptimal)
    {
      for (double w : SUBOPTIMALITIES)
      {
        algos.emplace_back(algo_type, std::make_pair("w", w));
      }
    }
    else
    {
      algos.emplace_back(algo_type);
    }
  }
  // print all algorithm
  for (const auto& algo : algos)
  {
    std::cout << magic_enum::enum_name(std::get<SIPP_implementation>(algo.type)) << std::endl;
  }

  // iterate over all maps
  for (int i = 0; i < (int)maps.size(); i++)
  {
    // update the progress bar iteration number display
    bars.set_option<0>(indicators::option::PostfixText{std::to_string(i) + "/" + std::to_string(maps.size())});

    // skip maps that weere already solved
    const int start_offset_map = i * (int)agent_nums.size() * RUNS_PER_MAP * magic_enum::enum_count<SIPP_implementation>();
    const int end_offset_map   = (i + 1) * agent_nums.size() * RUNS_PER_MAP * magic_enum::enum_count<SIPP_implementation>();
    if (std::all_of(skips.begin() + start_offset_map, skips.begin() + end_offset_map, [](bool b) { return b; }))
    {
      bars.tick<0>();
      bars.set_progress<1>(agent_nums.size());
      bars.set_progress<2>((size_t)RUNS_PER_MAP);
      skipped_maps++;
      skipped_agent_nums += (int)agent_nums.size();
      skipped_runs += (int)agent_nums.size() * RUNS_PER_MAP;
      skipped_implementations += (int)agent_nums.size() * RUNS_PER_MAP * magic_enum::enum_count<SIPP_implementation>();
      continue;
    }

    const std::string map_name = maps[i];

    const std::vector<std::string> scenes = get_scene_names(map_name, RUNS_PER_MAP);

    // iterate over agent numbers
    for (int j = 0; j < (int)agent_nums.size(); j++)
    {
      // update the progress bar iteration number display
      bars.set_option<1>(indicators::option::PostfixText{std::to_string(j) + "/" + std::to_string(agent_nums.size())});

      // skip agent_nums that were already solved
      const int start_offset_an = start_offset_map + j * RUNS_PER_MAP * magic_enum::enum_count<SIPP_implementation>();
      const int end_offset_an   = start_offset_map + (j + 1) * RUNS_PER_MAP * magic_enum::enum_count<SIPP_implementation>();
      if (std::all_of(skips.begin() + start_offset_an, skips.begin() + end_offset_an, [](bool b) { return b; }))
      {
        bars.tick<1>();
        bars.set_progress<2>((size_t)RUNS_PER_MAP);
        skipped_agent_nums++;
        skipped_runs += RUNS_PER_MAP;
        skipped_implementations += RUNS_PER_MAP * magic_enum::enum_count<SIPP_implementation>();
        continue;
      }

      int agent_num = agent_nums[j];

      // load all instances
      auto [instances, preprocessing_times] = load_instances(scenes, map_name, agent_num);

      // make sure every map gets enough runs
      for (int k = 0; k < RUNS_PER_MAP; k++)
      {
        // update the progress bar iteration number display
        bars.set_option<2>(indicators::option::PostfixText{std::to_string(k) + "/" + std::to_string(RUNS_PER_MAP)});

        // skip runs that were already solved
        const int start_offset_run = start_offset_an + k * magic_enum::enum_count<SIPP_implementation>();
        const int end_offset_run   = start_offset_an + (k + 1) * magic_enum::enum_count<SIPP_implementation>();
        if (std::all_of(skips.begin() + start_offset_run, skips.begin() + end_offset_run, [](bool b) { return b; }))
        {
          bars.tick<2>();
          skipped_runs++;
          skipped_implementations += magic_enum::enum_count<SIPP_implementation>();
          continue;
        }

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
        for (int l = 0; l < (int)magic_enum::enum_count<SIPP_implementation>(); l++)
        {

          // skip already solved
          if (skips[start_offset_run + l])
          {
            skipped_implementations++;
            continue;
          }

          const SIPP_implementation algo = magic_enum::enum_value<SIPP_implementation>(l);

          // generate a random seed
          int seed = dist(nondeterministic_rnd_generator);

          // create the seeded random generator (for experiment reproducibility)
          auto rnd_generator = std::mt19937(seed);

          // create solver
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
          const std::string algorithm_name = (std::string)magic_enum::enum_name(algo);
          nlohmann::json    experiment_res = {{"experiment_name", EXPERIMENT_NAME},
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
          save_experiment(experiment_res, map_name, scene_name, EXPERIMENT_NAME, algorithm_name);
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
  // mark all progress bars as finished
  bars.set_progress<0>(maps.size());
  bars.set_progress<1>(agent_nums.size());
  bars.set_progress<2>((size_t)RUNS_PER_MAP);

  std::cout << "Experiment finished. Skipped: " << skipped_maps << "/" << maps.size() << " maps, " << skipped_agent_nums << "/"
            << maps.size() * agent_nums.size() << " agent_nums, " << skipped_runs << "/" << maps.size() * agent_nums.size() * RUNS_PER_MAP
            << " runs, " << skipped_implementations << "/"
            << (int)maps.size() * (int)agent_nums.size() * (int)RUNS_PER_MAP * (int)magic_enum::enum_count<SIPP_implementation>()
            << " implementations." << std::endl;
}

