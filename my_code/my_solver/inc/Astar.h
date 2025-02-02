/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: 
 */

#pragma once
#include "utils.h"

namespace ASTAR
{
  Path find_path(const std::tuple<int, int>& start, const std::tuple<int, int>& goal, const std::vector<uint8_t>& map_data, const int map_width, const int map_height);
}

