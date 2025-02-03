/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: 
 */

#pragma once
#include "utils.h"
#include "Instance.h"

namespace ASTAR
{
  Path find_path(const std::tuple<int, int>& start, const std::tuple<int, int>& goal, const Map& map_data);
}

