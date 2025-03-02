/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <atomic>
#include <memory>
#include <thread>

#include "Instance.h"
#include "LNS.h"
#include "SharedData.h"

class Computation
{
public:
  // constructor and destructor
  Computation(const Instance& instance_, SharedData& shared_data_, bool is_visualization_, double w_);
  ~Computation();

  // manage the underlying thread
  void start();
  void stop();
  void join_thread();

  // the function called in the thread
  void run();

private:
  std::unique_ptr<LNS> solver;
  std::thread          comp_thread;
  const Instance&      instance;
  SharedData&          shared_data;
  std::atomic<bool>    running;
  const bool           is_visualization;
  double               w;
};
