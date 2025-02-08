/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "LNS.h"

LNS::LNS(std::shared_ptr<Instance> instance) : Solver("LNS", instance){};

Solution LNS::solve()
{
  // calculate initial solution
  Solution sol = Solution();

  // improve the solution
  return sol;
}
