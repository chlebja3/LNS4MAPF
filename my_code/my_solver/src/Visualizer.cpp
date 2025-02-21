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
#define LEFT_PANEL_SIZE 0.2
#define RIGHT_PANEL_SIZE 0.2
#define BOTTOM_PANEL_SIZE 0.2
#define WINDOW_WIDTH 1920

#define CELL_SIZE 20
#define BORDER_SIZE 1
#define GOAL_VIS_THICKNESS 3
#define LINE_WIDTH 3  // denominator of line width, so line width is 1/LINE_WIDTH * CELL_SIZE
#define MENU_BAR_SIZE 40

Visualizer::Visualizer(const Instance& instance, SharedData& shared_data) : instance(instance), shared_data(shared_data)
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

  // std::cout << "Instance initialized: " << instance.is_initialized() << std::endl;
  // generate the paths
  // for (int i = 0; i < (int)instance.get_optimal_paths().size(); i++)
  // {
  //   update_path(i, instance.get_optimal_paths()[i]);
  // }
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
  const int    screen_width  = (int)sf::VideoMode::getDesktopMode().width;
  const int    screen_height = (int)sf::VideoMode::getDesktopMode().height;
  const int    map_width     = instance.get_map_data().width;
  const int    map_height    = instance.get_map_data().height;
  const double aspect_ratio  = (double)map_width / (double)map_height;
  // std::cout << "Requested window height " << window_height << std::endl;

  // turn on antialiasing
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8.0;


  sf::RenderWindow window(sf::VideoMode(screen_width, screen_height), "Visualization", sf::Style::Default, settings);
  const int        window_width        = window.getSize().x;
  const int        window_height       = window.getSize().y;
  const double     window_aspect_ratio = (double)window_width / (double)window_height;
  // std::cout << "Window size: " << window_width << " " << window_height << std::endl;

  // set framerate
  window.setFramerateLimit(60);

  if (!ImGui::SFML::Init(window)) throw std::runtime_error("Unable to initialize ImGUI.");

  // Activate the window for OpenGL rendering
  window.setActive();


  // std::cout << "Map size: " << map_sprite.getGlobalBounds().left << " " << map_sprite.getGlobalBounds().top << " "
  //          << map_sprite.getGlobalBounds().width << " " << map_sprite.getGlobalBounds().height << std::endl;

  // create a view
  // sf::View view(sf::FloatRect(0, 0, window_width, window_height));
  sf::View view(sf::Vector2f((float)map_width * CELL_SIZE / 2, (float)map_height * CELL_SIZE / 2),
                sf::Vector2f((float)map_width * CELL_SIZE, (float)map_height * CELL_SIZE));

  // view size
  // std::cout << "View size: " << view.getSize().x << " " << view.getSize().y << std::endl;

  const double sfml_part_width        = window_width * (1.0 - LEFT_PANEL_SIZE - RIGHT_PANEL_SIZE);
  const double sfml_part_height       = window_height * (1.0 - BOTTOM_PANEL_SIZE);
  const double sfml_part_aspect_ratio = sfml_part_width / sfml_part_height;
  double       viewport_width, viewport_height;

  // std::cout << "sfml width: " << sfml_part_width << " height: " << sfml_part_height << " aspect ratio: " << sfml_part_aspect_ratio
  //         << std::endl;
  if (aspect_ratio < sfml_part_aspect_ratio)
  {
    // map is wider
    viewport_width  = sfml_part_width;
    viewport_height = viewport_width / aspect_ratio;
  }
  else
  {
    // map is taller
    viewport_height = sfml_part_height;
    viewport_width  = viewport_height * aspect_ratio;
  }
  view.setViewport(
      sf::FloatRect({LEFT_PANEL_SIZE, 0.0f}, {(float)(viewport_width / window_width), (float)(viewport_height / window_height)}));

  // Zoom settings
  float       zoomLevel = 1.0;
  const float zoomStep  = 1.1f;
  // view.zoom(zoomLevel);


  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->Clear();  // Clear old fonts
                      // load fonts
  io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/Cousine-Regular.ttf").c_str(), 20.0f);
  // io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/DroidSans.ttf").c_str(), 20.0f);
  if (!ImGui::SFML::UpdateFontTexture()) std::cout << "Could not load the default font. " << std::endl;

  // load fontawesome for icons
  static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};  // Unicode range for icons
                                                                        //
  // Load FontAwesome font
  ImFontConfig config;
  // config.MergeMode = true;  // Merge with default font
  // config.PixelSnapH       = true;   // Align to pixels for sharp rendering
  // config.GlyphMinAdvanceX = 13.0f;  // Use if you want to make the icon monospaced

  // if (!io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/forkawesome-webfont.ttf").c_str(), 13.0f, &config, icons_ranges))
  if (!io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/fa-solid-900.ttf").c_str(), 13.0f, &config, icons_ranges))
  {
    std::cout << "WARNING: Unable to load icons." << std::endl;
  };
  if (!ImGui::SFML::UpdateFontTexture()) std::cout << "Could not update the icon font. " << std::endl;

  std::cout << "Total fonts loaded: " << io.Fonts->Fonts.Size << std::endl;
  for (int i = 0; i < io.Fonts->Fonts.Size; i++)
  {
    std::cout << "Font " << i << ": " << io.Fonts->Fonts[i]->ConfigData->Name << std::endl;
  }


  // enable docking
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  sf::Clock deltaClock;
  while (window.isOpen() && running)
  {
    // check whether new paths arrived
    if (shared_data.are_new_paths.load(std::memory_order_acquire))
    {
      std::vector<Path> new_paths = shared_data.consume_paths();
      assertm(instance.num_of_agents == new_paths.size(), "Number of paths should be the same as the number of agents.");

      // construct the path visualizations
      for (int i = 0; i < instance.num_of_agents; i++)
      {
        update_path(i, new_paths[i]);
      }
    }


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
          view.zoom(1 / zoomStep);  // Zoom in
          zoomLevel *= 1 / zoomStep;
        }
        else
        {
          view.zoom(zoomStep);  // Zoom out
          zoomLevel *= zoomStep;
        }
      }
      // Panning with arrow keys
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
      {
        view.move(-10 * zoomLevel, 0);
      }
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
      {
        view.move(10 * zoomLevel, 0);
      }
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
      {
        view.move(0, -10 * zoomLevel);
      }
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
      {
        view.move(0, 10 * zoomLevel);
      }
    }


    // clear the window
    window.clear(sf::Color::Black);

    // update imgui
    ImGui::SFML::Update(window, deltaClock.restart());

    // Setup Docking Space
    ShowDockingSpace();

    ImGui::ShowDemoWindow();
    // Left Panel
    ImGui::Begin("Left Panel", nullptr, ImGuiWindowFlags_NoResize);
    ImGui::Text("Tools, Layers, or Options here...");

    ImVec2 leftSize = ImGui::GetWindowSize();
    ImGui::PushFont(io.Fonts->Fonts[1]);
    ImGui::Text(ICON_FA_SEARCH);
    if (ImGui::Button(ICON_FA_BRUSH))
    { /* do something */
    }
    ImGui::PopFont();
    ImGui::End();

    // Right Panel
    ImGui::Begin("Right Panel", nullptr, ImGuiWindowFlags_NoResize);
    ImGui::Text("Properties, Debug Info, etc.");
    ImVec2 rightSize = ImGui::GetWindowSize();
    ImGui::Text("Right Panel Size: %.2f, %.2f", rightSize.x, rightSize.y);
    ImGui::End();

    // Right Panel
    ImGui::Begin("Console");
    ImGui::Text("Solver output");
    ImGui::End();

    // draw the map
    window.draw(map_sprite);

    // draw the optimal paths
    draw_paths(window);

    // draw the agents
    draw_agents(window);
    draw_goals(window);

    // set the view
    window.setView(view);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));       // Increase padding (width, height)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));      // Adjust window padding
    ImGui::GetStyle().WindowBorderSize        = 2.0f;                      // Add border
    ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.1, 0.1, 0.1, 1);  // border

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
      ImVec2 menu_bar_size = ImGui::GetWindowSize();
      // set the are where the map and agents will be rendered
      // view.setViewport(sf::FloatRect({0.f, (float)menu_bar_size.y / (float)window_height}, {1.0f, 1.f}));

      ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleVar(2);  // Pop both styles

    // **ImGui UI Overlay**
    // ImGui::SetNextWindowPos(ImVec2(0, 460));  // Position below SFML map
    // ImGui::SetNextWindowSize(ImVec2(800, 140));
    // ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoResize);

    // if (ImGui::Button("Run A* Solver"))
    //{
    //  std::cout << "Running A*..." << std::endl;
    //  // TODO: Trigger A* algorithm
    //}

    // if (ImGui::Button("Load Instance"))
    //{
    //  std::cout << "Loading new instance..." << std::endl;
    //  // TODO: Load new instance
    //}

    // ImGui::End();

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
  // turn on antialiasing
  // map_texture.setSmooth(true);
  map_sprite = sf::Sprite(map_texture);
  // scale      = (double)WINDOW_WIDTH / (CELL_SIZE * map_data.width);
  // map_sprite.setScale(sf::Vector2f(scale, scale));
}
void Visualizer::update_agent_circles(const std::vector<Point2d>& positions)
{
  assertm(positions.size() == instance.num_of_agents, "Wrong number of agent positions.");

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
    agents_circles[i].setPosition(x * CELL_SIZE + BORDER_SIZE, y * CELL_SIZE + BORDER_SIZE);
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
  assertm(positions.size() == instance.num_of_agents, "Wrong number of agent positions.");

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
    // set position
    goal_circles[i].setPosition(x * CELL_SIZE + BORDER_SIZE, y * CELL_SIZE + BORDER_SIZE);
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
  // std::cout << "Updating path: " << agent_num << std::endl;
  assertm(agent_num >= 0 && agent_num < agents_colors.size() && agent_num < instance.num_of_agents, "Invalid agent number.");
  if (agent_num >= (int)agents_paths.size())
  {
    agents_paths.resize(agent_num + 1);
  }
  const sf::Color color = agents_colors[agent_num];
  sf::VertexArray path_vis(sf::Quads);
  if (path.size() >= 2)
  {
    auto           segment_start = path.cbegin();
    auto           segment_end   = path.cbegin() + 1;
    enum Direction dir           = Direction::NONE;

    // merge contiguous segments
    for (int i = 2; i < (int)path.size() + 1; i++)
    {
      if (segment_start == segment_end)  // segment of length 0
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
        int min_x, min_y, max_x, max_y;
        min_x = std::min(segment_start->x, segment_end->x);
        min_y = std::min(segment_start->y, segment_end->y);
        max_x = std::max(segment_start->x, segment_end->x);
        max_y = std::max(segment_start->y, segment_end->y);
        assertm(max_x - min_x > 0 != max_y - min_y > 0, "Segment must be either horizontal or vertical.");


        sf::Vector2f left_upper  = sf::Vector2f(CELL_SIZE * (min_x + 0.5 - (double)1 / (2 * LINE_WIDTH)),
                                               CELL_SIZE * (min_y + 0.5 - (double)1 / (2 * LINE_WIDTH)));
        sf::Vector2f right_upper = sf::Vector2f(CELL_SIZE * (max_x + 0.5 + (double)1 / (2 * LINE_WIDTH)),
                                                CELL_SIZE * (min_y + 0.5 - (double)1 / (2 * LINE_WIDTH)));
        sf::Vector2f right_lower = sf::Vector2f(CELL_SIZE * (max_x + 0.5 + (double)1 / (2 * LINE_WIDTH)),
                                                CELL_SIZE * (max_y + 0.5 + (double)1 / (2 * LINE_WIDTH)));
        sf::Vector2f left_lower  = sf::Vector2f(CELL_SIZE * (min_x + 0.5 - (double)1 / (2 * LINE_WIDTH)),
                                               CELL_SIZE * (max_y + 0.5 + (double)1 / (2 * LINE_WIDTH)));

        // add previous segment
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
  assertm(agent_num < agents_paths.size(), "Agent_num must be bigger than the size of the paths vector.");
  agents_paths[agent_num] = path_vis;
}

void Visualizer::draw_paths(sf::RenderWindow& window)
{
  for (auto& it : agents_paths)
  {
    if (it.getVertexCount() == 0) continue;
    window.draw(it);
  }
}

void Visualizer::ShowDockingSpace()
{
  // get the viewport
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  // std::cout << "Viewport imgui size: " << viewport->Size.x << " " << viewport->Size.y << std::endl;

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
  if (!initialized)  // initialize the dockspace
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
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, BOTTOM_PANEL_SIZE, nullptr, &dock_main_id);

    // split sides
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, LEFT_PANEL_SIZE, nullptr, &dock_main_id);
    // calculate the right panel size taking into account the left panel
    const double right_panel_size = (float)RIGHT_PANEL_SIZE / (1.0 - LEFT_PANEL_SIZE);
    ImGuiID      dock_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, (float)right_panel_size, nullptr, &dock_main_id);


    // Create the dock windows
    ImGui::DockBuilderDockWindow("Left Panel", dock_left);
    ImGui::DockBuilderDockWindow("Right Panel", dock_right);
    ImGui::DockBuilderDockWindow("Console", dock_bottom);

    // Finalize
    ImGui::DockBuilderFinish(dockspace_id);
  }
  ImGui::End();
}
