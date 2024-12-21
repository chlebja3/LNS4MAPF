#pragma once
#include <thread>
#include <atomic>
#include <SFML/Graphics.hpp>
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
    void mapToTexture(const std::vector<std::uint8_t>& map_data, int num_of_rows, int num_of_columns);

  private:
    std::thread vis_thread; // the underlying thread
    std::atomic<bool> running; // indicator whether the thread should be running
    sf::Texture map_texture; // the map texture
    sf::Sprite map_sprite; // the sprite that represents the map
};
