#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <iostream>
#include <cmath>
#include "Visualizer.h"
#include "utils.h"

#define WINDOW_WIDTH 800
#define CELL_SIZE 20
#define BORDER_SIZE 1

Visualizer::Visualizer(Instance& instance)
{
  running = false;
  map_width = instance.num_of_cols;
  map_height = instance.num_of_rows;
  start_positions = instance.get_start_positions();
  goal_positions = instance.get_goal_positions();

  // generate random colors for the agents
  for (int i = 0; i < (int) start_positions.size(); i++)
  {
    agents_colors.push_back(generate_random_color(255, 255, 255, 0.25));
  }

  // create the map texture
  mapToTexture(instance.get_map_data(), instance.num_of_rows, instance.num_of_cols);
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
  running = true;
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
  int window_height = std::round(WINDOW_WIDTH * (map_height / map_width));

  // turn on antialiasing
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8.0;

  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, window_height), "Visualization", sf::Style::Default, settings);

  // set framerate
  window.setFramerateLimit(60);

  // Activate the window for OpenGL rendering
  window.setActive();

  // create a view
  sf::View view(sf::FloatRect(0, 0, WINDOW_WIDTH, window_height));

  // Zoom settings
  float zoomLevel = 1.0f;
  const float zoomStep = 1.1f;

  while (window.isOpen() && running)
  {
    // poll all events from the window
    sf::Event event;
    while (window.pollEvent(event)) 
    {
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
          view.zoom(1 / zoomStep); // Zoom in
          zoomLevel *= 1 / zoomStep;
        } 
        else 
        {
          view.zoom(zoomStep); // Zoom out
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

    // draw the map
    window.draw(map_sprite);

    // draw the agents
    draw_agents(window);

    // set the view 
    window.setView(view);

    // display 
    window.display();
  }

  // close the window
  if (window.isOpen()) window.close();
  std::cout << "Visualizer thread ended" << std::endl;
}

// Render the map onto a texture
void Visualizer::mapToTexture(const std::vector<std::uint8_t>& map_data, int num_of_rows, int num_of_columns) 
{
  // initialize the texture
  sf::RenderTexture map_render_texture;
  map_render_texture.create(num_of_columns * CELL_SIZE, num_of_rows * CELL_SIZE);

  // clear the texture with a background color
  map_render_texture.clear(sf::Color::White);

  // Draw the grid onto the texture
  for (int y = 0; y < num_of_rows; y++) 
  {
    for (int x = 0; x < num_of_columns; x++) 
    {
      // create the cell 
      sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
      cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
      if ((int) map_data[y*num_of_columns + x] == 1) 
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
    scale = (double) WINDOW_WIDTH / (CELL_SIZE*num_of_columns);
    map_sprite.setScale(sf::Vector2f(scale, scale));
}

void Visualizer::draw_agents(sf::RenderWindow& window)
{
  agents_circles.clear();
  for (int i = 0; i < (int) start_positions.size(); i++)
  {
    int x = std::get<0>(start_positions[i]);
    int y = std::get<1>(start_positions[i]);
    // create shape
    agents_circles.push_back(sf::CircleShape((double) (CELL_SIZE - 2*BORDER_SIZE)/2));
    // set color
    agents_circles.back().setFillColor(agents_colors[i]);
    // set position
    agents_circles.back().setPosition(x*scale*CELL_SIZE+scale*BORDER_SIZE, y*scale*CELL_SIZE+scale*BORDER_SIZE);
    agents_circles.back().setScale(sf::Vector2f(scale, scale));
    // draw the shape 
    window.draw(agents_circles.back());
  }
}
