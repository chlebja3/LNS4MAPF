/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <utils.h>

#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>

#define MIN_BRIGHTNESS 20
#define MAX_BRIGHTNESS 230

Node::Node(const double g, const double h) : g(g), h(h), f(g + h){};

bool Node::operator>(const Node& other)
{
  return f > other.f;
}

bool Node::operator<(const Node& other)
{
  return f < other.f;
}

bool Node::operator==(const Node& other)
{
  return g == other.g && h == other.h;
}


Point2d::Point2d(int x, int y) : x(x), y(y){};

Point2d Point2d::operator+(const Point2d& other) const
{
  Point2d ret(x + other.x, y + other.y);
  return ret;
}

bool Point2d::operator==(const Point2d& other) const
{
  return x == other.x && y == other.y;
}

void Point2d::add(const Point2d& other)
{
  x += other.x;
  y += other.y;
}

size_t Point2d::hash_point::operator()(const Point2d& p) const
{
  // hash x and y, bit shift to break the symmetry (x, y) wont have the same hash as (y, x) and xor to finalize
  return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
}

enum Direction find_direction(const Point2d& start, const Point2d& end)
{
  if (start.x > end.x) return Direction::LEFT;
  if (start.y > end.y) return Direction::UP;
  if (start.y < end.y) return Direction::DOWN;
  if (start.x < end.x) return Direction::RIGHT;
  return Direction::NONE;
}

TimeInterval::TimeInterval(const int t_min, const int t_max) : t_min(t_min), t_max(t_max){};

bool TimeInterval::operator==(const TimeInterval& other) const
{
  return t_min == other.t_min && t_max == other.t_max;
}

TimeInterval TimeInterval::operator+(const int x) const
{
  return TimeInterval(t_min + x, t_max + x);
}

TimeInterval::operator std::string() const
{
  return "(" + std::to_string(t_min) + "," + std::to_string(t_max) + ")";
}

bool overlap(const TimeInterval& i1, const TimeInterval& i2)
{
  if (i1.t_max < i2.t_min || i2.t_max < i1.t_min)
    return false;
  else
    return true;
}

TimePoint::TimePoint(const int x, const int y, const int t_min, const int t_max) : position(Point2d(x, y)), interval(t_min, t_max){};
TimePoint::TimePoint(const Point2d& position, const TimeInterval& interval) : position(position), interval(interval){};

bool TimePoint::operator==(const TimePoint& other) const
{
  return position == other.position && interval == other.interval;
}

size_t TimePoint::hash_time_point::operator()(const TimePoint& p) const
{
  // hash x, y, tmin, tmax, bit shift to break the symmetry and xor to finalize
  return std::hash<int>()(p.position.x) ^ (std::hash<int>()(p.position.y) << 1) ^ (std::hash<int>()(p.interval.t_min) << 2) ^
         (std::hash<int>()(p.interval.t_max) << 3);
}

bool Map::is_in(const Point2d& p) const
{
  return !(p.x < 0 || p.x >= width || p.y < 0 || p.y >= height);
}

uint8_t Map::index(const Point2d& point) const
{
  assertm(is_in(point), "Trying to index a point that is not in the map.");
  return data[point.y * width + point.x];
}

int Map::position_to_index(const Point2d& p) const
{
  return p.y * width + p.x;
}

// functions to print Point2d and TimeInterval

std::ostream& operator<<(std::ostream& os, Point2d const& p)
{
  return os << "(" << p.x << "," << p.y << ")";
}

std::ostream& operator<<(std::ostream& os, TimeInterval const& i)
{
  return os << "(" << i.t_min << "," << i.t_max << ")";
}

std::vector<Point2d> Map::find_neighbors(const Point2d& point) const
{
  if (!is_in(point)) return {};  // invalid point
  std::vector<Point2d>         ret;
  const std::array<Point2d, 4> neighbors = {Point2d(0, 1), Point2d(1, 0), Point2d(0, -1), Point2d(-1, 0)};
  for (auto& it : neighbors)
  {
    Point2d neigh = point + it;
    if (!is_in(neigh)) continue;
    if (index(neigh) != 0) continue;
    ret.push_back(neigh);
  }
  return ret;
}

void Map::print(void) const
{
  std::cout << "Map size: " << width << "x" << height << std::endl;
  assertm(width * height > 0, "Invalid map size.");
  for (int i = 0; i < (int)data.size(); i++)
  {
    std::cout << (int)data[i];
    if (i % width == height - 1)
      std::cout << std::endl;
    else
      std::cout << " ";
  }
}

Path timepointpath_to_path(const TimePointPath& tp_path)
{
  Path path;
  for (auto it = tp_path.begin(); it != tp_path.end(); it++)
  {
    // for last element add the position only once, as it is obvious that the agent stays in the last position forever
    int N = 1;

    // find the start of the next time interval
    if (it != std::prev(tp_path.end()))
    {
      N = std::next(it)->interval.t_min - it->interval.t_min;
    }

    // insert N copies to the end of the vector
    path.resize(path.size() + N, it->position);
  }
  return path;
}

// based on https://stackoverflow.com/questions/43044/algorithm-to-randomly-generate-an-aesthetically-pleasing-color-palette
sf::Color generate_random_color(const int mix_r, const int mix_g, const int mix_b, const double mix_ratio)
{
  while (true)
  {
    const int r = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_r * mix_ratio);
    const int g = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_g * mix_ratio);
    const int b = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_b * mix_ratio);
    // make sure the color is not too similar to black or white
    if (MIN_BRIGHTNESS <= (r + g + b) / 3 && (r + g + b) / 3 <= MAX_BRIGHTNESS)
    {
      return sf::Color(r, g, b);
    }
  }
}

// Function to get the directory of the running executable
std::string get_base_path()
{
  return std::filesystem::canonical("/proc/self/exe").parent_path().parent_path().string();  // Linux only
}
