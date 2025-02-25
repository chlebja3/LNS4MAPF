/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include "SIPP.h"
#include "Solver.h"

class LNS : public Solver
{
public:
  bool                  is_visualisation;
  std::vector<SIPPInfo> sipp_info;

  // constructor
  LNS(const Instance& instance, bool is_visualisation);

  // Destructor
  ~LNS() override = default;

  // solve method
  void solve() override;

  void find_initial_solution();
  auto PrioritizedPlanning() -> Solution;

  [[nodiscard]] auto get_num_of_generated_nodes() const -> int;

private:
  std::unique_ptr<SIPP> planner;
};

