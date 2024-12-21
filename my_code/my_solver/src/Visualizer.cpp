#include <SFML/Graphics.hpp>
#include <iostream>
#include "Visualizer.h"

#define CELL_SIZE 20

Visualizer::Visualizer(Instance& instance)
{
  running = false;

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
  sf::RenderWindow window(sf::VideoMode(800, 600), "Visualization");

  // set framerate
  window.setFramerateLimit(60);

  // Activate the window for OpenGL rendering
  window.setActive();

  // create a view
  sf::View view(sf::FloatRect(0, 0, 800, 600));

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
    }

    // set the view 
    window.setView(view);

    // clear the window
    window.clear(sf::Color::Black);

    // TODO draw
    window.draw(map_sprite);
    
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

      if (map_data[y*num_of_rows + num_of_columns] == 1) 
      {
        // obstacles
        cell.setFillColor(sf::Color::Red);
      } 
      else 
      {
        // free cells
        cell.setFillColor(sf::Color::White);
      }

      // grid colro and thickness
      cell.setOutlineThickness(1);
      cell.setOutlineColor(sf::Color::Black);

      map_render_texture.draw(cell);
      }
    }

    // Finalize the rendering
    map_render_texture.display();

    // set the map texture
    map_texture = map_render_texture.getTexture();
    map_sprite = sf::Sprite(map_texture);
}
