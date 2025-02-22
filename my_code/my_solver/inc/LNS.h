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
  LNS(const Instance& instance, const bool is_visualisation);

  // Destructor
  ~LNS(void) = default;

  // solve method
  void solve(void) override;

  void     find_initial_solution(void);
  Solution PrioritizedPlanning(void);

  int get_num_of_generated_nodes(void) const;

private:
  std::unique_ptr<SIPP> planner;
};

