/*
 * Author: Jan Chleboun
 * Date: 06-04-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <string>
#include <vector>

#include "LNS.h"
#include "experiment_utils.h"

class Experiment
{
public:
  Experiment(std::string experiment_name_, std::function<void(LNS& lns)> experiment_function, std::vector<std::string> maps_,
             std::vector<std::vector<int>> agent_nums_, const std::vector<Algorithm>& algorithms_, int runs_per_map_,
             bool show_progress_ = false);
  void run();

private:
  void create_progress_bars(int agent_nums_init);
  // experiment settings
  std::string                         experiment_name;
  std::function<void(LNS& lns)>       experiment_function;
  const std::vector<std::string>      maps;
  const std::vector<std::vector<int>> agent_nums;
  const std::vector<Algorithm>&       algorithms;
  int                                 runs_per_map;
  bool                                show_progress;

  // skips
  std::vector<bool> skips;
  int               skipped_maps            = 0;
  int               skipped_agent_nums      = 0;
  int               skipped_runs            = 0;
  int               skipped_implementations = 0;
  int               sum_agent_nums          = 0;


  std::mt19937                       rnd_gen;
  std::uniform_int_distribution<int> dist;

  // visualization
  std::vector<std::unique_ptr<indicators::ProgressBar>>                progress_bars;
  std::optional<indicators::MultiProgress<indicators::ProgressBar, 3>> multi_progress_bar;
};
