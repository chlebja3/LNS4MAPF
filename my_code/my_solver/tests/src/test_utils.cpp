/*
 * Author: Jan Chleboun
 * Date: 12-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "test_utils.h"

#include <filesystem>

std::string get_base_path_tests()
{
  return std::filesystem::canonical("/proc/self/exe").parent_path().parent_path().parent_path().string();  // Linux only
}
