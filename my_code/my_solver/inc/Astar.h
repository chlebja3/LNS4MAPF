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
auto find_path(int agent_num, const Instance& instance) -> Path;

extern int generated_nodes;
}  // namespace ASTAR

