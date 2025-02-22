/*
 * Author: Jan Chleboun
 * Date: 22-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include "IterInfo.h"

SIPPIterationInfo::SIPPIterationInfo(const TimePoint& cur_expanded, const double g, const double h, const int generated, const int expanded,
                                     const int iteration_num)
    : cur_expanded(cur_expanded), g(g), h(h), generated(generated), expanded(expanded), iteration_num(iteration_num){};
