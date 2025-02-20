/*
 * Author: Jan Chleboun
 * Date: 20-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: Class to store the info shared between computation and visualization thread
 */
#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <atomic>
#include <shared_mutex>
#include <vector>

class SharedDara
{
public:
  const std::vector<sf::CircleShape>& get_agent_circles(void) const;

private:
  std::shared_mutex            mtx;
  std::vector<sf::CircleShape> agents_circles;
};
