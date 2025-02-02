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
    const double g, h, f;

    Node(const double g, const double h);
    virtual ~Node() = default; // make Node an abstract class

    // compare
    virtual bool operator>(const Node& other);
    virtual bool operator<(const Node& other);
    virtual bool operator==(const Node& other);
};

// generate a random visually pleasing color, based on
// https://stackoverflow.com/questions/43044/algorithm-to-randomly-generate-an-aesthetically-pleasing-color-palette
sf::Color generate_random_color(const int mix_r, const int mix_g, const int mix_b, const double mix_ratio = 0.5);

