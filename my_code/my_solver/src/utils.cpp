/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: 
 */

#include <utils.h>
#include <cmath>

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
sf::Color generate_random_color(int mix_r, int mix_g, int mix_b, double mix_ratio)
{
  int r = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_r * mix_ratio);
  int g = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_g * mix_ratio);
  int b = (int)std::round((std::rand() % 256) * (1 - mix_ratio) + mix_b * mix_ratio);
  return sf::Color(r, g, b);
}
