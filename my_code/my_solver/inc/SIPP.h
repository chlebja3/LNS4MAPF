/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#pragma once
#include <utils.h>

class SIPP
{
public:
  // constructor
  SIPP(const Map& map_data);

  // destructor
  ~SIPP() = default;

  Path plan(const Point2d& start, const Point2d& goal, const Map& map_data);

private:
  const Map& map_data;
};
