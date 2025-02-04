/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include "Instance.h"
#include "utils.h"

namespace ASTAR
{
Path find_path(const Point2d& start, const Point2d& goal, const Map& map_data);
}

