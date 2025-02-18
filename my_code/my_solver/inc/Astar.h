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
/**
 * @brief An implementation of the A* algorithm, find the shortest path between two points in a grid map considering 4-neighbourhood.
 *
 * @param start The start point.
 * @param goal The final point.
 * @param map_data The map represented as a 2D grid, where 0 represents a free cell and 1 represents an obstacle.
 *
 * @return The path from the start to the goal.
 */
Path find_path(const Point2d& start, const Point2d& goal, const Map& map_data);

extern int generated_nodes;
}  // namespace ASTAR

