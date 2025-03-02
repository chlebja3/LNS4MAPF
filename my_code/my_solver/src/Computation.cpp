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

Computation::Computation(const Instance& instance_, SharedData& shared_data_, bool is_visualization_, double w_ = 1.0)
    : instance(instance_), shared_data(shared_data_), running(false), is_visualization(is_visualization_), w(w_)
{
  std::cout << "Initialized computation." << std::endl;

  // initialize the solver
  solver = std::make_unique<LNS>(instance, is_visualization, w);
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
  // get the initial solution
  solver->find_initial_solution();

  shared_data.update_paths(solver->solution.paths, solver->sipp_info);

  std::cout << "Initial solution found, generated nodes: " << solver->get_num_of_generated_nodes()
            << ", expanded nodes: " << solver->get_num_of_expanded_nodes() << std::endl;

  // while (running)
  // {
  //   std::cout << "Computation thread running" << std::endl;
  //   i++;
  //   if (i >= 1) running = false;
  //   std::this_thread::sleep_for(std::chrono::seconds(3));
  // }
  std::cout << "Computation thread ended" << std::endl;
}
