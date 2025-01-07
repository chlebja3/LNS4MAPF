#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// define a path as a vectors of (int, int) tuples
typedef std::vector<std::tuple<int, int>> Path;

sf::Color generate_random_color(int mix_r, int mix_g, int mix_b, double mix_ratio = 0.5);
// generate a random visually pleasing color, based on https://stackoverflow.com/questions/43044/algorithm-to-randomly-generate-an-aesthetically-pleasing-color-palette
