/*
 * Author: Jan Chleboun
 * Date: 04-03-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <memory>

#include "Instance.h"


auto main(int argc, char** argv) -> int
{
  const std::vector<std::string> maps({"dummy"});
  const std::vector<std::string> scenes({"dummy_scen_1"});
  const std::vector<int>         agent_nums;

  for (auto map_name : maps)
  {
    for (auto scene_name : scenes)
    {
      for (int agent_num : agent_nums)
      {
        // create the instance
        std::unique_ptr<Instance> instance = std::make_unique<Instance>(map_name, scene_name, agent_num);
      }
    }
  }
}

