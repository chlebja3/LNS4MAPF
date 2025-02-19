/*
 * Author: Jan Chleboun
 * Date: 16-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gtest/gtest.h>
#include <limits.h>

#include "Astar.h"
#include "ConstraintTable.h"
#include "Instance.h"
#include "LNS.h"
#include "SIPP.h"
#include "test_utils.h"

/**
 * @brief Class for testing the ConstraintTable class
 */
class ConstraintTableTest : public ::testing::Test
{
protected:
  std::unique_ptr<Instance>        instance;
  std::unique_ptr<ConstraintTable> table;

  void SetUp() override
  {
    // load instance
    std::string base_path = get_base_path_tests();  // path to my_solver

    instance =
        std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);
    table = std::make_unique<ConstraintTable>(*instance);
  }

  void TearDown() override
  {
  }
};

// Constraint Table tests
// Test 1: Adding a single constraint and checking if it is stored correctly
TEST_F(ConstraintTableTest, SingleConstraint)
{
  TimePoint tp({1, 2}, {3, 5});  // Position (1,2) with time interval [3,5]
                                 //
  table->add_constraint(tp);

  // test that the last constraint is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 3);

  // test that the first safe interval is right
  TimeInterval first_si = table->get_first_safe_interval({1, 2});
  EXPECT_EQ(first_si, TimeInterval(0, 2));

  // test that the interval after the constraint is retrieved
  TimeInterval after_si = table->get_safe_interval({1, 2}, {1, 1}, 3);
  EXPECT_EQ(after_si, TimeInterval(6, INT_MAX));

  // test that both the interval before and after the constraint are retrieved
  std::vector<TimeInterval> safeIntervals = table->get_safe_intervals({1, 2}, {1, 1}, {0, 6});
  ASSERT_EQ(safeIntervals.size(), 2) << "Expected two safe intervals after adding a constraint!";
  EXPECT_EQ(safeIntervals[0], TimeInterval(0, 2));
  EXPECT_EQ(safeIntervals[1], TimeInterval(6, INT_MAX));

  // test that no interval is retrieved when the constraint is in the way
  safeIntervals = table->get_safe_intervals({1, 2}, {1, 1}, {2, 5});
  ASSERT_EQ(safeIntervals.size(), 1) << "Expected only one safe itnerval!";
  EXPECT_EQ(safeIntervals[0], TimeInterval(2, 2));
}

// Test 2: Adding multiple constraints and ensuring safe intervals are computed correctly
TEST_F(ConstraintTableTest, MultipleSingleConstraints)
{
  table->add_constraint(TimePoint({1, 2}, {2, 4}));
  table->add_constraint(TimePoint({1, 2}, {5, 5}));
  table->add_constraint(TimePoint({1, 2}, {6, 7}));
  table->add_constraint(TimePoint({1, 2}, {10, 15}));

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 10);

  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals({1, 2}, {0, 0}, {0, 20});
  ASSERT_EQ(safe_intervals.size(), 3) << "Expected three safe intervals after adding multiple constraints!";
  EXPECT_EQ(safe_intervals[0], TimeInterval(0, 1));
  EXPECT_EQ(safe_intervals[1], TimeInterval(8, 9));
  EXPECT_EQ(safe_intervals[2], TimeInterval(16, INT_MAX));
}

// Test 3: Removing a constraint and ensuring safe intervals update
TEST_F(ConstraintTableTest, RemoveConstraint)
{
  TimePoint tp({1, 2}, {3, 5});
  table->add_constraint(tp);
  table->remove_constraint(tp);

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 0);

  TimeInterval first_si = table->get_first_safe_interval({1, 2});
  EXPECT_TRUE(first_si.t_min == 0 && first_si.t_max == INT_MAX);
}

// Test 4: Querying safe interval for an empty position
TEST_F(ConstraintTableTest, GetSafeIntervalEmpty)
{
  TimeInterval first_si = table->get_first_safe_interval({1, 1});
  EXPECT_TRUE(first_si.t_min == 0 && first_si.t_max == INT_MAX) << "No constraints should mean an unlimited safe interval.";
}

// Test 5: Handling constraints dynamically (adding and removing in sequence)
TEST_F(ConstraintTableTest, DynamicAddRemove)
{
  table->add_constraint(TimePoint({1, 1}, {2, 4}));
  table->add_constraint(TimePoint({1, 1}, {6, 8}));
  table->remove_constraint(TimePoint({1, 1}, {2, 4}));

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 6);

  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals({1, 1}, {0, 0}, {0, 10});
  ASSERT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals after removal.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(0, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(9, INT_MAX));
}

// Test 6: Constraint removal from empty table if in debug mode
#ifdef NDEBUG
TEST_F(ConstraintTableTest, DISABLED_RemoveFromEmptyTable)
#else
TEST_F(ConstraintTableTest, RemoveFromEmptyTable)
#endif
{
  TimePoint tp({1, 1}, {3, 6});
  // expect assert death
  ASSERT_DEATH(table->remove_constraint(tp), "Trying to remove interval from an empty list.");
}

// Test 7: Constraint removal when no constraint exists
TEST_F(ConstraintTableTest, RemoveNonExistentConstraint)
{
  TimePoint tp({1, 1}, {3, 6});
  // add a dummy constraint to prevent assertion failure removing from empty table
  table->add_constraint(TimePoint({1, 1}, {0, 2}));
  try
  {
    table->remove_constraint(tp);
    FAIL() << "Removing non-existent constraint should throw an exception.";
  }
  catch (std::runtime_error& e)
  {
    EXPECT_STREQ(e.what(), "Trying to remove non-existing interval from the constraint table.");
  }
  catch (...)
  {
    FAIL() << "Removing non-existent constraint should throw a runtime error.";
  }
}

// Test 8: Adding a constraint that is already present (should cause assert death)
#ifdef NDEBUG
TEST_F(ConstraintTableTest, DISABLED_AddExistingConstraint)
#else
TEST_F(ConstraintTableTest, AddExistingConstraint)
#endif
{
  TimePoint tp({1, 1}, {3, 6});
  table->add_constraint(tp);
  ASSERT_DEATH(table->add_constraint(tp), "Cannot add overlapping constraints.");
}

// Test 9: Adding a path that forms constraints, test whether edge constraints are added
TEST_F(ConstraintTableTest, AddPathConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint({1, 1}, {0, 2}), TimePoint({1, 2}, {3, 4}), TimePoint({2, 2}, {5, 6})};
  table->add_constraints(path1);
  std::vector<TimePoint> path2 = {TimePoint({2, 1}, {0, 5}), TimePoint({1, 1}, {6, 8}), TimePoint({1, 2}, {9, 10})};
  table->add_constraints(path2);

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 9);

  // test first edge conflict
  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals({1, 1}, {1, 2}, {0, 3});
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe interval due to an edge conflict.";

  // test going to 1, 1 from a non constrained position
  safe_intervals = table->get_safe_intervals({1, 1}, {1, 0}, {0, 10});
  ASSERT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals after adding path constraints.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(3, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(9, INT_MAX));

  // test second edge conflict
  safe_intervals = table->get_safe_intervals({1, 1}, {1, 2}, {5, 9});
  EXPECT_EQ(safe_intervals.size(), 1) << "Expected only one safe interval.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(5, 5));
}

// Test 10: check assert death when the safe interval, because, there will be an edge conflict
#ifdef NDEBUG
TEST_F(ConstraintTableTest, DISABLED_WrongSafeInterval)
#else
TEST_F(ConstraintTableTest, WrongSafeInterval)
#endif
{
  std::vector<TimePoint> path1 = {TimePoint({1, 1}, {0, 1}), TimePoint({1, 2}, {2, 4})};
  table->add_constraints(path1);

  // expect assert death
  ASSERT_DEATH(table->get_safe_intervals({1, 1}, {1, 2}, {0, 3}), "Invalid time interval, the from position will be in edge collision.");
}

// Test 11: check assert death when the safe interval of to position is not right and there are more constraints in the list
#ifdef NDEBUG
TEST_F(ConstraintTableTest, DISABLED_WrongSafeIntervalMoreConstraints)
#else
TEST_F(ConstraintTableTest, WrongSafeIntervalMoreConstraints)
#endif
{
  std::vector<TimePoint> path1 = {TimePoint({1, 1}, {0, 1}), TimePoint({1, 2}, {2, 4}), TimePoint({1, 1}, {5, 6}),
                                  TimePoint({1, 2}, {7, 8}), TimePoint({1, 1}, {9, 10})};
  table->add_constraints(path1);

  // expect assert death
  ASSERT_DEATH(table->get_safe_intervals({1, 1}, {1, 2}, {0, 3}), "Invalid time interval, the from position will be in edge collision.");
}

// Test 12 check that the edge conflict is resolved correctly when there are more constraints in the list
TEST_F(ConstraintTableTest, EdgeConflictMoreConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint({1, 1}, {0, 1}), TimePoint({1, 2}, {2, 4}), TimePoint({1, 1}, {5, 6}),
                                  TimePoint({1, 2}, {7, 8}), TimePoint({1, 1}, {9, 10})};
  table->add_constraints(path1);

  // test going to 1, 1 from a position with edge conflict in time 0 - 2
  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals({1, 1}, {1, 2}, {0, 2});
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe intervals due to edge conflict.";
}

// SIPP tests

// **Test 1: Basic pathfinding in an empty map**
TEST(SIPPTest, BasicPathEmptyMap)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (0,0) to (4,4)
  Point2d       start = instance->get_start_positions()[0];
  Point2d       goal  = instance->get_goal_positions()[0];
  TimePointPath path  = sipp.plan(start, goal);

  ASSERT_EQ(path.size(), 9) << "SIPP failed to find a basic path!";
  EXPECT_EQ(path.front().position, start) << "Path does not start at the correct point!";
  EXPECT_EQ(path.back().position, goal) << "Path does not end at the correct point!";
}

// **Test 2: Pathfinding with static obstacles**
TEST(SIPPTest, StaticObstacleAvoidance)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/wall_5_5.map", base_path + "/tests/test_scen/wall_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (4,0) to (0,4)
  Point2d       start = instance->get_start_positions()[0];
  Point2d       goal  = instance->get_goal_positions()[0];
  TimePointPath path  = sipp.plan(start, goal);

  // check that the path size is correct
  ASSERT_EQ(path.size(), 9) << "SIPP failed to find a path around obstacles!";

  // check that the path starts and end at the correct points
  EXPECT_EQ(path.front().position, start) << "Path does not start at the correct point!";
  EXPECT_EQ(path.back().position, goal) << "Path does not end at the correct point!";

  // check that no obstacles are in the path
  for (const auto& tp : path)
  {
    EXPECT_TRUE(instance->get_map_data().index(tp.position) == 0) << "Path goes through an obstacle!";
  }
}

// **Test 3: Handling of time constraints (SIPP-specific test)**
TEST(SIPPTest, TimeConstraintHandling)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (0,0) to (4,4)
  Point2d start = instance->get_start_positions()[0];
  Point2d goal  = instance->get_goal_positions()[0];

  // Create a wall from time constraints
  for (int i = 0; i < 5; i++)
  {
    if (i == 2) continue;  // passage through (2,2)
    sipp.constraint_table.add_constraint(TimePoint({i, 2}, {0, 20}));
  }

  TimePointPath path = sipp.plan(start, goal);

  ASSERT_EQ(path.size(), 9) << "Path length invalid!";

  // check that the path does not go through the blocked area
  for (const auto& tp : path)
  {
    if (tp.position.y == 2)
    {
      EXPECT_EQ(tp.position.x, 2) << "Path goes through a blocked area!";
    }
  }

  // check the completion time of the path
  EXPECT_EQ(path.back().interval.t_min, 8) << "Path does not complete in the expected time!";
}

// **Test 4: Start and goal are the same (Trivial case)**
TEST(SIPPTest, TrivialStartGoal)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  Point2d start = Point2d(0, 0);

  TimePointPath path = sipp.plan(start, start);

  ASSERT_EQ(path.size(), 1) << "SIPP should return a single-point path!";
  EXPECT_EQ(path.front().position, start) << "Path should start and end at the same point!";
}

// **Test 5: No possible path due to obstacles**
TEST(SIPPTest, NoPathPossible)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/biparted_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (0,0) to (4,4)
  Point2d start = instance->get_start_positions()[0];
  Point2d goal  = instance->get_goal_positions()[0];

  TimePointPath path = sipp.plan(start, goal);

  ASSERT_TRUE(path.empty()) << "SIPP should return an empty path when no path exists!";
}

// **Test 6: No path due to time constraints**
TEST(SIPPTest, NoPathDueToTimeConstraints)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (0,0) to (4,4)
  Point2d start = instance->get_start_positions()[0];
  Point2d goal  = instance->get_goal_positions()[0];

  // Add constraints that make it impossible to leave start until time 10
  sipp.constraint_table.add_constraint(TimePoint({1, 0}, {0, 10}));
  sipp.constraint_table.add_constraint(TimePoint({0, 1}, {0, 10}));

  // add constraints that make it impossible to reach the goal from time 10 to 20
  sipp.constraint_table.add_constraint(TimePoint({3, 4}, {10, 20}));
  sipp.constraint_table.add_constraint(TimePoint({4, 3}, {10, 20}));

  // plan path
  TimePointPath path = sipp.plan(start, goal);

  // check that path was found
  ASSERT_FALSE(path.empty()) << "SIPP returned no path!";

  // check that the path does not leave the start until time 10
  for (const auto& tp : path)
  {
    if (tp.position == Point2d(1, 0) || tp.position == Point2d(0, 1))
    {
      EXPECT_TRUE(tp.interval.t_min > 10) << "Path leaves start before time 10!";
    }
  }

  // check that the path does not reach the goal before time 21
  for (const auto& tp : path)
  {
    if (tp.position == goal)
    {
      EXPECT_TRUE(tp.interval.t_min > 20) << "Path reaches goal before or at time 20!";
    }
  }
}

// **Test 7: Start outside of the map**
TEST(SIPPTest, StartOutsideMap)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  Point2d start = Point2d(3, 1);
  Point2d goal  = instance->get_goal_positions()[0];

  TimePointPath path = sipp.plan(start, goal);

  ASSERT_TRUE(path.empty()) << "SIPP should return an empty path when start is outside the map!";
}

// **Test 8: Goal outside of the map**
TEST(SIPPTest, GoalOutsideMap)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  Point2d start = instance->get_start_positions()[0];
  Point2d goal  = Point2d(-1, 0);

  TimePointPath path = sipp.plan(start, goal);

  ASSERT_TRUE(path.empty()) << "SIPP should return an empty path when goal is outside the map!";
}

// **Test 9: static obstacle at the goal**
TEST(SIPPTest, StaticObstacleAtGoal)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/wall_5_5.map", base_path + "/tests/test_scen/wall_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  Point2d start = Point2d(1, 2);
  Point2d goal  = Point2d(2, 1);

  TimePointPath path = sipp.plan(start, goal);

  ASSERT_TRUE(path.empty()) << "SIPP should return an empty path when goal is blocked!";
}

// **Test 10: dynamic obstacle at the goal after time of possible arrival**
TEST(SIPPTest, DynamicObstacleAtGoal)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (0,0) to (4,4)
  Point2d start = instance->get_start_positions()[0];
  Point2d goal  = instance->get_goal_positions()[0];

  // add a constraint that blocks the goal after time 20
  sipp.constraint_table.add_constraint(TimePoint(goal, {20, 30}));

  TimePointPath path = sipp.plan(start, goal);

  // the path should be found
  ASSERT_FALSE(path.empty()) << "SIPP should find a path when the goal is blocked after possible arrival time!";

  // check that the path reaches the goal at time 31
  EXPECT_EQ(path.back().interval.t_min, 31) << "Path does not reach the goal at the expected time!";
}

// test that SIPP takes into account the edge constraints
// **Test 11: Edge constraints at the goal
TEST(SIPPTest, EdgeConstraintsAtGoal)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_2.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (2, 0) to (2, 1)
  Point2d start = instance->get_start_positions()[0];
  Point2d goal  = instance->get_goal_positions()[0];

  // add edge constraints at the goal so that the agent has to go around
  sipp.constraint_table.add_constraints({TimePoint({2, 1}, {0, 0}), TimePoint({2, 0}, {1, 1}), TimePoint({1, 0}, {2, 2}),
                                         TimePoint({0, 0}, {3, 3}), TimePoint({0, 1}, {4, 4}), TimePoint({0, 2}, {5, 5}),
                                         TimePoint({1, 2}, {6, 6}), TimePoint({2, 2}, {7, 7})});


  TimePointPath path = sipp.plan(start, goal);

  // check that the path has the right length
  ASSERT_EQ(path.size(), 8) << "SIPP should find a path.";

  // check that the path is correct
  EXPECT_EQ(path, TimePointPath({TimePoint({2, 0}, {0, 0}), TimePoint({1, 0}, {1, 1}), TimePoint({0, 0}, {2, 2}), TimePoint({0, 1}, {3, 3}),
                                 TimePoint({0, 2}, {4, 4}), TimePoint({1, 2}, {5, 5}), TimePoint({2, 2}, {6, 6}),
                                 TimePoint({2, 1}, {7, INT_MAX})}));
}

// **Test 12: SIPP plans the same path as A* when no constraints are present
TEST(SIPPTest, SamePathAsAStar1)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (0,0) to (4,4)
  Point2d start = instance->get_start_positions()[0];
  Point2d goal  = instance->get_goal_positions()[0];

  // plan path by ASTAR
  Path path_astar = ASTAR::find_path(0, *instance);

  // assert that astar found a path
  ASSERT_FALSE(path_astar.empty()) << "A* should find a path when no constraints are present!";

  // plan path by SIPP
  TimePointPath path_sipp = sipp.plan(start, goal);

  // assert that SIPP found a path
  ASSERT_FALSE(path_sipp.empty()) << "SIPP should find a path when no constraints are present!";

  // convert SIPP path to the same format as A*
  Path path_sipp_converted = timepointpath_to_path(path_sipp);

  // check that the paths have the same length
  EXPECT_EQ(path_sipp_converted.size(), path_astar.size()) << "The paths found by SIPP and ASTAR should have the same length!";

  // check whether both algorithms expanded the same number of nodes
  EXPECT_EQ(ASTAR::generated_nodes, sipp.generated_nodes) << "SIPP should expand the same number of nodes as A*!";
}

// **Test 14: SIPP plans the same path as A* when no dynamic constraints are present
TEST(SIPPTest, SamePathAsAStar2)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", 1);

  // create SIPP
  SIPP sipp(*instance);

  // from (0,0) to (4,4)
  Point2d start = instance->get_start_positions()[0];
  Point2d goal  = instance->get_goal_positions()[0];

  // plan path by ASTAR
  Path path_astar = ASTAR::find_path(0, *instance);

  // assert that astar found a path
  ASSERT_FALSE(path_astar.empty()) << "A* should find a path when no constraints are present!";

  // plan path by SIPP
  TimePointPath path_sipp = sipp.plan(start, goal);

  // assert that SIPP found a path
  ASSERT_FALSE(path_sipp.empty()) << "SIPP should find a path when no constraints are present!";

  // convert SIPP path to the same format as A*
  Path path_sipp_converted = timepointpath_to_path(path_sipp);

  // check that the paths have the same length
  EXPECT_EQ(path_sipp_converted.size(), path_astar.size()) << "The paths found by SIPP and ASTAR should have the same length!";


  // std::cout << "SIPP expanded " << sipp.generated_nodes << " nodes." << std::endl;
  // std::cout << "A* expanded " << ASTAR::generated_nodes << " nodes." << std::endl;

  // check whether both algorithms expanded the same number of nodes
  EXPECT_EQ(ASTAR::generated_nodes, sipp.generated_nodes) << "SIPP should expand the same number of nodes as A*!";
}


// Test Prioritized Planning
// Test 1: Prioritized Planning with dummy map and 1 agent
TEST(SIPPTest, PrioritizedPlanning1agent)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);

  // create LNS
  LNS lns(*instance);

  // plan path by Prioritized Planning
  Solution sol = lns.PrioritizedPlanning();

  // check solution validity
  EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
}

// Test 2: Prioritized Planning with empty map and 2 agents
TEST(SIPPTest, PrioritizedPlanningEmpty2agents)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 2);

  // create LNS
  LNS lns(*instance);

  // plan path by Prioritized Planning
  Solution sol = lns.PrioritizedPlanning();

  // print the first path
  // for (const auto& tp : sol.paths[0])
  // {
  //   std::cout << tp << " " << std::endl;
  // }
  // std::cout << std::endl;

  // check solution validity
  EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
}


// Test Prioritized Planning
TEST(SIPPTest, PrioritizedPlanning)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", 100);

  // create LNS
  LNS lns(*instance);

  // plan path by Prioritized Planning
  Solution sol = lns.PrioritizedPlanning();

  // check solution validity
  EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
}

