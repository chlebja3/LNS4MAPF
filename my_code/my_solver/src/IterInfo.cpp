/*
 * Author: Jan Chleboun
 * Date: 22-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include "IterInfo.h"

SIPPIterationInfo::SIPPIterationInfo(const TimePoint& cur_expanded_, double g_, double h_, int generated_, int expanded_,
                                     int iteration_num_)
    : cur_expanded(cur_expanded_), g(g_), h(h_), generated(generated_), expanded(expanded_), iteration_num(iteration_num_)
{
}
