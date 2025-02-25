/*
 * Author: Jan Chleboun
 * Date: 20-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#pragma once
#include <atomic>
#include <mutex>

#include "IterInfo.h"
#include "utils.h"

class SharedData
{
public:
  std::atomic<bool> are_new_paths;

  SharedData(void);

  void update_paths(const std::vector<Path>& new_paths, std::vector<SIPPInfo>& sipp_info);

  std::pair<std::vector<Path>, std::vector<SIPPInfo>> consume_paths(void);

private:
  std::mutex            paths_mtx;
  std::vector<Path>     paths;
  std::vector<SIPPInfo> sipp_info;
};
