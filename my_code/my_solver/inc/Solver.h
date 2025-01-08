/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: 
 */

#pragma once
#include <string>
#include <vector>
#include "Instance.h"
#include "utils.h"

struct Solution
// struct to store a solution of the mapf problem
{
  bool feasible;

  // paths of the agents
  std::vector<Path> paths;

  int cost;
  int delay;  // cost - distance
};

class Solver
// abstract class to represent a solver for the mapf problem
{
public:
  std::string name;
  Instance    instance;

  // constructor
  Solver(const std::string& name, Instance& instance);

  // pure virtual function solve
  virtual Solution solve() = 0;
};
