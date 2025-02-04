/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <atomic>
#include <thread>

#include "Instance.h"

class Visualizer
{
public:
  // constructor and destructor
  Visualizer(Instance& instance);
  ~Visualizer();

  // manage the underlying thread
  void start();
  void stop();
  void join_thread();

  // the function called in the thread
  void run();

  // create a texture from the map
  void mapToTexture(const Map& map_data);

private:
  // draw the agents
  void                 draw_agents(sf::RenderWindow& window);
  void                 draw_goals(sf::RenderWindow& window);
  void                 draw_path(sf::RenderWindow& window, const Path& path, const sf::Color& color);
  void                 draw_arrow(sf::RenderWindow& window, const Point2d& start, const Point2d& end, const sf::Color& color);
  std::thread          vis_thread;  // the underlying thread
  std::atomic<bool>    running;     // indicator whether the thread should be running
  int                  map_width;
  int                  map_height;
  std::vector<Point2d> start_positions;
  std::vector<Point2d> goal_positions;
  std::vector<Path>    optimal_paths;

  // drawing objects
  sf::Texture                  map_texture;  // the map texture
  sf::Sprite                   map_sprite;   // the sprite that represents the map
  std::vector<sf::CircleShape> agents_circles;
  std::vector<sf::Color>       agents_colors;
  double                       scale;
};
