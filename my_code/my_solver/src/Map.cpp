/*
 * Author: Jan Chleboun
 * Date: 04-04-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include "Map.h"

#include <iostream>

auto Map::is_in(const Point2d& p) const -> bool
{
  return !(p.x < 0 || p.x >= width || p.y < 0 || p.y >= height);
}

auto Map::is_in(const int loc) const -> bool
{
  return loc >= 0 && loc < width * height;
}

auto Map::index(const Point2d& point) const -> uint8_t
{
  assertm(is_in(point), "Trying to index a point that is not in the map.");
  return data[point.y * width + point.x];
}

auto Map::index(const int loc) const -> uint8_t
{
  assertm(is_in(loc), "Trying to index a point that is not in the map.");
  return data[loc];
}

auto Map::position_to_index(const Point2d& p) const -> int
{
  return p.y * width + p.x;
}


auto Map::find_neighbors(const int loc) const -> std::vector<int>
{
  assertm(is_in(loc) && index(loc) == 0, "Invalid position.");
  const std::array<int, 4> neighbors_rel = {-width, -1, 1, width};
  std::vector<int>         ret;
  for (const auto& neigh_rel : neighbors_rel)
  {
    // edges - sjip the neighboring indexes which  are on different row
    if (loc % width == 0 && neigh_rel == -1)
    {
      continue;  // left edge
    }
    if (loc % width == width - 1 && neigh_rel == 1)
    {
      continue;  // right edge
    }
    const int neigh = loc + neigh_rel;
    if (is_in(neigh) && index(neigh) == 0)
    {
      ret.push_back(neigh);
    }
  }
  return ret;
}

auto Map::find_neighbors(const Point2d& pos) const -> std::vector<Point2d>
{
  assertm(is_in(pos) && index(pos) == 0, "Invalid position.");
  std::vector<Point2d>         ret;
  const std::array<Point2d, 4> neighbors = {Point2d(0, 1), Point2d(1, 0), Point2d(0, -1), Point2d(-1, 0)};
  for (const auto& it : neighbors)
  {
    const Point2d neigh = pos + it;
    if (!is_in(neigh))
    {
      continue;
    }
    if (index(neigh) != 0)
    {
      continue;
    }
    ret.push_back(neigh);
  }
  return ret;
}

void Map::print() const
{
  std::cout << "Map size: " << width << "x" << height << std::endl;
  assertm(width * height > 0, "Invalid map size.");
  for (int i = 0; i < (int)data.size(); i++)
  {
    std::cout << (int)data[i];
    if (i % width == height - 1)
    {
      std::cout << std::endl;
    }
    else
    {
      std::cout << " ";
    }
  }
}
