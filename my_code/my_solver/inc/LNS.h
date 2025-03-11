/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <random>

#include "SIPP.h"
#include "SharedData.h"
#include "Solver.h"

class LNS_settings
{
public:
  LNS_settings(int max_iter_, double time_limit_, SIPP_settings sipp_settings_)
      : max_iter(max_iter_), time_limit(time_limit_), sipp_settings(sipp_settings_)
  {
  }
  int           max_iter;
  double        time_limit;
  SIPP_settings sipp_settings;
};

class Operator
{
public:
  explicit Operator(std::function<void(Solution& sol)> func_) : func(func_)
  {
  }

  void apply(Solution& solution)
  {
    func(solution);
  }

private:
  const std::function<void(Solution& sol)> func;
};

class LNS : public Solver
{
public:
  std::vector<SIPPInfo> sipp_info;
  std::unique_ptr<SIPP> planner;

  // constructor
  LNS(const Instance& instance_, std::mt19937& rnd_generator_, SharedData* shared_data_, const LNS_settings& settings_);

  // Destructor
  ~LNS() override = default;

  // solve method
  void solve() override;

  auto find_initial_solution() -> bool;
  auto PrioritizedPlanning(SIPP_implementation sipp_implementation) -> Solution;

  [[nodiscard]] auto get_num_of_generated_nodes() const -> int
  {
    return planner->generated_all_iter;
  }
  [[nodiscard]] auto get_num_of_expanded_nodes() const -> int
  {
    return planner->expanded_all_iter;
  }
  void discard_solution(const Solution& sol, const Solution& prev_sol) const;

private:
  void destroy_random(Solution& sol) const;
  void repair_default(Solution& sol) const;

  SharedData*         shared_data;
  const LNS_settings& settings;
  int                 iteration_num = 0;
  double              initial_solution_time;
  double              curr_time;
  Operator            destroy_operator;
  Operator            repair_operator;
};
