/*
 * Author: Jan Chleboun
 * Date: 26-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <gtest/gtest.h>

#include <climits>

#include "Astar.h"
#include "Instance.h"
#include "old/ConstraintTable_old.h"
#include "old/LNS_old.h"
#include "old/SIPP_old.h"
#include "test_utils.h"

/**
 * @brief Class for testing the ConstraintTable class
 */
class ConstraintTableTestOld : public ::testing::Test
{
protected:
  std::unique_ptr<Instance>             instance;
  std::unique_ptr<old::ConstraintTable> table;

  void SetUp() override
  {
    // load instance
    std::string base_path = get_base_path_tests();  // path to my_solver

    instance =
        std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);
    table = std::make_unique<old::ConstraintTable>(*instance);
  }

  void TearDown() override
  {
  }
};

// Constraint Table tests
// Adding a single constraint and checking if it is stored correctly
// old implementation
TEST_F(ConstraintTableTestOld, SingleConstraint)
{
  TimePoint tp(7, {3, 5});  // location (1,2) with time interval [3,5]
                            //
  table->add_constraint(tp);

  // test that the last constraint is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 3);

  // test that the first safe interval is right
  TimeInterval first_si = table->get_first_safe_interval(7);
  EXPECT_EQ(first_si, TimeInterval(0, 2));

  // test that the interval after the constraint is retrieved
  TimeInterval after_si = table->get_safe_interval(7, 4, 3);
  EXPECT_EQ(after_si, TimeInterval(6, INT_MAX));

  // test that both the interval before and after the constraint are retrieved
  std::vector<TimeInterval> safeIntervals = table->get_safe_intervals(7, 4, {0, 6});
  ASSERT_EQ(safeIntervals.size(), 2) << "Expected two safe intervals after adding a constraint!";
  EXPECT_EQ(safeIntervals[0], TimeInterval(0, 2));
  EXPECT_EQ(safeIntervals[1], TimeInterval(6, INT_MAX));

  // test that no interval is retrieved when the constraint is in the way
  safeIntervals = table->get_safe_intervals(7, 4, {2, 5});
  ASSERT_EQ(safeIntervals.size(), 1) << "Expected only one safe interval!";
  EXPECT_EQ(safeIntervals[0], TimeInterval(2, 2));
}

// Adding multiple constraints and ensuring safe intervals are computed correctly
TEST_F(ConstraintTableTestOld, MultipleSingleConstraints)
{
  table->add_constraint(TimePoint(7, {2, 4}));
  table->add_constraint(TimePoint(7, {5, 5}));
  table->add_constraint(TimePoint(7, {6, 7}));
  table->add_constraint(TimePoint(7, {10, 15}));

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 10);

  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals(7, 0, {0, 20});
  ASSERT_EQ(safe_intervals.size(), 3) << "Expected three safe intervals after adding multiple constraints!";
  EXPECT_EQ(safe_intervals[0], TimeInterval(0, 1));
  EXPECT_EQ(safe_intervals[1], TimeInterval(8, 9));
  EXPECT_EQ(safe_intervals[2], TimeInterval(16, INT_MAX));
}

// Removing a constraint and ensuring safe intervals update
TEST_F(ConstraintTableTestOld, RemoveConstraintOld)
{
  TimePoint tp(7, {3, 5});
  table->add_constraint(tp);
  table->remove_constraint(tp);

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 0);

  TimeInterval first_si = table->get_first_safe_interval(7);
  EXPECT_TRUE(first_si.t_min == 0 && first_si.t_max == INT_MAX);
}

// Querying safe interval for an empty position
TEST_F(ConstraintTableTestOld, GetSafeIntervalEmpty)
{
  TimeInterval first_si = table->get_first_safe_interval(4);
  EXPECT_TRUE(first_si.t_min == 0 && first_si.t_max == INT_MAX) << "No constraints should mean an unlimited safe interval.";
}

// Handling constraints dynamically (adding and removing in sequence)
TEST_F(ConstraintTableTestOld, DynamicAddRemove)
{
  table->add_constraint(TimePoint(4, {2, 4}));
  table->add_constraint(TimePoint(4, {6, 8}));
  table->remove_constraint(TimePoint(4, {2, 4}));

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 6);

  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals(4, 0, {0, 10});
  ASSERT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals after removal.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(0, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(9, INT_MAX));
}

// Constraint removal from empty table if in debug mode
#ifdef NDEBUG
TEST_F(ConstraintTableTestOld, DISABLED_RemoveFromEmptyTable)
#else
TEST_F(ConstraintTableTestOld, RemoveFromEmptyTable)
#endif
{
  TimePoint tp(4, {3, 6});
  // expect assert death
  EXPECT_DEATH(table->remove_constraint(tp), "Trying to remove interval from an empty list.");
}

// Constraint removal when no constraint exists
TEST_F(ConstraintTableTestOld, RemoveNonExistentConstraint)
{
  TimePoint tp(4, {3, 6});
  // add a dummy constraint to prevent assertion failure removing from empty table
  table->add_constraint(TimePoint(4, {0, 2}));
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
TEST_F(ConstraintTableTestOld, DISABLED_AddExistingConstraint)
#else
TEST_F(ConstraintTableTestOld, AddExistingConstraint)
#endif
{
  TimePoint tp(4, {3, 6});
  table->add_constraint(tp);
  ASSERT_DEATH(table->add_constraint(tp), "Cannot add overlapping constraints.");
}

// Adding a path that forms constraints, test whether edge constraints are added
TEST_F(ConstraintTableTestOld, AddPathConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 2}), TimePoint(7, {3, 4}), TimePoint(8, {5, 6})};
  table->add_constraints(path1);
  std::vector<TimePoint> path2 = {TimePoint(5, {0, 5}), TimePoint(4, {6, 8}), TimePoint(7, {9, 10})};
  table->add_constraints(path2);

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint(), 9);

  // test first edge conflict
  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals(4, 7, {0, 3});
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe interval due to an edge conflict.";

  // test going to 1, 1 from a non constrained position
  safe_intervals = table->get_safe_intervals(4, 1, {0, 10});
  ASSERT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals after adding path constraints.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(3, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(9, INT_MAX));

  // test second edge conflict
  safe_intervals = table->get_safe_intervals(4, 7, {5, 9});
  EXPECT_EQ(safe_intervals.size(), 1) << "Expected only one safe interval.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(5, 5));
}

// check assert death when the time interval is wrong, because, there will be an edge conflict if an agent came from the location
#ifdef NDEBUG
TEST_F(ConstraintTableTestOld, DISABLED_WrongSafeInterval)
#else
TEST_F(ConstraintTableTestOld, WrongSafeInterval)
#endif
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 1}), TimePoint(7, {2, 4})};
  table->add_constraints(path1);

  // expect assert death
  EXPECT_DEATH(table->get_safe_intervals(4, 7, {0, 3}), "Invalid time interval, the from location will be in edge collision.");
}

// check assert death when the safe interval of to position is not right and there are more constraints in the list
#ifdef NDEBUG
TEST_F(ConstraintTableTestOld, DISABLED_WrongSafeIntervalMoreConstraints)
#else
TEST_F(ConstraintTableTestOld, WrongSafeIntervalMoreConstraints)
#endif
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 1}), TimePoint(7, {2, 4}), TimePoint(4, {5, 6}), TimePoint(7, {7, 8}),
                                  TimePoint(4, {9, 10})};
  table->add_constraints(path1);

  // expect assert death
  EXPECT_DEATH(table->get_safe_intervals(4, 7, {0, 3}), "Invalid time interval, the from location will be in edge collision.");
}

// check that the edge conflict is resolved correctly when there are more constraints in the list
TEST_F(ConstraintTableTestOld, EdgeConflictMoreConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 1}), TimePoint(7, {2, 4}), TimePoint(4, {5, 6}), TimePoint(7, {7, 8}),
                                  TimePoint(4, {9, 10})};
  table->add_constraints(path1);

  // test going to 1, 1 from a location with edge conflict in time 0 - 2
  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals(4, 7, {0, 2});
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe intervals due to edge conflict.";
}

// test edge constraint table
// add to edge constraint table
TEST_F(ConstraintTableTestOld, EdgeConstraintTableAdd)
{
  // add a constraint to the table
  table->edge_constraint_table.add(4, 7, 3);

  // check that the constraint is in the table
  EXPECT_TRUE(table->edge_constraint_table.get(4, 7, 3));

  // check that the other times are not there
  EXPECT_FALSE(table->edge_constraint_table.get(4, 7, 2));
  EXPECT_FALSE(table->edge_constraint_table.get(4, 7, 4));
}

// remove from edge constraint table
TEST_F(ConstraintTableTestOld, EdgeConstraintTableRemove)
{
  // add a constraint to the table
  table->edge_constraint_table.add(1, 2, 3);

  EXPECT_TRUE(table->edge_constraint_table.get(1, 2, 3));

  // remove the constraint
  table->edge_constraint_table.remove(1, 2, 3);

  // check that the constraint is not in the table
  EXPECT_FALSE(table->edge_constraint_table.get(1, 2, 3));
}

// add an existing edge constraint
TEST_F(ConstraintTableTestOld, EdgeConstraintTableAddExisting)
{
  // add a constraint to the table
  table->edge_constraint_table.add(1, 2, 3);

  // expect assert death
  EXPECT_DEATH(table->edge_constraint_table.add(1, 2, 3), "Edge constraint already exists.");
}

// add an invalid edge constraint
TEST_F(ConstraintTableTestOld, EdgeConstraintTableAddInvalid)
{
  // expect assert death
  EXPECT_DEATH(table->edge_constraint_table.add(1, 1, 2), "Invalid edge constraint.");
}

