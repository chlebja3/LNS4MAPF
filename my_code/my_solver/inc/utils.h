/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <stdint.h>

#include <SFML/Graphics.hpp>
#include <cassert>
#include <vector>

// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert((void(msg), exp))

class Node
{
public:
  double g, h, f;

  Node(const double g, const double h);
  virtual ~Node() = default;  // make Node an abstract class

  // compare
  virtual bool operator>(const Node& other);
  virtual bool operator<(const Node& other);
  virtual bool operator==(const Node& other);
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
  Point2d operator+(const Point2d& other) const;

  /**
   * @brief Subtract two points.
   *
   * @param other The other point to subtract.
   *
   * @return A new point that is the difference of the two points.
   */
  Point2d operator-(const Point2d& other) const;


  /**
   * @brief Check if two points are equal.
   *
   * @param other The other point to compare.
   *
   * @return True if the two points are equal, false otherwise.
   */
  bool operator==(const Point2d& other) const;


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
  friend std::ostream& operator<<(std::ostream& os, Point2d const& p);

  /**
   * @brief Hash function for the Point2d class.
   */
  struct hash_point
  {
    size_t operator()(const Point2d& p) const;
  };
};


class TimeInterval
{
public:
  int t_min, t_max;

  TimeInterval(const int t_min, const int t_max);

  bool                 operator==(const TimeInterval& other) const;
  TimeInterval         operator+(const int x) const;
                       operator std::string() const;
  friend std::ostream& operator<<(std::ostream& os, TimeInterval const& p);
};

bool overlap(const TimeInterval& i1, const TimeInterval& i2);

class TimePoint
{
public:
  Point2d      position;
  TimeInterval interval;

  TimePoint(const int x, const int y, const int t_min, const int t_max);
  TimePoint(const Point2d& position, const TimeInterval& interval);

  struct hash_time_point
  {
    size_t operator()(const TimePoint& p) const;
  };

  bool operator==(const TimePoint& other) const;
};


class Map
{
public:
  int                  width      = 0;
  int                  height     = 0;
  int                  free_cells = 0;
  std::vector<uint8_t> data;

  Map() = default;
  void    print() const;
  bool    is_in(const Point2d& point) const;
  uint8_t index(const Point2d& point) const;
  int     position_to_index(const Point2d& p) const;


  /**
   * @brief Get the neighbors of a point.
   *
   * @param p The point to find neighbors for.
   *
   * @return A vector of points that are neighbors of the input point.
   */
  std::vector<Point2d> find_neighbors(const Point2d& p) const;
};

// define a path as a vectors of (int, int) tuples
typedef std::vector<Point2d>   Path;
typedef std::vector<TimePoint> TimePointPath;

Path timepointpath_to_path(const TimePointPath& path);

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
sf::Color generate_random_color(const int mix_r, const int mix_g, const int mix_b, const double mix_ratio = 0.5);

/**
 * @brief Get the path to the project directory, assuming the project is run from the build directory. This function is useful for loading
 * files from the project directory.
 *
 * @return The path to the project directory.
 */
std::string get_base_path();
