/*
 * Author: Jan Chleboun
 * Date: 26-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <gtest/gtest.h>

#include <climits>

#include "ConstraintTable.h"
#include "Instance.h"
#include "LNS.h"
#include "SIPP.h"
#include "SafeIntervalTable.h"
#include "test_utils.h"

/**
 * @brief Class for testing the SafeIntervalTable class
 */
class SafeIntervalTableTest : public ::testing::Test
{
protected:
  std::unique_ptr<Instance> instance;
  std::unique_ptr<SafeIntervalTable> table;
  std::unique_ptr<ConstraintTable> constraint_table;

  void SetUp() override
  {
    // load instance
    std::string base_path = get_base_path_tests();  // path to my_solver

    instance = std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map",
                                          base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);
    table = std::make_unique<SafeIntervalTable>(*instance);
    constraint_table = std::make_unique<ConstraintTable>(*instance);
  }

  void TearDown() override {}
};

// Constraint Table tests
// Adding a single constraint and checking if it is stored correctly
TEST_F(SafeIntervalTableTest, SingleConstraint)
{
  TimePoint tp(7, {3, 5});  // location (1,2) with time interval [3,5]

  table->add_constraint(tp, 0, *constraint_table);

  // test that the first safe interval is right
  auto first_si = table->get_first_safe_interval(7).value()->interval;
  EXPECT_EQ(first_si, TimeInterval(0, 2));

  // test that both the interval before and after the constraint are retrieved
  auto [start, end] = table->get_safe_intervals(7, {0, 6});
  ASSERT_EQ(std::distance(start, end), 2) << "Expected two safe intervals after adding a constraint!";
  EXPECT_EQ(start->interval, TimeInterval(0, 2));
  EXPECT_EQ(std::next(start)->interval, TimeInterval(6, INT_MAX));

  // test that no interval is retrieved when the constraint is in the way
  std::tie(start, end) = table->get_safe_intervals(7, {2, 5});
  ASSERT_EQ(std::distance(start, end), 1) << "Expected only one safe interval!";
  EXPECT_EQ(start->interval, TimeInterval(0, 2));
}

// Adding multiple constraints and ensuring safe intervals are computed correctly
TEST_F(SafeIntervalTableTest, MultipleSingleConstraints)
{
  table->add_constraint(TimePoint(7, {2, 4}), 0, *constraint_table);
  table->add_constraint(TimePoint(7, {5, 5}), 0, *constraint_table);
  table->add_constraint(TimePoint(7, {6, 7}), 0, *constraint_table);
  table->add_constraint(TimePoint(7, {10, 15}), 0, *constraint_table);

  auto [start, end] = table->get_safe_intervals(7, {0, 20});
  ASSERT_EQ(std::distance(start, end), 3) << "Expected three safe intervals after adding multiple constraints!";
  EXPECT_EQ(start->interval, TimeInterval(0, 1));
  EXPECT_EQ(std::next(start)->interval, TimeInterval(8, 9));
  EXPECT_EQ(std::next(start, 2)->interval, TimeInterval(16, INT_MAX));
}

// Removing a constraint and ensuring safe intervals update
TEST_F(SafeIntervalTableTest, RemoveConstraint)
{
  TimePoint tp(7, {3, 5});
  table->add_constraint(tp, 0, *constraint_table);
  table->remove_constraint(tp, 0, *constraint_table);

  auto first_si = table->get_first_safe_interval(7).value();
  EXPECT_TRUE(first_si->interval.t_min == 0 && first_si->interval.t_max == INT_MAX);

  auto [start, end] = table->get_safe_intervals(7, {0, INT_MAX});
  ASSERT_EQ(std::distance(start, end), 1) << "Expected one safe interval after removing a constraint!";
  EXPECT_EQ(start->interval, TimeInterval(0, INT_MAX));
}

TEST_F(SafeIntervalTableTest, GetSafeIntervalEmpty)
{
  auto first_si = table->get_first_safe_interval(8).value();
  EXPECT_TRUE(first_si->interval.t_min == 0 && first_si->interval.t_max == INT_MAX)
      << "No constraints should mean an unlimited safe interval.";

  auto [start, end] = table->get_safe_intervals(8, {0, INT_MAX});
  ASSERT_EQ(std::distance(start, end), 1) << "Expected one safe interval for an empty position!";
  EXPECT_EQ(start->interval, TimeInterval(0, INT_MAX));
}

// Handling constraints dynamically (adding and removing in sequence)
TEST_F(SafeIntervalTableTest, DynamicAddRemove)
{
  TimePoint tp(7, {3, 5});
  table->add_constraint(TimePoint(2, {2, 4}), 0, *constraint_table);
  table->add_constraint(TimePoint(2, {6, 8}), 0, *constraint_table);
  table->remove_constraint(TimePoint(2, {2, 4}), 0, *constraint_table);

  auto [start, end] = table->get_safe_intervals(2, {0, 10});
  ASSERT_EQ(std::distance(start, end), 2) << "Expected two safe intervals after removal.";
  EXPECT_EQ(start->interval, TimeInterval(0, 5));
  EXPECT_EQ(std::next(start)->interval, TimeInterval(9, INT_MAX));
}

// Constraint removal from empty table if in debug mode
#ifdef NDEBUG
TEST_F(SafeIntervalTableTest, DISABLED_RemoveFromEmptyTable)
#else
TEST_F(SafeIntervalTableTest, RemoveFromEmptyTable)
#endif
{
  TimePoint tp(3, {3, 6});
  // expect assert death
  EXPECT_DEATH(table->remove_constraint(tp, 0, *constraint_table),
               "Constraint interval can not have any overlap with safe interval.");
}

// Constraint removal when no constraint exists
#ifdef NDEBUG
TEST_F(SafeIntervalTableTest, DISABLED_RemoveNonExistentConstraint)
#else
TEST_F(SafeIntervalTableTest, RemoveNonExistentConstraint)
#endif
{
  TimePoint tp(3, {3, 6});
  // test the second method
  EXPECT_DEATH(table->remove_constraint(tp, 0, *constraint_table),
               "Constraint interval can not have any overlap with safe interval.");
}

// Test 8: Adding a constraint that is already present (should cause assert death)
#ifdef NDEBUG
TEST_F(SafeIntervalTableTest, DISABLED_AddExistingConstraint)
#else
TEST_F(SafeIntervalTableTest, AddExistingConstraint)
#endif
{
  TimePoint tp(3, {3, 6});
  table->add_constraint(tp, 0, *constraint_table);
  ASSERT_DEATH(table->add_constraint(tp, 0, *constraint_table), "Can not add an overlapping constraint.");
}
