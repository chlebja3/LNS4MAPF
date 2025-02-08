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

class Computation
{
public:
  // constructor and destructor
  Computation(std::shared_ptr<Instance> instance);
  ~Computation();

  // manage the underlying thread
  void start();
  void stop();
  void join_thread();

  // the function called in the thread
  void run();

private:
  std::unique_ptr<LNS>      solver;
  std::thread               comp_thread;
  std::shared_ptr<Instance> instance;
  std::atomic<bool>         running;
};
