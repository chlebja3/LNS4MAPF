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

Computation::Computation(const Instance& instance_, SharedData& shared_data_, INFO_type info_type_, double w_ = 1.0, int seed_ = -1)
    : instance(instance_), shared_data(shared_data_), running(false), info_type(info_type_), w(w_), seed(seed_)
{
  // initialized the random generator
  if (seed < 0)
  {
    // random device to generate a random seed
    std::random_device rd;
    rnd_generator = std::mt19937(rd());
  }
  else
  {
    rnd_generator = std::mt19937(seed);
  }

  // initialize the solver
  solver = std::make_unique<LNS>(instance, rnd_generator, info_type, w);
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

  std::vector<Path> converted_paths(solver->solution.paths.size());
  for (int i = 0; i < (int)solver->solution.paths.size(); i++)
  {
    converted_paths[i] = timepointpath_to_path(solver->solution.paths[i]);
  }

  shared_data.update_paths(converted_paths, solver->sipp_info);

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
