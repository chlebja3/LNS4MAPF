/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include "SIPP.h"
#include "Solver.h"

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
  bool                  is_visualisation;
  std::vector<SIPPInfo> sipp_info;

  // constructor
  LNS(const Instance& instance, bool is_visualisation, double w_ = 1.0);

  // Destructor
  ~LNS() override = default;

  // solve method
  void solve() override;

  auto find_initial_solution() -> bool;
  auto PrioritizedPlanning() -> Solution;

  [[nodiscard]] auto get_num_of_generated_nodes() const -> int;
  [[nodiscard]] auto get_num_of_expanded_nodes() const -> int;

private:
  void destroy_random(Solution& sol) const;
  void repair_default(Solution& sol) const;

  std::unique_ptr<SIPP> planner;
  int                   iteration_num = 0;
  int                   max_iter;
  double                initial_solution_time;
  double                time_limit;
  double                curr_time;
  double                w;
  Operator              destroy_operator;
  Operator              repair_operator;
};
