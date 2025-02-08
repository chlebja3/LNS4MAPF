/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include "Solver.h"

class LNS : public Solver
{
public:
  // constructor
  LNS(std::shared_ptr<Instance> instance);

  // solve method
  Solution solve() override;
};

