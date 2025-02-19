/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#pragma once
#include <utils.h>

#include "ConstraintTable.h"

class SIPP
{
public:
  ConstraintTable constraint_table;
  int             generated_nodes = 0;

  // constructor
  SIPP(const Instance& instance);

  // destructor
  ~SIPP() = default;

  TimePointPath plan(const int agent_num);

private:
  const Instance& instance;
};
