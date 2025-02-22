/*
 * Author: Jan Chleboun
 * Date: 20-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include "SharedData.h"

#include <iostream>

SharedData::SharedData() : are_new_paths(false)
{
  std::cout << "Initialized shared data." << std::endl;
}

void SharedData::update_paths(const std::vector<Path>& new_paths, std::vector<SIPPInfo>& new_sipp_info)
{
  // lock  the mutex
  std::lock_guard<std::mutex> lock(paths_mtx);

  // update the paths
  paths = new_paths;

  // update the info
  sipp_info = std::move(new_sipp_info);
  new_sipp_info.clear();

  // signal that new data arrived
  are_new_paths.store(true, std::memory_order_release);
}

std::pair<std::vector<Path>, std::vector<SIPPInfo>> SharedData::consume_paths(void)
{
  assertm(are_new_paths, "Trying to consume paths when there are no paths.");
  // lock  the mutex
  std::lock_guard<std::mutex> lock(paths_mtx);

  // signal that the paths have been consumed
  are_new_paths.store(false, std::memory_order_release);

  // return the paths (which consumes them)
  return {std::move(paths), std::move(sipp_info)};
}

