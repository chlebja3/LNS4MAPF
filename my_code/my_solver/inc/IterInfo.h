/*
 * Author: Jan Chleboun
 * Date: 22-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#pragma once
#include "utils.h"

class SIPPIterationInfo
{
public:
  const TimePoint cur_expanded;
  const double    g;
  const double    h;
  const double    h2;
  const int       generated;
  const int       expanded;
  const int       iteration_num;
  SIPPIterationInfo(const TimePoint& cur_expanded_, double g_, double h_, double h2_, int generated_, int expanded_, int iteration_num_);
};

using SIPPInfo = std::vector<SIPPIterationInfo>;
