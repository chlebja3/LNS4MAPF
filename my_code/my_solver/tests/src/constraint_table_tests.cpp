/*
 * Author: Jan Chleboun
 * Date: 26-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <gtest/gtest.h>

#include <climits>

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
// Adding a single constraint and checking if it is stored correctly
// old implementation
TEST_F(ConstraintTableTest, SingleConstraintOld)
{
  TimePoint tp(7, {3, 5});  // location (1,2) with time interval [3,5]
                            //
  table->add_constraint_old(tp);

  // test that the last constraint is right
  EXPECT_EQ(table->get_last_dynamic_constraint_old(), 3);

  // test that the first safe interval is right
  TimeInterval first_si = table->get_first_safe_interval_old(7);
  EXPECT_EQ(first_si, TimeInterval(0, 2));

  // test that the interval after the constraint is retrieved
  TimeInterval after_si = table->get_safe_interval_old(7, 4, 3);
  EXPECT_EQ(after_si, TimeInterval(6, INT_MAX));

  // test that both the interval before and after the constraint are retrieved
  std::vector<TimeInterval> safeIntervals = table->get_safe_intervals_old(7, 4, {0, 6});
  ASSERT_EQ(safeIntervals.size(), 2) << "Expected two safe intervals after adding a constraint!";
  EXPECT_EQ(safeIntervals[0], TimeInterval(0, 2));
  EXPECT_EQ(safeIntervals[1], TimeInterval(6, INT_MAX));

  // test that no interval is retrieved when the constraint is in the way
  safeIntervals = table->get_safe_intervals_old(7, 4, {2, 5});
  ASSERT_EQ(safeIntervals.size(), 1) << "Expected only one safe interval!";
  EXPECT_EQ(safeIntervals[0], TimeInterval(2, 2));
}

// the newer method
TEST_F(ConstraintTableTest, SingleConstraint)
{
  TimePoint tp(7, {3, 5});  // location (1,2) with time interval [3,5]
                            //
  table->add_constraint(tp);

  // test the estimate of the maximal path len
  table->update_max_path_len_estimate();
  EXPECT_EQ(table->get_max_path_len_estimate(), 15);

  // test that the first safe interval is right
  auto first_si = table->get_first_safe_interval(7);
  EXPECT_EQ(*first_si, TimeInterval(0, 2));

  // test that both the interval before and after the constraint are retrieved
  std::list<TimeInterval>::const_iterator start, end;
  std::tie(start, end) = table->get_safe_intervals(7, {0, 6});
  ASSERT_EQ(std::distance(start, end), 2) << "Expected two safe intervals after adding a constraint!";
  EXPECT_EQ(*start, TimeInterval(0, 2));
  EXPECT_EQ(*std::next(start), TimeInterval(6, INT_MAX));


  // test that no interval is retrieved when the constraint is in the way
  std::tie(start, end) = table->get_safe_intervals(7, {2, 5});
  ASSERT_EQ(std::distance(start, end), 1) << "Expected only one safe interval!";
  EXPECT_EQ(*start, TimeInterval(0, 2));
}

// Adding multiple constraints and ensuring safe intervals are computed correctly
TEST_F(ConstraintTableTest, MultipleSingleConstraintsOld)
{
  table->add_constraint_old(TimePoint(7, {2, 4}));
  table->add_constraint_old(TimePoint(7, {5, 5}));
  table->add_constraint_old(TimePoint(7, {6, 7}));
  table->add_constraint_old(TimePoint(7, {10, 15}));

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint_old(), 10);

  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals_old(7, 0, {0, 20});
  ASSERT_EQ(safe_intervals.size(), 3) << "Expected three safe intervals after adding multiple constraints!";
  EXPECT_EQ(safe_intervals[0], TimeInterval(0, 1));
  EXPECT_EQ(safe_intervals[1], TimeInterval(8, 9));
  EXPECT_EQ(safe_intervals[2], TimeInterval(16, INT_MAX));
}

// the newer method
TEST_F(ConstraintTableTest, MultipleSingleConstraints)
{
  table->add_constraint(TimePoint(7, {2, 4}));
  table->add_constraint(TimePoint(7, {5, 5}));
  table->add_constraint(TimePoint(7, {6, 7}));
  table->add_constraint(TimePoint(7, {10, 15}));

  // test the estimate of the maximal path len
  table->update_max_path_len_estimate();
  EXPECT_EQ(table->get_max_path_len_estimate(), 25);

  std::list<TimeInterval>::const_iterator start, end;
  std::tie(start, end) = table->get_safe_intervals(7, {0, 20});
  ASSERT_EQ(std::distance(start, end), 3) << "Expected three safe intervals after adding multiple constraints!";
  EXPECT_EQ(*start, TimeInterval(0, 1));
  EXPECT_EQ(*std::next(start), TimeInterval(8, 9));
  EXPECT_EQ(*std::next(start, 2), TimeInterval(16, INT_MAX));
}

// Removing a constraint and ensuring safe intervals update
TEST_F(ConstraintTableTest, RemoveConstraintOld)
{
  TimePoint tp(7, {3, 5});
  table->add_constraint_old(tp);
  table->remove_constraint_old(tp);

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint_old(), 0);

  TimeInterval first_si = table->get_first_safe_interval_old(7);
  EXPECT_TRUE(first_si.t_min == 0 && first_si.t_max == INT_MAX);
}

TEST_F(ConstraintTableTest, RemoveConstraint)
{
  TimePoint tp(7, {3, 5});
  table->add_constraint(tp);
  table->remove_constraint(tp);

  // test the estimate of the maximal path len
  table->update_max_path_len_estimate();
  EXPECT_EQ(table->get_max_path_len_estimate(), 9);

  auto first_si = table->get_first_safe_interval(7);
  EXPECT_TRUE(first_si->t_min == 0 && first_si->t_max == INT_MAX);

  std::list<TimeInterval>::const_iterator start, end;
  std::tie(start, end) = table->get_safe_intervals(7, {0, INT_MAX});
  ASSERT_EQ(std::distance(start, end), 1) << "Expected one safe interval after removing a constraint!";
  EXPECT_EQ(*start, TimeInterval(0, INT_MAX));
}

// Querying safe interval for an empty position
TEST_F(ConstraintTableTest, GetSafeIntervalEmptyOld)
{
  TimeInterval first_si = table->get_first_safe_interval_old(4);
  EXPECT_TRUE(first_si.t_min == 0 && first_si.t_max == INT_MAX) << "No constraints should mean an unlimited safe interval.";
}

// the other method
TEST_F(ConstraintTableTest, GetSafeIntervalEmpty)
{
  auto first_si = table->get_first_safe_interval(4);
  EXPECT_TRUE(first_si->t_min == 0 && first_si->t_max == INT_MAX) << "No constraints should mean an unlimited safe interval.";

  std::list<TimeInterval>::const_iterator start, end;
  std::tie(start, end) = table->get_safe_intervals(4, {0, INT_MAX});
  ASSERT_EQ(std::distance(start, end), 1) << "Expected one safe interval for an empty position!";
  EXPECT_EQ(*start, TimeInterval(0, INT_MAX));
}


// Handling constraints dynamically (adding and removing in sequence)
TEST_F(ConstraintTableTest, DynamicAddRemoveOld)
{
  table->add_constraint_old(TimePoint(4, {2, 4}));
  table->add_constraint_old(TimePoint(4, {6, 8}));
  table->remove_constraint_old(TimePoint(4, {2, 4}));

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint_old(), 6);

  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals_old(4, 0, {0, 10});
  ASSERT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals after removal.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(0, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(9, INT_MAX));
}

// the other method
TEST_F(ConstraintTableTest, DynamicAddRemove)
{
  table->add_constraint(TimePoint(4, {2, 4}));
  table->add_constraint(TimePoint(4, {6, 8}));
  table->remove_constraint(TimePoint(4, {2, 4}));

  // test the estimate of the maximal path len
  table->update_max_path_len_estimate();
  EXPECT_EQ(table->get_max_path_len_estimate(), 18);

  std::list<TimeInterval>::const_iterator start, end;
  std::tie(start, end) = table->get_safe_intervals(4, {0, 10});
  ASSERT_EQ(std::distance(start, end), 2) << "Expected two safe intervals after removal.";
  EXPECT_EQ(*start, TimeInterval(0, 5));
  EXPECT_EQ(*std::next(start), TimeInterval(9, INT_MAX));
}

// Constraint removal from empty table if in debug mode
#ifdef NDEBUG
TEST_F(ConstraintTableTest, DISABLED_RemoveFromEmptyTable)
#else
TEST_F(ConstraintTableTest, RemoveFromEmptyTable)
#endif
{
  TimePoint tp(4, {3, 6});
  // expect assert death
  EXPECT_DEATH(table->remove_constraint_old(tp), "Trying to remove interval from an empty list.");
  EXPECT_DEATH(table->remove_constraint(tp), "Constraint interval can not have any overlap with safe interval.");
}

// Constraint removal when no constraint exists
TEST_F(ConstraintTableTest, RemoveNonExistentConstraint)
{
  TimePoint tp(4, {3, 6});
  // add a dummy constraint to prevent assertion failure removing from empty table
  table->add_constraint_old(TimePoint(4, {0, 2}));
  try
  {
    table->remove_constraint_old(tp);
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

  // test the second method
  EXPECT_DEATH(table->remove_constraint(tp), "Constraint interval can not have any overlap with safe interval.");
}

// Test 8: Adding a constraint that is already present (should cause assert death)
#ifdef NDEBUG
TEST_F(ConstraintTableTest, DISABLED_AddExistingConstraint)
#else
TEST_F(ConstraintTableTest, AddExistingConstraint)
#endif
{
  TimePoint tp(4, {3, 6});
  table->add_constraint_old(tp);
  ASSERT_DEATH(table->add_constraint_old(tp), "Cannot add overlapping constraints.");
  table->add_constraint(tp);
  ASSERT_DEATH(table->add_constraint(tp), "Can not add an overlapping constraint.");
}

// Adding a path that forms constraints, test whether edge constraints are added
TEST_F(ConstraintTableTest, AddPathConstraintsOld)
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 2}), TimePoint(7, {3, 4}), TimePoint(8, {5, 6})};
  table->add_constraints_old(path1);
  std::vector<TimePoint> path2 = {TimePoint(5, {0, 5}), TimePoint(4, {6, 8}), TimePoint(7, {9, 10})};
  table->add_constraints_old(path2);

  // test that the last constraint end is right
  EXPECT_EQ(table->get_last_dynamic_constraint_old(), 9);

  // test first edge conflict
  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals_old(4, 7, {0, 3});
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe interval due to an edge conflict.";

  // test going to 1, 1 from a non constrained position
  safe_intervals = table->get_safe_intervals_old(4, 1, {0, 10});
  ASSERT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals after adding path constraints.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(3, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(9, INT_MAX));

  // test second edge conflict
  safe_intervals = table->get_safe_intervals_old(4, 7, {5, 9});
  EXPECT_EQ(safe_intervals.size(), 1) << "Expected only one safe interval.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(5, 5));
}

// Adding a path that forms constraints, test whether edge constraints are added
TEST_F(ConstraintTableTest, AddPathConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 2}), TimePoint(7, {3, 4}), TimePoint(8, {5, 6})};
  table->add_constraints(path1);
  std::vector<TimePoint> path2 = {TimePoint(5, {0, 5}), TimePoint(4, {6, 8}), TimePoint(7, {9, 10})};
  table->add_constraints(path2);

  // test the estimate of the maximal path len
  table->update_max_path_len_estimate();
  EXPECT_EQ(table->get_max_path_len_estimate(), 20);

  // test first edge conflict
  std::list<TimeInterval>::const_iterator start, end;
  std::tie(start, end) = table->get_safe_intervals(4, {0, 3});
  ASSERT_EQ(std::distance(start, end), 0) << "Expected no safe interval due to an edge conflict.";
  // TODO TEST EDGE CONFLICT

  // test going to 1, 1 from a non constrained position
  std::tie(start, end) = table->get_safe_intervals(4, {0, 10});
  ASSERT_EQ(std::distance(start, end), 2) << "Expected two safe intervals after adding path constraints.";
  EXPECT_EQ(*start, TimeInterval(3, 5));
  EXPECT_EQ(*std::next(start), TimeInterval(9, INT_MAX));

  // test second edge conflict
  std::tie(start, end) = table->get_safe_intervals(4, {5, 9});
  ASSERT_EQ(std::distance(start, end), 1) << "Expected only one safe interval.";
  EXPECT_EQ(*start, TimeInterval(5, 5));
  // TODO TEST EDGE CONFLICT
}

// check assert death when the time interval is wrong, because, there will be an edge conflict if an agent came from the location
#ifdef NDEBUG
TEST_F(ConstraintTableTest, DISABLED_WrongSafeInterval)
#else
TEST_F(ConstraintTableTest, WrongSafeInterval)
#endif
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 1}), TimePoint(7, {2, 4})};
  table->add_constraints(path1);

  // expect assert death
  EXPECT_DEATH(table->get_safe_intervals_old(4, 7, {0, 3}), "Invalid time interval, the from location will be in edge collision.");

  // the other method TODO test edge collision
  EXPECT_DEATH(table->get_safe_intervals(4, {0, 3}), "Invalid time interval, the from location will be in edge collision.");
}

// check assert death when the safe interval of to position is not right and there are more constraints in the list
#ifdef NDEBUG
TEST_F(ConstraintTableTest, DISABLED_WrongSafeIntervalMoreConstraints)
#else
TEST_F(ConstraintTableTest, WrongSafeIntervalMoreConstraints)
#endif
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 1}), TimePoint(7, {2, 4}), TimePoint(4, {5, 6}), TimePoint(7, {7, 8}),
                                  TimePoint(4, {9, 10})};
  table->add_constraints_old(path1);

  // expect assert death
  EXPECT_DEATH(table->get_safe_intervals_old(4, 7, {0, 3}), "Invalid time interval, the from location will be in edge collision.");

  // the other method
  EXPECT_DEATH(table->get_safe_intervals(4, {0, 3}), "Invalid time interval, the from location will be in edge collision.");
}

// check that the edge conflict is resolved correctly when there are more constraints in the list
TEST_F(ConstraintTableTest, EdgeConflictMoreConstraintsOld)
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 1}), TimePoint(7, {2, 4}), TimePoint(4, {5, 6}), TimePoint(7, {7, 8}),
                                  TimePoint(4, {9, 10})};
  table->add_constraints_old(path1);

  // test going to 1, 1 from a location with edge conflict in time 0 - 2
  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals_old(4, 7, {0, 2});
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe intervals due to edge conflict.";
}


// check that the edge conflict is resolved correctly when there are more constraints in the list
TEST_F(ConstraintTableTest, EdgeConflictMoreConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint(4, {0, 1}), TimePoint(7, {2, 4}), TimePoint(4, {5, 6}), TimePoint(7, {7, 8}),
                                  TimePoint(4, {9, 10})};
  table->add_constraints(path1);

  std::list<TimeInterval>::const_iterator start, end;
  std::tie(start, end) = table->get_safe_intervals(4, {0, 2});
  ASSERT_EQ(std::distance(start, end), 0) << "Expected no safe intervals due to edge conflict.";
}

