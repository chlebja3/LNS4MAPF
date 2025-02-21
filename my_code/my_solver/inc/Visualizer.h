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
#include <memory>
#include <thread>

#include "Instance.h"
#include "SharedData.h"

class Visualizer
{
public:
  // constructor and destructor
  Visualizer(const Instance& instance, SharedData& shared_data);
  ~Visualizer();

  // manage the underlying thread
  void start();
  void stop();
  void join_thread();

  // the function called in the thread
  void run();

  // create a texture from the map
  void mapToTexture(const Map& map_data);

  void update_agent_circles(const std::vector<Point2d>& positions);
  void update_goal_circles(const std::vector<Point2d>& positions);
  void update_path(const int agent_num, const Path& path);


private:
  // draw the agents
  void            draw_agents(sf::RenderWindow& window);
  void            draw_goals(sf::RenderWindow& window);
  void            draw_paths(sf::RenderWindow& window);
  double          rescale(double x);
  std::thread     vis_thread;  // the underlying thread
  bool            running;     // indicator whether the thread should be running
  const Instance& instance;
  SharedData&     shared_data;
  void            ShowDockingSpace();

  // drawing objects
  sf::Texture                  map_texture;  // the map texture
  sf::Sprite                   map_sprite;   // the sprite that represents the map
  std::vector<sf::CircleShape> agents_circles;
  std::vector<sf::CircleShape> goal_circles;
  std::vector<sf::Color>       agents_colors;
  std::vector<sf::VertexArray> agents_paths;
  double                       scale;
};
