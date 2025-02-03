/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>


class Map
{
public:
  int                  width  = 0;
  int                  height = 0;
  std::vector<uint8_t> data;

  void print() const;
};

// an abstract node class
class Node
{
public:
  Node*        parent;
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
  void add(const Point2d& other);
};

// define a path as a vectors of (int, int) tuples
typedef std::vector<Point2d> Path;
std::vector<Point2d> find_neighbors(const Point2d& p, const Map& map_data);

// generate a random visually pleasing color, based on
// https://stackoverflow.com/questions/43044/algorithm-to-randomly-generate-an-aesthetically-pleasing-color-palette
sf::Color generate_random_color(const int mix_r, const int mix_g, const int mix_b, const double mix_ratio = 0.5);

