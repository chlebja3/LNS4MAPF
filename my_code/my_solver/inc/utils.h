/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <SFML/Graphics.hpp>
#include <cassert>
#include <cstdint>
#include <vector>

// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert((void(msg), exp))

enum Direction
{
  NONE,
  UP,
  RIGHT,
  DOWN,
  LEFT
};


class Node
{
public:
  double g, h, f;

  Node(double g, double h);
  virtual ~Node() = default;  // make Node an abstract class

  // compare
  virtual auto operator>(const Node& other) -> bool;
  virtual auto operator<(const Node& other) -> bool;
  virtual auto operator==(const Node& other) -> bool;
};

class Point2d
{
public:
  int x, y;
  Point2d(int x, int y);
  ~Point2d() = default;

  /**
   * @brief Add two points.
   *
   * @param other The other point to add.
   *
   * @return A new point that is the sum of the two points.
   */
  auto operator+(const Point2d& other) const -> Point2d;

  /**
   * @brief Subtract two points.
   *
   * @param other The other point to subtract.
   *
   * @return A new point that is the difference of the two points.
   */
  auto operator-(const Point2d& other) const -> Point2d;


  /**
   * @brief Check if two points are equal.
   *
   * @param other The other point to compare.
   *
   * @return True if the two points are equal, false otherwise.
   */
  auto operator==(const Point2d& other) const -> bool;


  /**
   * @brief Add a point to this point.
   *
   * @param other The other point to add.
   */
  void add(const Point2d& other);

  /**
   * @brief Operator for printing the point.
   *
   * @param os The output stream.
   * @param p The point to print.
   *
   * @return The output stream.
   */
  friend auto operator<<(std::ostream& os, Point2d const& p) -> std::ostream&;

  /**
   * @brief Hash function for the Point2d class.
   */
  struct hash_point
  {
    auto operator()(const Point2d& p) const -> size_t;
  };


  explicit operator std::string() const;
};

auto find_direction(const Point2d& start, const Point2d& end) -> enum Direction;
auto find_direction(int start, int end) -> enum Direction;

class TimeInterval
{
public:
  int t_min, t_max;

  TimeInterval(int t_min, int t_max);

  auto        operator==(const TimeInterval& other) const -> bool;
  auto        operator+(int x) const -> TimeInterval;
  explicit    operator std::string() const;
  friend auto operator<<(std::ostream& os, TimeInterval const& i) -> std::ostream&;
};

auto overlap(const TimeInterval& i1, const TimeInterval& i2) -> bool;

class TimePoint
{
public:
  int          location;
  TimeInterval interval;

  TimePoint(int location, const TimeInterval& interval);

  auto operator==(const TimePoint& other) const -> bool;
};


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

// define a path as a vectors of (int, int) tuples
using Path          = std::vector<int>;
using PointPath     = std::vector<Point2d>;
using TimePointPath = std::vector<TimePoint>;

auto timepointpath_to_path(const TimePointPath& path) -> Path;

/**
 * @brief Generate a random color. To make the color palette look more visually pleasing, the random color is mixed with (mix_r, mix_g,
 * mix_b).
 *
 * @param mix_r The red channel value of the mix color, recommended value is 255.
 * @param mix_g The green channel value of the mix color, recommended value is 255.
 * @param mix_b The blue channel value of the mix color, recommended value is 255.
 * @param mix_ratio The ratio at which to blend the randomly generated color with the mix color.
 *
 * @return A random color.
 */
auto generate_random_color(int mix_r, int mix_g, int mix_b, double mix_ratio = 0.5) -> sf::Color;

/**
 * @brief Get the path to the project directory, assuming the project is run from the build directory. This function is useful for loading
 * files from the project directory.
 *
 * @return The path to the project directory.
 */
auto get_base_path() -> std::string;
