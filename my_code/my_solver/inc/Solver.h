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
  bool feasible = true;

  // paths of the agents
  std::vector<TimePointPath> paths;
  std::vector<int>           delays;
  std::vector<int>           destroyed_paths;

  int cost;
  int sum_of_delays;  // cost - distance
  int sum_of_costs;

  [[nodiscard]] auto is_valid(const Instance& instance) const -> bool;
  void               load(const std::string& filename, const Instance& instance);
  void               calculate_cost(const Instance& instance);
};

class Solver
// abstract class to represent a solver for the mapf problem
{
public:
  const std::string name;
  const Instance&   instance;

  // constructor
  Solver(std::string name_, const Instance& instance_);

  // virtual destructor
  virtual ~Solver() = default;

  // pure virtual function solve
  virtual void solve() = 0;

  Solution solution;  // the so far best solution
};
