/*
 * Author: Jan Chleboun
 * Date: 06-04-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include "Experiment.h"

#include <utility>

#include "LNS.h"
#include "Solver.h"

Experiment::Experiment(std::string experiment_name_, std::function<void(LNS& vis)> experiment_function_, std::vector<std::string> maps_,
                       std::vector<std::vector<int>> agent_nums_, const std::vector<Algorithm>& algorithms_, int runs_per_map_,
                       bool show_progress_)
    : experiment_name(std::move(experiment_name_)),
      experiment_function(std::move(experiment_function_)),
      maps(std::move(maps_)),
      agent_nums(std::move(agent_nums_)),
      algorithms(algorithms_),
      runs_per_map(runs_per_map_),
      show_progress(show_progress_),
      rnd_gen(std::mt19937(std::random_device{}())),
      dist(0, std::numeric_limits<int>::max())
{
  skips = what_can_be_skipped(experiment_name, maps, agent_nums, runs_per_map, algorithms);
  if (show_progress)
  {
    create_progress_bars(agent_nums[0].size());
  }
}

void Experiment::create_progress_bars(int agent_nums_init)
{
  assertm(show_progress, "Creating progress bars when progress should not be shown.");
  progress_bars.push_back(create_progress_bar("Maps:\t\t", static_cast<int>(maps.size())));
  progress_bars.push_back(create_progress_bar("Agent nums:\t", agent_nums_init));
  progress_bars.push_back(create_progress_bar("Scene:\t\t", runs_per_map));
  multi_progress_bar.emplace(*progress_bars[0], *progress_bars[1], *progress_bars[2]);

  // first tick
  multi_progress_bar->set_progress<0>(static_cast<size_t>(0));
  multi_progress_bar->set_progress<1>(static_cast<size_t>(0));
  multi_progress_bar->set_progress<2>(static_cast<size_t>(0));
}

void Experiment::run()
{
  // iterate over all maps
  for (int i = 0; i < static_cast<int>(static_cast<int>(maps.size())); i++)
  {
    // reset the map progress bars
    if (show_progress)
    {
      multi_progress_bar->set_progress<0>(static_cast<size_t>(i));
      multi_progress_bar->set_option<0>(
          indicators::option::PostfixText{std::to_string(i) + "/" + std::to_string(static_cast<int>(maps.size()))});
    }

    // skip maps that were already solved
    const int start_offset_map = sum_agent_nums * runs_per_map * static_cast<int>(algorithms.size());
    sum_agent_nums += static_cast<int>(agent_nums[i].size());
    const int end_offset_map = sum_agent_nums * runs_per_map * static_cast<int>(algorithms.size());

    if (std::all_of(skips.begin() + start_offset_map, skips.begin() + end_offset_map, [](bool b) { return b; }))
    {
      if (show_progress)
      {
        multi_progress_bar->set_progress<1>(agent_nums[i].size());
        multi_progress_bar->set_progress<2>(static_cast<size_t>(runs_per_map));
      }
      skipped_maps++;
      skipped_agent_nums += static_cast<int>(agent_nums[i].size());
      skipped_runs += static_cast<int>(agent_nums[i].size()) * runs_per_map;
      skipped_implementations += static_cast<int>(agent_nums[i].size()) * runs_per_map * static_cast<int>(algorithms.size());
      continue;
    }

    // retrieve map name and scene names
    const std::string              map_name = maps[i];
    const std::vector<std::string> scenes   = get_scene_names(map_name, runs_per_map);

    if (show_progress)
    {
      multi_progress_bar->set_option<1>(indicators::option::MaxProgress{(int)agent_nums[i].size()});
    }
    // iterate over agent numbers
    for (int j = 0; j < static_cast<int>(agent_nums[i].size()); j++)
    {
      // update the progress bar iteration number display
      if (show_progress)
      {
        multi_progress_bar->set_option<1>(indicators::option::PostfixText{std::to_string(j) + "/" + std::to_string(agent_nums[i].size())});
        multi_progress_bar->set_progress<1>(static_cast<size_t>(j));
      }

      // skip agent_nums that were already solved
      const int start_offset_an = start_offset_map + j * runs_per_map * static_cast<int>(algorithms.size());
      const int end_offset_an   = start_offset_map + (j + 1) * runs_per_map * static_cast<int>(algorithms.size());
      if (std::all_of(skips.begin() + start_offset_an, skips.begin() + end_offset_an, [](bool b) { return b; }))
      {
        if (show_progress)
        {
          multi_progress_bar->set_progress<2>((size_t)runs_per_map);
        }
        skipped_agent_nums++;
        skipped_runs += runs_per_map;
        skipped_implementations += runs_per_map * static_cast<int>(algorithms.size());
        continue;
      }

      int agent_num = agent_nums[i][j];

      // load all instances TODO load only instances that will not be skipped
      auto [instances, preprocessing_times] = load_instances(scenes, map_name, agent_num);

      // make sure every map gets enough runs
      for (int k = 0; k < runs_per_map; k++)
      {
        // update the progress bar iteration number display
        if (show_progress)
        {
          multi_progress_bar->set_option<2>(indicators::option::PostfixText{std::to_string(k) + "/" + std::to_string(runs_per_map)});
          multi_progress_bar->set_progress<2>(static_cast<size_t>(k));
        }

        // skip runs that were already solved
        const int start_offset_run = start_offset_an + k * static_cast<int>(algorithms.size());
        const int end_offset_run   = start_offset_an + (k + 1) * static_cast<int>(algorithms.size());
        if (std::all_of(skips.begin() + start_offset_run, skips.begin() + end_offset_run, [](bool b) { return b; }))
        {
          skipped_runs++;
          skipped_implementations += static_cast<int>(algorithms.size());
          continue;
        }

        // choose instance
        assertm(scenes.size() == instances.size(), "The size of scenes and instances must be the same.");
        const int          idx        = k % static_cast<int>(scenes.size());
        const std::string& scene_name = scenes[idx];
        const Instance&    instance   = instances[idx];
        // check whether enough agents loaded
        if (instance.get_num_of_agents() != agent_num)
        {
          throw std::runtime_error("Loaded " + std::to_string(instance.get_num_of_agents()) + " agents instead of " +
                                   std::to_string(agent_num));
        }

        // generate a random seed (same for all implementations) TODO - for experiments found in the database use the same seed
        int seed = dist(rnd_gen);

        // iterate over all implementations
        for (int l = 0; l < static_cast<int>(algorithms.size()); l++)
        {
          // skip already solved
          if (skips[start_offset_run + l])
          {
            skipped_implementations++;
            continue;
          }

          const Algorithm& algo = algorithms[l];

          // create the seeded random generator (for experiment reproducibility)
          auto                      rnd_generator       = std::mt19937(seed);
          double                    w                   = 1.0;
          const SIPP_implementation sipp_implementation = std::get<SIPP_implementation>(algo.type);
          if (sipp_implementation == SIPP_implementation::SIPP_suboptimal)
          {
            for (const auto& it : algo.parameters)
            {
              if (it.first == "w")
              {
                w = std::any_cast<double>(it.second);
              }
            }
          }

          // prepare the solver
          LNS solver(instance, rnd_generator, nullptr, algo.lns_settings);

          // run the experiment function and measure its execution time
          Clock clock;
          clock.start();
          experiment_function(solver);
          auto [pp_duration_wall, pp_duration_cpu] = clock.end();

          // if feasible, calculate the cost
          if (solver.solution.feasible)
          {
            solver.solution.calculate_cost(instance);
          }

          // write the results of the experiment
          const std::string algorithm_name = algo.get_name();
          nlohmann::json    experiment_res = {{"experiment_name", experiment_name},
                                           {"map_name", map_name},
                                           {"scene_name", scene_name},
                                           {"num_agents", agent_num},
                                           {"algo_name", algorithm_name},
                                           {"algo_parameters", algo.get_parameters_str()},
                                           {"feasible", solver.solution.feasible},
                                           {"sum_of_dist", instance.get_sum_of_distances()},
                                           {"sum_of_cost", solver.solution.sum_of_costs},
                                           {"makespan", solver.solution.makespan},
                                           {"preprocessing_time_wall", preprocessing_times[idx].first},
                                           {"preprocessing_time_cpu", preprocessing_times[idx].second},
                                           {"experiment_time_wall", pp_duration_wall},
                                           {"experiment_time_cpu", pp_duration_cpu},
                                           {"expanded", solver.get_num_of_expanded_nodes()},
                                           {"generated", solver.get_num_of_generated_nodes()},
                                           {"seed", seed}};

          // save the experiment
          save_experiment(experiment_res, agent_num, map_name, scene_name, experiment_name, algorithm_name);
        }
      }
    }
  }
}
