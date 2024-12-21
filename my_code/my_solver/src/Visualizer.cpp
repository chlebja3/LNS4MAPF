#include <SFML/Graphics.hpp>
#include <iostream>
#include "Visualizer.h"

Visualizer::Visualizer(Instance& instance):instance(instance)
{
  running = false;
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

  while (window.isOpen() && running)
  {
    // poll all events from the window
    sf::Event event;
    while (window.pollEvent(event)) 
    {
      if (event.type == sf::Event::Closed) 
      {
        running = false;
      }
    }

    // clear the window
    window.clear(sf::Color::Black);

    // TODO draw
    
    // display 
    window.display();
  }

  // close the window
  if (window.isOpen()) window.close();
  std::cout << "Visualizer thread ended" << std::endl;
}
