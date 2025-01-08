/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: 
 */

#include <utils.h>
#include <cmath>

sf::Color generate_random_color(int mix_r, int mix_g, int mix_b, double mix_ratio)
{
  int r = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_r * mix_ratio);
  int g = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_g * mix_ratio);
  int b = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_b * mix_ratio);
  return sf::Color(r, g, b);
}
