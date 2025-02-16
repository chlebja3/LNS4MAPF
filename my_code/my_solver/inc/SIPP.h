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
  // constructor
  SIPP(const std::shared_ptr<Instance> instance);
  SIPP(const Map& map_data, const ConstraintTable& constraint_table);

  // destructor
  ~SIPP() = default;

  Path plan(const Point2d& start, const Point2d& goal);

private:
  const Map&      map_data;
  ConstraintTable constraint_table;
};
