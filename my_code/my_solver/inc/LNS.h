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
  // constructor
  LNS(const Instance& instance);

  // Destructor
  ~LNS() = default;

  // solve method
  void solve() override;

  void find_initial_solution();

private:
  Solution              PrioritizedPlanning();
  std::unique_ptr<SIPP> planner;
};

