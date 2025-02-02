/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: 
 */

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// define a path as a vectors of (int, int) tuples
typedef std::vector<std::tuple<int, int>> Path;

// an abstract node class
class Node
{
  public:
    Node* parent;
    double g, h;

    Node(const int g, const int h);
    virtual ~Node() = default;

    int get_f(void);
};

// generate a random visually pleasing color, based on
// https://stackoverflow.com/questions/43044/algorithm-to-randomly-generate-an-aesthetically-pleasing-color-palette
sf::Color generate_random_color(const int mix_r, const int mix_g, const int mix_b, const double mix_ratio = 0.5);

