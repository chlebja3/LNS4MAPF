/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <bits/stdc++.h>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <atomic>
#include <memory>
#include <thread>

#include "Instance.h"
#include "IterInfo.h"
#include "SharedData.h"

class Visualization
{
public:
  int                                     time              = 0;
  int                                     max_time          = 100;
  bool                                    is_playing        = false;
  int                                     iteration_counter = 0;
  float                                   speed             = 2.0;
  std::function<void(Visualization& vis)> update_func;
  explicit Visualization(std::function<void(Visualization& vis)> update_func_);
  void start();
  void reset();
  void pause();
  void update();
  void step_forward();
  void step_backward();
  void new_data(int new_len);
  void next_iteration();
};

class Visualizer
{
public:
  // constructor and destructor
  Visualizer(const Instance& instance_, SharedData& shared_data_);
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
  void update_path(int agent_num, const Path& path);
  void update_all_paths(int time);
  void update_cell_rectangles_active(int time);
  void create_cell_rectangles();
  auto get_agent_position(int agent_num, int time) const -> const Point2d;
  auto get_agent_location(int agent_num, int time) const -> int;
  auto get_agent_positions(int time) const -> std::vector<Point2d>;
  auto get_agent_locations(int time) const -> std::vector<int>;
  void solution_visualization_update(const Visualization& vis);
  void sipp_visualization_update(const Visualization& vis);


private:
  void load_fonts();

  void adjust_sfml_part_viewport(sf::RenderWindow& window, sf::View& view, float left_panel_size, float right_panel_size,
                                 float bottom_panel_size);

  void process_events(sf::RenderWindow& window, sf::View& view);

  void create_left_panel();
  void create_right_panel();
  void create_bottom_panel();
  // draw the agents
  void            draw_agents(sf::RenderWindow& window);
  void            draw_goals(sf::RenderWindow& window);
  void            draw_paths(sf::RenderWindow& window);
  void            draw_cell_rectangles(sf::RenderWindow& window);
  std::thread     vis_thread;  // the underlying thread
  bool            running;     // indicator whether the thread should be running
  const Instance& instance;
  SharedData&     shared_data;

  void create_docking_space();
  void create_menu_bar();

  // drawing objects
  sf::Texture                                  map_texture;  // the map texture
  sf::Sprite                                   map_sprite;   // the sprite that represents the map
  std::vector<sf::CircleShape>                 agents_circles;
  std::vector<sf::CircleShape>                 goal_circles;
  std::vector<sf::Color>                       agents_colors;
  std::vector<sf::VertexArray>                 agents_paths_vis;
  std::vector<Path>                            agents_paths;
  std::vector<std::vector<sf::RectangleShape>> cell_rectangles;
  std::vector<std::vector<bool>>               cell_rectangles_active;
  std::vector<SIPPInfo>                        sipp_info;
  int                                          selected_agent = -1;
  bool                                         is_moving      = false;
  sf::Vector2i                                 last_click_pos;
  sf::RectangleShape                           curr_expanded_rectangle;
  std::vector<std::string>                     log;

  // Zoom settings
  float       zoom_level = 1.0;
  const float zoom_step  = 1.1F;

  // Visualizations
  Visualization solution_vis;
  Visualization sipp_vis;
};
