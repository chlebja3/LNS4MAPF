/*
 * Author: Jan Chleboun
 * Date: 16-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gtest/gtest.h>
#include <limits.h>

#include "ConstraintTable.h"
#include "Instance.h"
#include "test_utils.h"

// Constraint Table tests
class ConstraintTableTest : public ::testing::Test
{
protected:
  std::shared_ptr<Instance>        instance;
  std::unique_ptr<ConstraintTable> table;

  void SetUp() override
  {
    std::cout << "Setting up ConstraintTableTest" << std::endl;
    // load instance
    std::string base_path = get_base_path_tests();  // path to my_solver

    instance =
        std::make_shared<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);
    table = std::make_unique<ConstraintTable>(instance);
    std::cout << "table pointer: " << table.get() << std::endl;
  }

  void TearDown() override
  {
    std::cout << "Tearing down ConstraintTableTest" << std::endl;
  }
};

// Test 1: Adding a single constraint and checking if it is stored correctly
TEST_F(ConstraintTableTest, SingleConstraint)
{
  std::cout << "Starting SingleConstraint test" << std::endl;
  TimePoint tp({1, 2}, {3, 5});  // Position (1,2) with time interval [3,5]
  std::cout << "table pointer: " << table.get() << std::endl;
  table->add_constraint(tp);
  std::cout << "Constraint added" << std::endl;

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

  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals({1, 1}, {0, 0}, {0, 10});
  ASSERT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals after removal.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(0, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(9, INT_MAX));
}

// Test 6: Constraint removal when no constraint exists
TEST_F(ConstraintTableTest, RemoveNonExistentConstraint)
{
  TimePoint tp({1, 1}, {3, 6});
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

// Test 7: Adding a constraint that is already present (should cause assert death)
TEST_F(ConstraintTableTest, AddExistingConstraint)
{
  TimePoint tp({1, 1}, {3, 6});
  table->add_constraint(tp);
  ASSERT_DEATH(table->add_constraint(tp), "Cannot add overlapping constraints.");
}

// Test 8: Adding a path that forms constraints, test whether edge constraints are added
TEST_F(ConstraintTableTest, AddPathConstraints)
{
  std::vector<TimePoint> path1 = {TimePoint({1, 1}, {0, 2}), TimePoint({1, 2}, {3, 4}), TimePoint({1, 3}, {5, 6})};
  table->add_constraints(path1);
  std::vector<TimePoint> path2 = {TimePoint({2, 1}, {0, 5}), TimePoint({1, 1}, {6, 8}), TimePoint({1, 2}, {9, 10})};
  table->add_constraints(path2);

  // test first edge conflict
  std::vector<TimeInterval> safe_intervals = table->get_safe_intervals({1, 1}, {1, 2}, {0, 3});
  EXPECT_EQ(safe_intervals.size(), 0) << "Expected no safe interval due to an edge conflict.";

  // test going to 1, 1 from a non constrained position
  safe_intervals = table->get_safe_intervals({1, 1}, {1, 0}, {0, 10});
  ASSERT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals after adding path constraints.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(3, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(9, INT_MAX));

  // test second edge conflict
  safe_intervals = table->get_safe_intervals({1, 1}, {1, 2}, {5, 10});
  EXPECT_EQ(safe_intervals.size(), 2) << "Expected two safe intervals.";
  EXPECT_EQ(safe_intervals[0], TimeInterval(5, 5));
  EXPECT_EQ(safe_intervals[1], TimeInterval(10, INT_MAX));
}

