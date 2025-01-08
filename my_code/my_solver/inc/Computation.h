/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: 
 */

#pragma once
#include <thread>
#include <atomic>
#include "Instance.h"

class Computation
{
public:
  Instance& instance;

  // constructor and destructor
  Computation(Instance& instance);
  ~Computation();

  // manage the underlying thread
  void start();
  void stop();
  void join_thread();

  // the function called in the thread
  void run();

private:
  std::thread       comp_thread;
  std::atomic<bool> running;
};
