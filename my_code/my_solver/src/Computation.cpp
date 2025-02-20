/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "Computation.h"

#include <chrono>
#include <iostream>

#include "LNS.h"

Computation::Computation(const Instance& instance, SharedData& shared_data) : instance(instance), shared_data(shared_data)
{
  std::cout << "Initialized computation." << std::endl;
  running = false;

  // initialize the solver
  solver = std::make_unique<LNS>(instance);
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

  // initialize LNS
  std::unique_ptr<LNS> lns = std::make_unique<LNS>(instance);

  // get the initial solution
  lns->find_initial_solution();

  shared_data.update_paths(lns->solution.paths);

  std::cout << "Initial solution found, number of nodes: " << lns->get_num_of_generated_nodes() << std::endl;

  // while (running)
  // {
  //   std::cout << "Computation thread running" << std::endl;
  //   i++;
  //   if (i >= 1) running = false;
  //   std::this_thread::sleep_for(std::chrono::seconds(3));
  // }
  std::cout << "Computation thread ended" << std::endl;
}
