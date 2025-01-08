/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <iostream>
#include <chrono>
#include "Computation.h"

Computation::Computation(Instance& instance) : instance(instance)
{
  std::cout << "Initialized computation." << std::endl;
  running = false;
}

Computation::~Computation()
{
  stop();
  join_thread();
}

void Computation::start()
{
  // start the underlying thread
  comp_thread = std::thread(&Computation::run, this);
  running     = true;
}

void Computation::stop()
{
  // set the stop signal
  running = false;
}

void Computation::join_thread()
{
  if (comp_thread.joinable())
    {
      comp_thread.join();
    }
}

void Computation::run()
{
  int i = 0;
  while (running)
    {
      std::cout << "Computation thread running" << std::endl;
      i++;
      if (i >= 3)
        running = false;
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  std::cout << "Computation thread ended" << std::endl;
}
