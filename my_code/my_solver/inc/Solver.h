/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Instance.h"
#include "utils.h"

class Solution
// struct to store a solution of the mapf problem
{
public:
  bool feasible;

  // paths of the agents
  std::vector<Path> paths;

  int cost;
  int delay;  // cost - distance

  bool is_valid(const Instance& instance) const;
  void load(const std::string& filename, const Instance& instance);
};

class Solver
// abstract class to represent a solver for the mapf problem
{
public:
  const std::string name;
  const Instance&   instance;

  // constructor
  Solver(const std::string& name, const Instance& instance);

  // virtual destructor
  virtual ~Solver() = default;

  // pure virtual function solve
  virtual void solve() = 0;

  Solution solution;  // the so far best solution
};
