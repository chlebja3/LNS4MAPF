/*
 * Author: Jan Chleboun
 * Date: 12-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <list>
#include <string>

#include "ConstraintTable.h"
#include "utils.h"

auto get_base_path_tests() -> std::string;

auto generate_random_timepointpath(int len) -> TimePointPath;

auto filter_time_intervals(std::pair<std::list<TimeInterval>::const_iterator, std::list<TimeInterval>::const_iterator> start_goal,
                           TimeInterval interval, int from, int to, ConstraintTable& table) -> std::vector<TimeInterval>;
