/*
 * Author: Jan Chleboun
 * Date: 12-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gtest/gtest.h>

#include "Instance.h"
#include "Solver.h"
#include "test_utils.h"
#include "utils.h"

// Test Case 1: Valid Solution (No Collisions, Correct Paths)
TEST(SolutionTest, ValidSolution)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);


  Solution sol;
  sol.paths = {
      {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}},  // Agent 1
      {{2, 2}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}   // Agent 2
  };

  EXPECT_TRUE(sol.is_valid(instance));
}

// Test Case 2: Invalid Solution (Wrong Start Position)
TEST(SolutionTest, InvalidStartPosition)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {{{1, 0}, {2, 0}, {2, 1}, {2, 2}},  // Agent 1 starts at wrong position
               {{2, 2}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}};

  EXPECT_FALSE(sol.is_valid(instance));
}

// Test Case 3: Invalid Solution (Wrong Goal Position)
TEST(SolutionTest, InvalidGoalPosition)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {{{0, 0}, {1, 0}, {2, 0}, {2, 1}, {1, 1}},  // Agent 1 does not reach goal
               {{2, 2}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}};

  EXPECT_FALSE(sol.is_valid(instance));
}

// Test Case 4: Invalid Solution (Collision at Same Position)
TEST(SolutionTest, InvalidCollisionSamePosition)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {
      {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}},  // Agent 1
      {{2, 2}, {2, 1}, {2, 0}, {0, 1}, {0, 0}}   // Collision at (2,0) at t=2
  };

  EXPECT_FALSE(sol.is_valid(instance));
}

// Test Case 5: Invalid Solution (Edge Swap Collision)
TEST(SolutionTest, InvalidEdgeSwapCollision)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {
      {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}},         // Agent 1
      {{2, 2}, {2, 1}, {2, 1}, {2, 0}, {1, 0}, {0, 0}}  // Agents swap places at t=3
  };

  EXPECT_FALSE(sol.is_valid(instance));
}

// Test Case 6: Invalid Solution (Passing Through Obstacle)
TEST(SolutionTest, InvalidThroughObstacle)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {{{0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2}},  // Moves through obstacle
               {{2, 2}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}};

  EXPECT_FALSE(sol.is_valid(instance));
}

// Test Case 7: Empty Paths (No Solution Found)
TEST(SolutionTest, NoSolution)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {};  // No paths

  EXPECT_FALSE(sol.is_valid(instance));
}

// Test Case 8: One empty Path
TEST(SolutionTest, AgentWithoutPath)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {{{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}},  // agent 1
               {}};                                       // agent 2 has no path

  EXPECT_FALSE(sol.is_valid(instance));
}


// Test Case 9: Agent Moves Diagonally (Invalid Move)
TEST(SolutionTest, InvalidDiagonalMove)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/empty_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {{{0, 0}, {1, 1}, {2, 2}},  // Moves diagonally
               {{2, 2}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}};

  EXPECT_FALSE(sol.is_valid(instance));
}

// Test Case 10: Agent Moves Outside Map to negative (Out of Bounds)
TEST(SolutionTest, InvalidMoveOutsideMapNeg)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {{{0, 0}, {0, 1}, {-1, 1}, {-1, 2}, {0, 2}, {1, 2}, {2, 2}},  // Moves out of the map (negative x)
               {{2, 2}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}};

  EXPECT_FALSE(sol.is_valid(instance));
}


// Test Case 11: Agent Moves Outside Map to positive (Out of Bounds)
TEST(SolutionTest, InvalidMoveOutsideMapPos)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {{{0, 0}, {1, 0}, {2, 0}, {3, 0}, {3, 1}, {2, 1}, {2, 2}},  // Moves out of the map (x too big)
               {{2, 2}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}};

  EXPECT_FALSE(sol.is_valid(instance));
}

// Test Case 12: Agent Moves More Than One Cell Per Step (Invalid Jump)
TEST(SolutionTest, InvalidJumpMove)
{
  // load instance
  std::string     base_path = get_base_path_tests();  // path to my_solver
  std::shared_ptr instance =
      std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);

  Solution sol;
  sol.paths = {{{0, 0}, {2, 0}, {2, 2}},  // Jumps from (0,0) to (2,0)
               {{2, 2}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}};

  EXPECT_FALSE(sol.is_valid(instance));
}

