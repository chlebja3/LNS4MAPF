/*
 * Author: Jan Chleboun
 * Date: 04-03-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <memory>
#include <random>

#include "Instance.h"
#include "LNS.h"

#define RUNS_PER_MAP 50


auto main(int argc, char** argv) -> int
{
  const std::vector<std::string> maps({"dummy"});
  const std::vector<std::string> scenes({"dummy_scen_1"});
  const std::vector<int>         agent_nums;

  // nondeterministic random generator for generating seeds
  std::random_device rd;
  auto               nondeterministic_rnd_generator = std::mt19937(rd());

  // iterate over all maps
  for (const auto& map_name : maps)
  {
    // iterate over agent numbers
    for (int agent_num : agent_nums)
    {
      int count = 0;
      // make sure every map gets enough runs
      while (count < RUNS_PER_MAP)
      {
        // iterate over scenes
        for (const auto& scene_name : scenes)
        {
          // create the instance
          Instance instance(map_name, scene_name, agent_num);

          // generate a random seed
          uint32_t seed = nondeterministic_rnd_generator();

          // create the seeded random generator (for experiment reproducibility)
          auto rnd_generator = std::mt19937(seed);

          // create solver
          LNS solver(instance, rnd_generator, INFO_type::experiment, 1.0);

          // find the solution

          count++;
        }
      }
    }
  }
}

