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

class LNSIterationInfo
{
public:
  int                   iteration_num;
  bool                  accepted;
  int                   improvement;
  std::vector<SIPPInfo> sipp_info;
  LNSIterationInfo(int iteration_num_, bool accepted_, int improvement_, std::vector<SIPPInfo>& sipp_info_)
      : iteration_num(iteration_num_), accepted(accepted_), improvement(improvement_), sipp_info(std::move(sipp_info_))
  {
  }
};

using LNSInfo = std::vector<LNSIterationInfo>;
