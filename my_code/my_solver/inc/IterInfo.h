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
  const int       generated;
  const int       expanded;
  const int       iteration_num;
  SIPPIterationInfo(const TimePoint& cur_expanded, const double g, const double h, const int generated, const int expanded,
                    const int iteration_num);
};

typedef std::vector<SIPPIterationInfo> SIPPInfo;
