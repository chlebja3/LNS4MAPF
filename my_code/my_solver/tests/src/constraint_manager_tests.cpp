/*
 * Author: Jan Chleboun
 * Date: 26-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <gtest/gtest.h>

#include <climits>

#include "ConstraintManager.h"
#include "Instance.h"
#include "LNS.h"
#include "SIPP.h"
#include "test_utils.h"

/**
 * @brief Class for testing the ConstraintManager class
 */
class ConstraintManagerTest : public ::testing::Test
{
protected:
  std::unique_ptr<Instance>          instance;
  std::unique_ptr<ConstraintManager> constraint_manager;

  void SetUp() override
  {
    // load instance
    std::string base_path = get_base_path_tests();  // path to my_solver

    instance =
        std::make_unique<Instance>(base_path + "/tests/test_maps/empty_3_3.map", base_path + "/tests/test_scen/empty_3_3_scen_1.scen", 4);
    constraint_manager = std::make_unique<ConstraintManager>(*instance);
  }

  void TearDown() override
  {
  }
};

// Adding a path that forms constraints, test whether edge constraints are added
// Adding a path that forms constraints, test whether edge constraints are added
TEST_F(ConstraintManagerTest, AddPathConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint(6, {0, 2}), TimePoint(7, {3, 4}), TimePoint(8, {5, 6})};
  constraint_manager->add_trajectory(path1, 0);
  std::vector<TimePoint> path2 = {TimePoint(3, {0, 5}), TimePoint(6, {6, 8}), TimePoint(7, {9, 10})};
  constraint_manager->add_trajectory(path2, 1);

  // test first edge conflict
  std::vector<TimeInterval> safe_intervals =
      filter_time_intervals(constraint_manager->get_safe_intervals(6, {0, 3}), {0, 3}, 7, 6, *constraint_manager);
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe interval due to an edge conflict.";

  // test going to 1, 1 from a non constrained position
  auto [start, end] = constraint_manager->get_safe_intervals(6, {0, 10});
  ASSERT_EQ(std::distance(start, end), 2) << "Expected two safe intervals after adding path constraints.";
  EXPECT_EQ(start->interval, TimeInterval(3, 5));
  EXPECT_EQ(std::next(start)->interval, TimeInterval(9, INT_MAX));

  // test second edge conflict
  auto start_end = constraint_manager->get_safe_intervals(6, {5, 9});
  safe_intervals       = filter_time_intervals(start_end, {5, 9}, 7, 6, *constraint_manager);
  EXPECT_EQ(safe_intervals.size(), 1) << "Expected only one safe interval.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(5, 5));
}

// check that the edge conflict is resolved correctly when there are more constraints in the list
TEST_F(ConstraintManagerTest, EdgeConflictMoreConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint(6, {0, 1}), TimePoint(7, {2, 4}), TimePoint(6, {5, 6}), TimePoint(7, {7, 8}),
                                  TimePoint(6, {9, 10})};
  constraint_manager->add_trajectory(path1, 0);

  std::vector<TimeInterval> safe_intervals =
      filter_time_intervals(constraint_manager->get_safe_intervals(6, {0, 2}), {0, 2}, 7, 6, *constraint_manager);
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe intervals due to edge conflict.";
}


// Constraint Table tests
// Adding a single constraint and checking if it is stored correctly
TEST_F(ConstraintManagerTest, SingleConstraintBlockingAgent)
{
  TimePoint tp(7, {3, 5});  // location (1,2) with time interval [3,5]

  constraint_manager->add_trajectory({tp}, 0);

  // test that the visiting agents counts are right
  ASSERT_EQ(constraint_manager->get_agents_counts(7).size(), 1) << "There should be one agent.";
  EXPECT_EQ(constraint_manager->get_agents_counts(7).begin()->first, 0);
  EXPECT_EQ(constraint_manager->get_agents_counts(7).begin()->second, 1);

  // test that there is no constraint before
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 2), std::make_pair(-1, -1));

  // test that the constraint is stored correctly
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 3), std::make_pair(0, -1));
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 4), std::make_pair(0, -1));
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 5), std::make_pair(0, -1));

  // test that there is no constraint after
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 6), std::make_pair(-1, -1));
}

// Adding multiple constraints and ensuring safe intervals are computed correctly
TEST_F(ConstraintManagerTest, MultipleSingleConstraintsBlockingAgents)
{
  TimePointPath traj0({TimePoint(7, {2, 4})});
  TimePointPath traj1({TimePoint(7, {5, 5})});
  TimePointPath traj2({TimePoint(7, {6, 7})});
  TimePointPath traj3({TimePoint(7, {10, 15})});
  constraint_manager->add_trajectory(traj0, 0);
  constraint_manager->add_trajectory(traj1, 1);
  constraint_manager->add_trajectory(traj2, 2);
  constraint_manager->add_trajectory(traj3, 3);

  // test that the visiting agents counts are right
  ASSERT_EQ(constraint_manager->get_agents_counts(7).size(), 4) << "There should be four agents.";
  for (const auto& it : constraint_manager->get_agents_counts(7))
  {
    // expect that the agent is 0, 1, 2 or 3 and that the count is 1
    EXPECT_TRUE(it.first >= 0 && it.first <= 3) << "Agent should be 0, 1, 2 or 3.";
    EXPECT_EQ(it.second, 1) << "Count should be 1.";
  }

  // test that there is no constraint before
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 1), std::make_pair(-1, -1));

  // test that the constraints are stored correctly
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 2), std::make_pair(0, -1));
  EXPECT_EQ(constraint_manager->get_blocking_agent(8, 7, 5), std::make_pair(1, -1));
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 6), std::make_pair(2, -1));
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 10), std::make_pair(3, -1));
  EXPECT_EQ(constraint_manager->get_blocking_agent(8, 7, 15), std::make_pair(3, -1));

  // test that there is no constraint after
  EXPECT_EQ(constraint_manager->get_blocking_agent(6, 7, 16), std::make_pair(-1, -1));
}

// Adding a path that forms constraints, test whether edge constraints are added
TEST_F(ConstraintManagerTest, AddPathConstraintsBlocking)
{
  TimePointPath path1 = {TimePoint(4, {0, 2}), TimePoint(7, {3, 4}), TimePoint(8, {5, 6})};
  constraint_manager->add_trajectory(path1, 1);
  TimePointPath path2 = {TimePoint(5, {0, 5}), TimePoint(4, {6, 8}), TimePoint(7, {9, 10})};
  constraint_manager->add_trajectory(path2, 2);

  // test first edge conflict
  EXPECT_EQ(constraint_manager->get_blocking_agent(7, 4, 2), std::make_pair(1, -1));  // regular conflict
  EXPECT_EQ(constraint_manager->get_blocking_agent(7, 4, 3), std::make_pair(-1, 1)) << "There should be edge conflict.";  // edge conflict

  // test going to 4, from a non constrained position
  EXPECT_EQ(constraint_manager->get_blocking_agent(1, 4, 3), std::make_pair(-1, -1)) << "There should be no conflict.";
  EXPECT_EQ(constraint_manager->get_blocking_agent(1, 4, 9), std::make_pair(-1, -1)) << "There should be no conflict.";

  // test second edge conflict
  EXPECT_EQ(constraint_manager->get_blocking_agent(7, 4, 5), std::make_pair(-1, -1));
  EXPECT_EQ(constraint_manager->get_blocking_agent(7, 4, 6), std::make_pair(2, -1));  // regular conflict
  EXPECT_EQ(constraint_manager->get_blocking_agent(7, 4, 9), std::make_pair(-1, 2));  // edge  conflict
}

// check that the edge conflict is resolved correctly when there are more constraints in the list
TEST_F(ConstraintManagerTest, EdgeConflictMoreConstraintsBlocking)
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 1}), TimePoint(7, {2, 4}), TimePoint(4, {5, 6}), TimePoint(7, {7, 8}),
                                  TimePoint(4, {9, 10})};
  constraint_manager->add_trajectory(path1, 1);

  EXPECT_EQ(constraint_manager->get_blocking_agent(7, 4, 2), std::make_pair(-1, 1)) << "There should be edge conflict.";
  EXPECT_EQ(constraint_manager->get_blocking_agent(7, 4, 7), std::make_pair(-1, 1)) << "There should be edge conflict.";

  // test agent counts
  ASSERT_EQ(constraint_manager->get_agents_counts(4).size(), 1) << "There should be one agent.";
  EXPECT_EQ(constraint_manager->get_agents_counts(4).begin()->first, 1) << "There should be 3 counts for agent 1.";
  EXPECT_EQ(constraint_manager->get_agents_counts(4).begin()->second, 3) << "There should be 3 counts for agent 1.";
}
