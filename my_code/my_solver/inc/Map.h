/*
 * Author: Jan Chleboun
 * Date: 04-04-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#pragma once
#include "utils.h"

class Map
{
public:
  int                  width      = 0;
  int                  height     = 0;
  int                  free_cells = 0;
  std::vector<uint8_t> data;

  Map() = default;
  void               print() const;
  [[nodiscard]] auto is_in(const Point2d& point) const -> bool;
  [[nodiscard]] auto is_in(int loc) const -> bool;
  [[nodiscard]] auto index(const Point2d& point) const -> uint8_t;
  [[nodiscard]] auto index(int loc) const -> uint8_t;
  [[nodiscard]] auto position_to_index(const Point2d& p) const -> int;


  /**
   * @brief Get the neighbors of a point.
   *
   * @param p The point to find neighbors for.
   *
   * @return A vector of points that are neighbors of the input point.
   */
  [[nodiscard]] auto find_neighbors(const Point2d& p) const -> std::vector<Point2d>;

  [[nodiscard]] auto find_neighbors(int loc) const -> std::vector<int>;
};
