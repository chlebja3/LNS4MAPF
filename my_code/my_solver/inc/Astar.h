/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: An implementation of the A* algorithm, find the shortest path between two points in a grid map considering 4-neighbourhood.
 */

#pragma once
#include "Instance.h"
#include "utils.h"

namespace ASTAR
{
Path find_path(const int agent_num, const Instance& instance);

extern int generated_nodes;
}  // namespace ASTAR

