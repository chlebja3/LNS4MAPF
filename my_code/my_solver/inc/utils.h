/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Node
{
public:
  const double g, h, f;

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

  Point2d operator+(const Point2d& other) const;
  bool    operator==(const Point2d& other) const;
  void    add(const Point2d& other);

  friend std::ostream& operator<<(std::ostream& os, Point2d const& p);

  struct hash_point
  {
    size_t operator()(const Point2d& p) const;
  };
};

class Map
{
public:
  int                  width  = 0;
  int                  height = 0;
  std::vector<uint8_t> data;

  void    print() const;
  bool    is_in(const Point2d& point) const;
  uint8_t index(const Point2d& point) const;
  std::vector<Point2d> find_neighbors(const Point2d& p) const;
};

// define a path as a vectors of (int, int) tuples
typedef std::vector<Point2d> Path;

/**
 * @brief Generate a random color. To make the color palette look more visually pleasing, the random color is mixed with (mix_r, mix_g, mix_b).
 *
 * @param mix_r The red channel value of the mix color, recommended value is 255.
 * @param mix_g The green channel value of the mix color, recommended value is 255.
 * @param mix_b The blue channel value of the mix color, recommended value is 255.
 * @param mix_ratio The ratio at which to blend the randomly generated color with the mix color.
 *
 * @return A random color.
 */
sf::Color generate_random_color(const int mix_r, const int mix_g, const int mix_b, const double mix_ratio = 0.5);

std::string get_base_path();
