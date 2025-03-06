/*
 * Author: Jan Chleboun
 * Date: 06-03-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <random>

#include "Instance.h"
#include "indicators/indicators.hpp"
#include "utils.h"
#define PROGRESS_BAR_WIDTH 100

auto get_mapf_benchmark_path() -> std::string;

auto get_scene_names(const std::string& map_name, int runs_per_map) -> std::vector<std::string>;

auto create_progress_bar(std::string name, int max_iter) -> indicators::ProgressBar;

auto load_instances(const std::vector<std::string>& scenes, const std::string& map_name, int agent_num)
    -> std::pair<std::vector<Instance>, std::vector<std::pair<double, double>>>;

void save_experiment(nlohmann::json& experiment_res, const std::string& map_name, const std::string& scene_name,
                     const std::string& experiment_name, const std::string& algorithm_name);
