/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: 
 */

#include <utils.h>
#include <cmath>

Node::Node(const double g, const double h) : g(g), h(h), f(g+h) {};

bool Node::operator>(const Node& other)
{
  return f > other.f;
}

bool Node::operator<(const Node& other)
{
  return f < other.f;
}

bool Node::operator==(const Node& other)
{
  return g == other.g && h == other.h;
}

/**
 * @brief Generate a random color. To make the color palette look more visually pleasing, mix the random color with (mix_r, mix_g, mix_b).
 *
 * @param mix_r The red channel value of the mix color, recommended value is 255.
 * @param mix_g The green channel value of the mix color, recommended value is 255.
 * @param mix_b The blue channel value of the mix color, recommended value is 255.
 * @param mix_ratio The ratio at which to blend the randomly generated color with the mix color.
 *
 * @return
 */
sf::Color generate_random_color(const int mix_r, const int mix_g, const int mix_b, const double mix_ratio)
{
  const int r = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_r * mix_ratio);
  const int g = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_g * mix_ratio);
  const int b = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_b * mix_ratio);
  return sf::Color(r, g, b);
}
