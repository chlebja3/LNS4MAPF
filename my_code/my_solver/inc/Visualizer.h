#pragma once
#include <thread>
#include <atomic>
#include "Instance.h"

class Visualizer
{
  public:
    Instance& instance;
	
    // constructor and destructor
    Visualizer(Instance& instance);
    ~Visualizer();

    // manage the underlying thread
    void start();
    void stop();
    void join_thread();

    // the function called in the thread
    void run();

  private:
    std::thread vis_thread;
    std::atomic<bool> running;

};
