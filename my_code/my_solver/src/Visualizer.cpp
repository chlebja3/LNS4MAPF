#include "Visualizer.h"
#include <iostream>

Visualizer::Visualizer(Instance& instance):instance(instance)
{
  running = false;
  std::cout << "Initialized visualizer." << std::endl;
}

Visualizer::~Visualizer()
{
  stop();
  join_thread();
}

void Visualizer::start()
{
  // start the underlying thread
  vis_thread = std::thread(&Visualizer::run, this);
  running = true;
}

void Visualizer::stop()
{
  // set the stop signal
  running = false;
}

void Visualizer::join_thread()
{
  if (vis_thread.joinable()) 
  {
    vis_thread.join();
  }
}

void Visualizer::run()
{
  int i = 0;
  while (running)
  {
    std::cout << "Visualizer thread running" << std::endl;
    i++;
    if (i >= 3) running = false;
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
  std::cout << "Visualizer thread ended" << std::endl;
}
