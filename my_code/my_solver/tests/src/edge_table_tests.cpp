/*
 * Author: Jan Chleboun
 * Date: 26-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <gtest/gtest.h>

#include <climits>

#include "EdgeTable.h"
#include "Instance.h"
#include "LNS.h"
#include "SIPP.h"
#include "test_utils.h"

/**
 * @brief Class for testing the EdgeTable class
 */
class EdgeTableTest : public ::testing::Test
{
protected:
  std::unique_ptr<Instance>  instance;
  std::unique_ptr<EdgeTable> edge_table;

  void SetUp() override
  {
    // load instance
    std::string base_path = get_base_path_tests();  // path to my_solver

    instance =
        std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);
    edge_table = std::make_unique<EdgeTable>(*instance);
  }

  void TearDown() override
  {
  }
};

// test edge constraint edge_table
// add to edge constraint edge_table
TEST_F(EdgeTableTest, Add)
{
  // add a constraint to the edge_table
  edge_table->add(6, 7, 3, 0);

  // check that the constraint is in the edge_table
  EXPECT_TRUE(edge_table->get(6, 7, 3));

  // check that the other times are not there
  EXPECT_FALSE(edge_table->get(6, 7, 2));
  EXPECT_FALSE(edge_table->get(6, 7, 4));
}

// remove from edge constraint edge_table
TEST_F(EdgeTableTest, Remove)
{
  // add a constraint to the edge_table
  edge_table->add(1, 2, 3, 0);

  EXPECT_TRUE(edge_table->get(1, 2, 3));

  // remove the constraint
  edge_table->remove(1, 2, 3);

  // check that the constraint is not in the edge_table
  EXPECT_FALSE(edge_table->get(1, 2, 3));
}

// add an existing edge constraint
#ifdef NDEBUG
TEST_F(EdgeTableTest, DISABLED_AddExisting)
#else
TEST_F(EdgeTableTest, AddExisting)
#endif
{
  // add a constraint to the edge_table
  edge_table->add(1, 2, 3, 0);

  // expect assert death
  EXPECT_DEATH(edge_table->add(1, 2, 3, 0), "Edge constraint already exists.");
}

// add an invalid edge constraint
#ifdef NDEBUG
TEST_F(EdgeTableTest, DISABLED_AddInvalid)
#else
TEST_F(EdgeTableTest, AddInvalid)
#endif
{
  // expect assert death
  EXPECT_DEATH(edge_table->add(1, 1, 2, 0), "Invalid edge constraint.");
}

