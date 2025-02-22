/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "Visualizer.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "IconsFontAwesome5.h"
#include "imgui-SFML.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "utils.h"

// default size of the panels (0 - 1)
#define LEFT_PANEL_SIZE_DEFAULT 0.2
#define RIGHT_PANEL_SIZE_DEFAULT 0.2
#define BOTTOM_PANEL_SIZE_DEFAULT 0.2

#define CELL_SIZE 20
#define BORDER_SIZE 1
#define GOAL_VIS_THICKNESS 3
#define LINE_WIDTH 3  // denominator of line width, so line width is 1/LINE_WIDTH * CELL_SIZE
#define MENU_BAR_SIZE 40
#define ICON_SIZE 20.0f
#define FRAME_RATE 60

Visualizer::Visualizer(const Instance& instance, SharedData& shared_data)
    : instance(instance),
      shared_data(shared_data),
      solution_vis([this](Visualization& vis) { this->solution_visualization_update(vis); }),
      sipp_vis([this](Visualization& vis) { this->sipp_visualization_update(vis); })
{
  // generate random colors for the agents
  for (int i = 0; i < (int)instance.get_start_positions().size(); i++)
  {
    agents_colors.push_back(generate_random_color(255, 255, 255, 0.25));
  }

  // create the map texture
  mapToTexture(instance.get_map_data());

  // generate the circles that represent agents and their goals
  update_agent_circles(instance.get_start_positions());
  update_goal_circles(instance.get_goal_positions());

  // initialize visualization for the astar paths
  for (int i = 0; i < (int)instance.get_optimal_paths().size(); i++)
  {
    update_path(i, instance.get_optimal_paths()[i]);
  }
  std::cout << "Visualizer initialized. " << std::endl;
}

Visualizer::~Visualizer()
{
  stop();
  join_thread();
}

void Visualizer::start()
{
  // start the underlying thread
  running    = true;
  vis_thread = std::thread(&Visualizer::run, this);
}

void Visualizer::stop()
{
  // set the stop signal
  running = false;
}

void Visualizer::join_thread()
{
  if (vis_thread.joinable())
  {
    vis_thread.join();
  }
}

void Visualizer::run()
{
  // calculate useful constants
  const int screen_width  = (int)sf::VideoMode::getDesktopMode().width;
  const int screen_height = (int)sf::VideoMode::getDesktopMode().height;

  // get the map size
  const int map_width  = instance.get_map_data().width;
  const int map_height = instance.get_map_data().height;

  // turn on antialiasing
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8.0;

  // create the sfml window
  sf::RenderWindow window(sf::VideoMode(screen_width, screen_height), "MAPF solver", sf::Style::Default, settings);

  // set framerate
  window.setFramerateLimit(FRAME_RATE);

  // check whether the window was created
  if (!ImGui::SFML::Init(window)) throw std::runtime_error("Unable to initialize ImGUI.");

  // Activate the window for OpenGL rendering
  window.setActive();

  // create a view with the same size as the map and center in the middle of the map
  sf::View view(sf::Vector2f((float)map_width * ((float)CELL_SIZE / 2), (float)map_height * ((float)CELL_SIZE / 2)),
                sf::Vector2f((float)map_width * CELL_SIZE, (float)map_height * CELL_SIZE));

  // adjust the sml part so that it fits between the panels but the aspect ratio is preserved
  adjust_sfml_part_viewport(window, view, LEFT_PANEL_SIZE_DEFAULT, RIGHT_PANEL_SIZE_DEFAULT, BOTTOM_PANEL_SIZE_DEFAULT);

  // load the fonts
  load_fonts();

  // enable docking
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  sf::Clock deltaClock;

  // the main loop
  while (window.isOpen() && running)
  {
    // check whether new paths arrived
    if (solution_vis.is_playing)
    {
      solution_vis.next_iteration();
    }
    else if (shared_data.are_new_paths.load(std::memory_order_acquire))
    {
      // get the new paths from the shared structure
      std::tie(agents_paths, sipp_info) = shared_data.consume_paths();
      assertm(instance.num_of_agents == (int)agents_paths.size(), "Number of paths should be the same as the number of agents.");

      int max_path_len = 0;
      // construct the path visualizations and find the new length of the visualization
      for (int i = 0; i < instance.num_of_agents; i++)
      {
        update_path(i, agents_paths[i]);
        max_path_len = std::max(max_path_len, (int)agents_paths[i].size() - 1);
      }

      // update the visualization
      solution_vis.new_data(max_path_len);
    }

    // process the events
    process_events(window, view);

    // clear the window
    window.clear(sf::Color::Black);

    // update imgui
    ImGui::SFML::Update(window, deltaClock.restart());

    // create docking space so that the GUI elements can be docked
    create_docking_space();

    // show the window for debugging imgui
#ifndef NDEBUG
    ImGui::ShowDemoWindow();
#endif

    // create the GUI elements
    create_left_panel();
    create_right_panel();
    create_bottom_panel();

    // create the menu bar
    create_menu_bar();

    // draw the map
    window.draw(map_sprite);

    // draw the path visualizations
    draw_paths(window);

    // draw the agents
    draw_goals(window);
    draw_agents(window);

    // set the view
    window.setView(view);

    // Render ImGui on top
    ImGui::SFML::Render(window);

    // display
    window.display();
  }

  // close the window
  if (window.isOpen()) window.close();
  ImGui::SFML::Shutdown();
  std::cout << "Visualizer thread ended" << std::endl;
}

// Render the map onto a texture
void Visualizer::mapToTexture(const Map& map_data)
{
  // initialize the texture
  sf::RenderTexture map_render_texture;
  map_render_texture.create(map_data.width * CELL_SIZE, map_data.height * CELL_SIZE);

  // clear the texture with a background color
  map_render_texture.clear(sf::Color::White);

  // Draw the grid onto the texture
  for (int y = 0; y < map_data.height; y++)
  {
    for (int x = 0; x < map_data.width; x++)
    {
      // create the cell
      sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
      cell.setPosition((float)x * CELL_SIZE, (float)y * CELL_SIZE);
      if ((int)map_data.data[y * map_data.width + x] == 1)
      {
        // obstacles
        cell.setFillColor(sf::Color::Black);
      }
      else
      {
        // free cells
        cell.setFillColor(sf::Color::White);
      }

      // grid color and thickness
      cell.setOutlineThickness(-BORDER_SIZE);
      cell.setOutlineColor(sf::Color::Black);

      map_render_texture.draw(cell);
    }
  }

  // Finalize the rendering
  map_render_texture.display();

  // set the map texture
  map_texture = map_render_texture.getTexture();

  // turn on antialiasing (makes the lines look like they have low resolution)
  // map_texture.setSmooth(true);

  // create the map sprite
  map_sprite = sf::Sprite(map_texture);
}
void Visualizer::update_agent_circles(const std::vector<Point2d>& positions)
{
  assertm((int)positions.size() == instance.num_of_agents, "Wrong number of agent positions.");

  // initialize the vector
  if (agents_circles.size() != positions.size())
  {
    agents_circles.clear();
    agents_circles.resize(positions.size(), sf::CircleShape(0));
  }

  // assign the correct circles
  for (int i = 0; i < (int)positions.size(); i++)
  {
    const int x = positions[i].x;
    const int y = positions[i].y;
    // create shape
    agents_circles[i] = sf::CircleShape((double)(CELL_SIZE - 2 * BORDER_SIZE) / 2);
    // set color
    agents_circles[i].setFillColor(agents_colors[i]);
    agents_circles[i].setOutlineThickness(0);
    // set position
    agents_circles[i].setPosition((float)x * CELL_SIZE + BORDER_SIZE, (float)y * CELL_SIZE + BORDER_SIZE);
  }
}

void Visualizer::draw_agents(sf::RenderWindow& window)
{
  assertm(agents_circles.size() > 0, "Trying to draw empty agent vector.");
  for (const auto& it : agents_circles)
  {
    // draw the shape
    window.draw(it);
  }
}

void Visualizer::update_goal_circles(const std::vector<Point2d>& positions)
{
  assertm((int)positions.size() == instance.num_of_agents, "Wrong number of agent positions.");

  // initialize the vector
  if (goal_circles.size() != positions.size())
  {
    goal_circles.clear();
    goal_circles.resize(positions.size(), sf::CircleShape(0));
  }

  // assign the correct circles
  for (int i = 0; i < (int)positions.size(); i++)
  {
    const int x = positions[i].x;
    const int y = positions[i].y;

    // create shape
    goal_circles[i] = sf::CircleShape((double)(CELL_SIZE - 2 * BORDER_SIZE) / 2);

    // set color
    goal_circles[i].setOutlineColor(agents_colors[i]);
    goal_circles[i].setOutlineThickness(-GOAL_VIS_THICKNESS);
    goal_circles[i].setFillColor(sf::Color::White);  // make the inner part white
                                                     //
    // set position
    goal_circles[i].setPosition((float)x * CELL_SIZE + BORDER_SIZE, (float)y * CELL_SIZE + BORDER_SIZE);
  }
}

void Visualizer::draw_goals(sf::RenderWindow& window)
{
  assertm(goal_circles.size() > 0, "Trying to draw empty goal vector.");
  for (const auto& it : goal_circles)
  {
    // draw the shape
    window.draw(it);
  }
}

void Visualizer::update_path(const int agent_num, const Path& path)
{
  assertm(agent_num >= 0 && agent_num < (int)agents_colors.size() && agent_num < instance.num_of_agents, "Invalid agent number.");

  // check whether the vector is big enough
  if (agent_num >= (int)agents_paths_vis.size())
  {
    agents_paths_vis.resize(agent_num + 1);
  }

  // find the color for this agent
  const sf::Color color = agents_colors[agent_num];

  // create the path visualization
  sf::VertexArray path_vis(sf::Quads);
  if (path.size() >= 2)
  {
    auto           segment_start = path.cbegin();
    auto           segment_end   = path.cbegin() + 1;
    enum Direction dir           = Direction::NONE;

    // merge contiguous segments
    for (int i = 2; i < (int)path.size() + 1; i++)
    {
      if (segment_start == segment_end || *segment_start == *segment_end)  // segment of length 0
      {
        segment_end++;
        continue;
      }
      // skip repeating points
      if (i < (int)path.size() && path[i] == *segment_end)
      {
        segment_end++;
        continue;
      }

      if (dir == Direction::NONE)
      {
        dir = find_direction(*segment_start, *segment_end);
      }

      assertm(dir != Direction::NONE, "Direction must be set.");

      // check whether segment continuous (trick with checking if i == path.size() is used to prevent out of bounds)
      if (i != (int)path.size() && dir == find_direction(*segment_end, path[i]))
      {
        // extend the end of the segment
        segment_end++;
        continue;
      }
      else
      {
        // find the rectangle that represents the segment
        int min_x, min_y, max_x, max_y;
        min_x = std::min(segment_start->x, segment_end->x);
        min_y = std::min(segment_start->y, segment_end->y);
        max_x = std::max(segment_start->x, segment_end->x);
        max_y = std::max(segment_start->y, segment_end->y);
        assertm(max_x - min_x > 0 != max_y - min_y > 0, "Segment must be either horizontal or vertical.");

        // calculate the corners of the rectangle
        sf::Vector2f left_upper  = sf::Vector2f(CELL_SIZE * (min_x + 0.5 - (double)1 / (2 * LINE_WIDTH)),
                                               CELL_SIZE * (min_y + 0.5 - (double)1 / (2 * LINE_WIDTH)));
        sf::Vector2f right_upper = sf::Vector2f(CELL_SIZE * (max_x + 0.5 + (double)1 / (2 * LINE_WIDTH)),
                                                CELL_SIZE * (min_y + 0.5 - (double)1 / (2 * LINE_WIDTH)));
        sf::Vector2f right_lower = sf::Vector2f(CELL_SIZE * (max_x + 0.5 + (double)1 / (2 * LINE_WIDTH)),
                                                CELL_SIZE * (max_y + 0.5 + (double)1 / (2 * LINE_WIDTH)));
        sf::Vector2f left_lower  = sf::Vector2f(CELL_SIZE * (min_x + 0.5 - (double)1 / (2 * LINE_WIDTH)),
                                               CELL_SIZE * (max_y + 0.5 + (double)1 / (2 * LINE_WIDTH)));

        // add the segment into the path visualization
        path_vis.append(sf::Vertex(left_upper, color));
        path_vis.append(sf::Vertex(right_upper, color));
        path_vis.append(sf::Vertex(right_lower, color));
        path_vis.append(sf::Vertex(left_lower, color));

        // start a new segment
        segment_start = segment_end;
        segment_end++;
        dir = find_direction(*segment_start, *segment_end);
      }
    }
  }
  assertm(agent_num < (int)agents_paths_vis.size(), "Agent_num must be bigger than the size of the paths visualization vector.");
  agents_paths_vis[agent_num] = path_vis;
}

void Visualizer::update_all_paths(int time)
{
  for (int i = 0; i < (int)agents_paths.size(); i++)
  {
    // calculate the start index of the path according to the visualization time
    int idx = std::min((int)agents_paths[i].size() - 1, time);
    update_path(i, Path(agents_paths[i].begin() + idx, agents_paths[i].end()));
  }
}

void Visualizer::draw_paths(sf::RenderWindow& window)
{
  for (auto& it : agents_paths_vis)
  {
    if (it.getVertexCount() == 0) continue;
    window.draw(it);
  }
}

void Visualizer::create_docking_space(void)
{
  // get the viewport for the whole window
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);

  // set the flags for the gui overlay
  ImGuiWindowFlags host_window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                       ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

  // set the style
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  // create the dockspace
  ImGui::Begin("DockSpace Window", nullptr, host_window_flags);
  ImGui::PopStyleVar(3);

  // Get menu bar height
  float menuBarHeight = MENU_BAR_SIZE;
  // float menuBarHeight = ImGui::GetFrameHeight(); // does not work for some reason

  // Offset dockspace position to account for menu bar
  ImGui::SetCursorPosY(menuBarHeight);

  // Create the dockspace
  ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
  static bool initialized = false;
  if (!initialized)  // initialize the dockspace only once
  {
    initialized = true;

    // Clear old dock layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);

    // Adjust dock builder size based on menu bar height
    ImVec2 dockspaceSize = ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight);
    ImGui::DockBuilderSetNodeSize(dockspace_id, dockspaceSize);

    // Create dock spaces
    ImGuiID dock_main_id = dockspace_id;  // Main area

    // split bottom
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, BOTTOM_PANEL_SIZE_DEFAULT, nullptr, &dock_main_id);

    // split sides
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, LEFT_PANEL_SIZE_DEFAULT, nullptr, &dock_main_id);

    // calculate the right panel size taking into account the left panel
    const double right_panel_size = (float)RIGHT_PANEL_SIZE_DEFAULT / (1.0 - LEFT_PANEL_SIZE_DEFAULT);
    ImGuiID      dock_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, (float)right_panel_size, nullptr, &dock_main_id);

    // Create the dock windows
    ImGui::DockBuilderDockWindow("Left Panel", dock_left);
    ImGui::DockBuilderDockWindow("Right Panel", dock_right);
    ImGui::DockBuilderDockWindow(ICON_FA_CODE " Console", dock_bottom);

    // Finalize
    ImGui::DockBuilderFinish(dockspace_id);
  }
  ImGui::End();
}

void Visualizer::load_fonts(void)
{
  // Clear old fonts
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->Clear();

  // load the default font
  io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/Cousine-Regular.ttf").c_str(), 20.0f);
  // io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/DroidSans.ttf").c_str(), 20.0f);

  // check, whether the load was successful
  if (!ImGui::SFML::UpdateFontTexture())
  {
    std::cout << "WARNING: Could not load the default font. " << std::endl;
    io.Fonts->AddFontDefault();
  }

  // Load FontAwesome font for icons
  static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};  // Unicode range for icons

  ImFontConfig config;
  config.MergeMode        = true;       // Merge with the default font
  config.PixelSnapH       = true;       // Align to pixels for sharp rendering
  config.GlyphMinAdvanceX = ICON_SIZE;  // make the icon monospaced

  // if (!io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/forkawesome-webfont.ttf").c_str(), 13.0f, &config, icons_ranges))
  if (!io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/fa-solid-900.ttf").c_str(), ICON_SIZE, &config, icons_ranges))
  {
    std::cout << "WARNING: Unable to load icons." << std::endl;
  };

  if (!ImGui::SFML::UpdateFontTexture()) std::cout << "WARNING: Could not update the icon font. " << std::endl;
}

void Visualizer::adjust_sfml_part_viewport(sf::RenderWindow& window, sf::View& view, const float left_panel_size,
                                           const float right_panel_size, const float bottom_panel_size)
{
  // calculate the map parameters
  const int    map_width        = instance.get_map_data().width;
  const int    map_height       = instance.get_map_data().height;
  const double map_aspect_ratio = (double)map_width / (double)map_height;

  // get the window parameters
  const int window_width  = window.getSize().x;
  const int window_height = window.getSize().y;

  // calculate the size of the part of the window where the sfml stuff will be rendered
  const double sfml_part_width        = window_width * (1.0 - left_panel_size - right_panel_size);
  const double sfml_part_height       = window_height * (1.0 - bottom_panel_size);
  const double sfml_part_aspect_ratio = sfml_part_width / sfml_part_height;
  double       viewport_width, viewport_height;

  // the viewport must have the same aspoect ratio as the map
  if (map_aspect_ratio < sfml_part_aspect_ratio)
  {
    // map is wider
    viewport_width  = sfml_part_width;
    viewport_height = viewport_width / map_aspect_ratio;
  }
  else
  {
    // map is taller
    viewport_height = sfml_part_height;
    viewport_width  = viewport_height * map_aspect_ratio;
  }

  // calculate the viewport size relative to the window size
  const float viewport_width_rel  = (float)(viewport_width / sfml_part_width);
  const float viewport_height_rel = (float)(viewport_height / sfml_part_height);
  const float offset_x            = (1 - viewport_width_rel) / 2;
  const float offset_y            = (1 - viewport_height_rel) / 2;
  view.setViewport(sf::FloatRect({left_panel_size + offset_x, offset_y},
                                 {(float)(viewport_width / window_width), (float)(viewport_height / window_height)}));
}

void Visualizer::process_events(sf::RenderWindow& window, sf::View& view)
{
  // poll all events from the window
  sf::Event event;
  while (window.pollEvent(event))
  {
    ImGui::SFML::ProcessEvent(window, event);  // Send event to ImGui

    // close event
    if (event.type == sf::Event::Closed)
    {
      running = false;
    }
    // zoom event
    else if (event.type == sf::Event::MouseWheelScrolled)
    {
      if (event.mouseWheelScroll.delta > 0)
      {
        view.zoom(1 / zoom_step);  // Zoom in
        zoom_level *= 1 / zoom_step;
      }
      else
      {
        view.zoom(zoom_step);  // Zoom out
        zoom_level *= zoom_step;
      }
    }
    // mouse click
    else if (event.type == sf::Event::MouseButtonPressed)
    {
      if (event.mouseButton.button == sf::Mouse::Left)
      {
        const sf::Vector2i mouse_pos      = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
        const sf::Vector2f translated_pos = window.mapPixelToCoords(mouse_pos);
        for (int i = 0; i < (int)agents_circles.size(); i++)
        {
          // check whether clicked on agent
          if (agents_circles[i].getGlobalBounds().contains(translated_pos))
          {
            selected_agent = i;
            return;
          }

          // clicked on the map
          is_moving      = true;
          last_click_pos = mouse_pos;
        }
      }
    }
    // mouse is pressed
    else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
      if (is_moving)
      {
        const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
        const sf::Vector2i delta     = last_click_pos - mouse_pos;
        // get viewport size
        const float viewport_width  = view.getViewport().width;
        const float viewport_height = view.getViewport().height;
        const float view_width      = view.getSize().x;
        const float view_height     = view.getSize().y;
        const int   window_width    = window.getSize().x;
        const int   window_height   = window.getSize().y;
        view.move(view_width * delta.x / ((float)window_width * viewport_width),
                  view_height * delta.y / ((float)window_height * viewport_height));
        last_click_pos = mouse_pos;
      }
    }
    else if (event.type == sf::Event::MouseButtonReleased)
    {
      is_moving = false;
    }
    // Panning with arrow keys
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
      view.move(-CELL_SIZE * zoom_level, 0);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
      view.move(CELL_SIZE * zoom_level, 0);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
      view.move(0, -CELL_SIZE * zoom_level);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
      view.move(0, CELL_SIZE * zoom_level);
    }
  }
}

void Visualizer::create_left_panel(void)
{
  ImGui::Begin("Left Panel", nullptr, ImGuiWindowFlags_NoResize);
  ImGui::Text("Agent info:");
  if (selected_agent != -1) ImGui::Text("Selected agent: %d", selected_agent);
  if ((int)sipp_info.size() > 0)
  {
    ImGui::Text("Sipp iterations: %d", sipp_info[0].back().iteration_num);
  }
  ImGui::End();
}

void Visualizer::create_right_panel(void)
{
  ImGui::Begin("Right Panel", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::Text("Visualization:");

  // Set button background color to transparent
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));

  // calculate the width of the slider label
  const float time_text_width = ImGui::CalcTextSize("Time: ").x + ImGui::GetStyle().ItemSpacing.x;

  // Get the width of the remaining space in the panel after adding the slider label
  const float panel_size      = ImGui::GetContentRegionAvail().x;
  float       available_width = panel_size - time_text_width;

  // create the slider
  ImGui::Text("Time: ");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(available_width);
  if (ImGui::SliderInt("##time_slider", &solution_vis.time, 0,
                       solution_vis.max_time))  // ##time_slider means that the slider name is not displayed
  {
    // slider changed
    solution_vis.update();
  }

  // calculate the width of button array
  const int num_buttons = 5;
  float     total_width = num_buttons * ImGui::CalcTextSize(ICON_FA_PLAY).x + 2 * num_buttons * ImGui::GetStyle().FramePadding.x +
                      (num_buttons - 1) * ImGui::GetStyle().ItemSpacing.x;

  // center the playback buttons
  float start_x = time_text_width + (available_width - total_width) * 0.5f;
  if (start_x + total_width > panel_size) start_x = panel_size - total_width;
  ImGui::SetCursorPosX(start_x);

  if (ImGui::Button(ICON_FA_PLAY))
  {
    solution_vis.start();
  }
  ImGui::SameLine();

  if (ImGui::Button(ICON_FA_STOP))
  {
    solution_vis.reset();
  }
  ImGui::SameLine();

  if (ImGui::Button(ICON_FA_STEP_BACKWARD))
  {
    solution_vis.step_backward();
  }
  ImGui::SameLine();

  if (ImGui::Button(ICON_FA_PAUSE))
  {
    solution_vis.pause();
  }
  ImGui::SameLine();

  if (ImGui::Button(ICON_FA_STEP_FORWARD))
  {
    solution_vis.step_forward();
  }

  ImGui::PopStyleColor(1);

  ImGui::End();
}


void Visualizer::create_bottom_panel(void)
{
  ImGui::Begin(ICON_FA_CODE " Console");
  ImGui::Text("Solver output");
  ImGui::End();
}

void Visualizer::create_menu_bar(void)
{
  // Set style
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));       // Increase padding (width, height)
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));      // Adjust window padding
  ImGui::GetStyle().WindowBorderSize        = 2.0f;                      // Add border
  ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.1, 0.1, 0.1, 1);  // border

  // create the widget
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("Instance"))
    {
      if (ImGui::MenuItem("Load map")) std::cout << "Loading a map." << std::endl;
      if (ImGui::MenuItem("Load scene")) std::cout << "Loading a scene." << std::endl;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Settings"))
    {
      if (ImGui::MenuItem("Algorithm settings")) std::cout << "Algorithm settings." << std::endl;
      ImGui::EndMenu();
    }
    // ImVec2 menu_bar_size = ImGui::GetWindowSize();

    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleVar(2);  // Pop both styles
}

std::vector<Point2d> Visualizer::get_agent_positions(int time)
{
  std::vector<Point2d> agent_positions;

  // find the current position for each agent
  for (int i = 0; i < (int)agents_paths.size(); i++)
  {
    const int idx = std::min((int)agents_paths[i].size() - 1, time);
    agent_positions.push_back(agents_paths[i][idx]);
  }

  return agent_positions;
}

void Visualizer::solution_visualization_update(const Visualization& vis)
{
  update_all_paths(vis.time);
  update_agent_circles(get_agent_positions(vis.time));
}


void Visualizer::sipp_visualization_update(const Visualization& vis)
{
}

Visualization::Visualization(std::function<void(Visualization& vis)> update_func) : update_func(update_func)
{
}

void Visualization::reset(void)
{
  is_playing        = false;
  time              = 0;
  iteration_counter = 0;
  update();
}

void Visualization::start(void)
{
  if (time >= max_time) reset();
  is_playing = true;
}

void Visualization::pause(void)
{
  is_playing = false;
}

void Visualization::update(void)
{
  update_func(*this);
}

void Visualization::step_forward(void)
{
  if (time < max_time)
  {
    time++;
    update();
  }
}


void Visualization::step_backward(void)
{
  if (time > 0)
  {
    time--;
    update();
  }
}

void Visualization::new_data(int new_len)
{
  // reset the visualization time
  time              = 0;
  iteration_counter = 0;
  max_time          = new_len;
}

void Visualization::next_iteration(void)
{
  iteration_counter++;

  // update the iteration time
  if ((int)std::round((float)iteration_counter * speed) % FRAME_RATE == 0)
  {
    iteration_counter = 0;
    time++;
    if (time >= max_time)
    {
      pause();
    }
    update();
  }
}

