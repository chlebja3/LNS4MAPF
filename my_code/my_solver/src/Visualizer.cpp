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

#include "imgui-SFML.h"
#include "imgui.h"
#include "utils.h"

#define WINDOW_WIDTH 800
#define CELL_SIZE 20
#define BORDER_SIZE 1
#define GOAL_VIS_THICKNESS 3
#define LINE_WIDTH 3  // denominator od line width, so line width is 1/LINE_WIDTH * CELL_SIZE
#define MENU_BAR_SIZE 46


Visualizer::Visualizer(std::shared_ptr<Instance> instance) : instance(instance)
{
  running = false;

  // generate random colors for the agents
  for (int i = 0; i < (int)instance->get_start_positions().size(); i++)
  { agents_colors.push_back(generate_random_color(255, 255, 255, 0.25)); }

  // create the map texture
  mapToTexture(instance->get_map_data());
  std::cout << "Initialized visualizer." << std::endl;
}

Visualizer::~Visualizer()
{
  stop();
  join_thread();
}

void Visualizer::start()
{
  // start the underlying thread
  vis_thread = std::thread(&Visualizer::run, this);
  running    = true;
}

void Visualizer::stop()
{
  // set the stop signal
  running = false;
}

void Visualizer::join_thread()
{
  if (vis_thread.joinable()) { vis_thread.join(); }
}

void Visualizer::run()
{
  const int map_width     = instance->get_map_data().width;
  const int map_height    = instance->get_map_data().height;
  const int window_height = (int)std::round(WINDOW_WIDTH * ((double)map_width / (double)map_height)) + MENU_BAR_SIZE;
  // std::cout << "Requested window height " << window_height << std::endl;

  // turn on antialiasing
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8.0;


  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, window_height), "Visualization", sf::Style::Default, settings);

  // set framerate
  window.setFramerateLimit(60);

  if (!ImGui::SFML::Init(window)) throw std::runtime_error("Unable to initialize ImGUI.");

  // Activate the window for OpenGL rendering
  window.setActive();

  // create a view
  sf::View view(sf::FloatRect(0, 0, WINDOW_WIDTH, window_height));

  // Zoom settings
  float       zoomLevel = 1.0f;
  const float zoomStep  = 1.1f;


  // turn on antialisasing for fonts
  // ImGuiIO& io = ImGui::GetIO();
  // io.Fonts->Clear();                                             // Clear old fonts
  // std::cout << (get_base_path() + "/fonts/timeburner-font/Timeburner-xJB8.ttf" ).c_str() << std::endl;
  // io.Fonts->AddFontFromFileTTF((get_base_path() + "/fonts/timeburner-font/Timeburner-xJB8.ttf" ).c_str(), 18.0f, NULL,
  // io.Fonts->GetGlyphRangesDefault()); // Load font with size 18 io.Fonts->Build(); io.Fonts->TexGlyphPadding = 1;  // Improves quality
  ImGui::GetIO().FontGlobalScale = 2.0f;  // Increase all UI elements

  sf::Clock deltaClock;
  while (window.isOpen() && running)
  {
    // poll all events from the window
    sf::Event event;
    while (window.pollEvent(event))
    {
      ImGui::SFML::ProcessEvent(window, event);  // Send event to ImGui

      // close event
      if (event.type == sf::Event::Closed) { running = false; }
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

    // draw the map
    window.draw(map_sprite);

    // draw the optimal paths
    // for (int i = 0; i < (int) optimal_paths.size(); i++)
    //{
    //  draw_path(window, optimal_paths[i], agents_colors[i]);
    //}
    draw_path(window, instance->get_optimal_paths()[0], agents_colors[0]);

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
      view.setViewport(sf::FloatRect({0.f, (float)menu_bar_size.y / (float)window_height}, {1.0f, 1.f}));

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
  scale      = (double)WINDOW_WIDTH / (CELL_SIZE * map_data.width);
  map_sprite.setScale(sf::Vector2f(scale, scale));
}

void Visualizer::draw_agents(sf::RenderWindow& window)
{
  agents_circles.clear();
  for (int i = 0; i < (int)instance->get_start_positions().size(); i++)
  {
    const int x = instance->get_start_positions()[i].x;
    const int y = instance->get_start_positions()[i].y;
    // create shape
    agents_circles.push_back(sf::CircleShape((double)(CELL_SIZE - 2 * BORDER_SIZE) / 2));
    // set color
    agents_circles.back().setFillColor(agents_colors[i]);
    // set position
    agents_circles.back().setPosition(x * scale * CELL_SIZE + scale * BORDER_SIZE, y * scale * CELL_SIZE + scale * BORDER_SIZE);
    agents_circles.back().setScale(sf::Vector2f(scale, scale));
    // draw the shape
    window.draw(agents_circles.back());
  }
}

void Visualizer::draw_goals(sf::RenderWindow& window)
{
  agents_circles.clear();
  for (int i = 0; i < (int)instance->get_start_positions().size(); i++)
  {
    const int x = instance->get_goal_positions()[i].x;
    const int y = instance->get_goal_positions()[i].y;
    // create shape
    agents_circles.push_back(sf::CircleShape((double)(CELL_SIZE - 2 * BORDER_SIZE) / 2));
    // set color
    agents_circles.back().setOutlineColor(agents_colors[i]);
    agents_circles.back().setOutlineThickness(-GOAL_VIS_THICKNESS);
    // set position
    agents_circles.back().setPosition((float)(x * scale * CELL_SIZE + scale * BORDER_SIZE),
                                      (float)(y * scale * CELL_SIZE + scale * BORDER_SIZE));
    agents_circles.back().setScale(sf::Vector2f(scale, scale));
    // draw the shape
    window.draw(agents_circles.back());
  }
}

void Visualizer::draw_arrow(sf::RenderWindow& window, const Point2d& start, const Point2d& end, const sf::Color& color)
{
  const double x = (end.x + start.x) / 2.;
  const double y = (end.y + start.y) / 2.;

  // draw the line
  sf::Vector2f       shape;
  sf::RectangleShape line;
  if (start.x == end.x)  // vertical
  {
    shape = sf::Vector2f(CELL_SIZE / (float)LINE_WIDTH, CELL_SIZE + (float)CELL_SIZE / LINE_WIDTH);
    line  = sf::RectangleShape(shape);
    line.setPosition((x + (float)1 / LINE_WIDTH) * scale * CELL_SIZE, (y - (float)1 / (2 * LINE_WIDTH)) * scale * CELL_SIZE);
  }
  else  // horizontal
  {
    shape = sf::Vector2f(CELL_SIZE + (float)CELL_SIZE / LINE_WIDTH, CELL_SIZE / (float)LINE_WIDTH);
    line  = sf::RectangleShape(shape);
    line.setPosition((x - (float)1 / (2 * LINE_WIDTH)) * scale * CELL_SIZE, (y + (float)1 / LINE_WIDTH) * scale * CELL_SIZE);
  }
  line.setScale(sf::Vector2f(scale, scale));
  line.setFillColor(color);
  window.draw(line);
}

void Visualizer::draw_path(sf::RenderWindow& window, const Path& path, const sf::Color& color)
{
  for (int i = 1; i < (int)path.size(); i++) { draw_arrow(window, path[i - 1], path[i], color); }
}
