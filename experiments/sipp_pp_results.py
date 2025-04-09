import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np

def plot_algorithm_metrics(algo_names, avg_values, median_values, std_values, min_values, max_values, metric_name):

    plt.figure(figsize=(12, 9))

    # X positions for each algorithm
    x_positions = np.arange(len(algo_names))

    # Plot mean with error bars (std deviation)
    plt.errorbar(x_positions, avg_values, yerr=std_values, fmt='o', color='blue', label='Mean ± Std Dev', capsize=5, zorder = 5)

    # Plot median values as red triangles
    plt.scatter(x_positions, median_values, color='red', marker='x', label='Median', zorder=6)

    # Plot min/max as vertical whiskers
    for i in range(len(algo_names)):
        if i == 0:
            plt.plot([x_positions[i], x_positions[i]], [min_values[i], max_values[i]], color='gray', linestyle='--', label='range', zorder=3, linewidth=4)
        else:
            plt.plot([x_positions[i], x_positions[i]], [min_values[i], max_values[i]], color='gray', linestyle='--', zorder=3, linewidth=4)

    # Formatting
    plt.xticks(x_positions, algo_names, rotation=15)
    plt.ylabel(metric_name)
    plt.title(f"Comparison of {metric_name} across SIPP implementations")
    plt.legend()
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Show plot
    plt.show()

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
df = pd.read_sql_query("SELECT * FROM sipp_pp", conn)
df["delay"] = df["init_sol_sum_of_cost"] - df["sum_of_dist"]

# close the database
conn.close()


df_feasible = df[df["feasible"] == 1]

group_sizes = df_feasible.groupby(["map_name", "num_agents", "seed"]).size()
total_algorithms = df["algo_name"].nunique()
valid_groups = group_sizes[group_sizes == total_algorithms].index
df_filtered = df_feasible.set_index(["map_name", "num_agents", "seed"]).loc[valid_groups]

df_grouped = df.groupby(["map_name", "num_agents", "algo_name"])
# df_feasible_grouped = df_feasible.groupby(["map_name", "num_agents", "algo_name"])
df_feasible_grouped = df_filtered.groupby(["map_name", "num_agents", "algo_name"])
stats = df_grouped.agg({
    "experiment_time_cpu": ["mean", "std", "median", "min", "max"],
    "experiment_time_wall": ["mean", "std", "median", "min", "max"],
    "expanded": ["mean", "std", "median", "min", "max"],
    "generated": ["mean", "std", "median", "min", "max"],
    "init_sol_makespan": ["mean", "std", "median", "min", "max"],
    "init_sol_sum_of_cost": ["mean", "std", "median", "min", "max"],
    "sum_of_dist": ["mean", "std", "median", "min", "max"],
    "delay": ["mean", "std", "median", "min", "max"],
    "feasible" : ["mean"] 
}).reset_index()

stats_feasible = df_feasible_grouped.agg({
    "experiment_time_cpu": ["mean", "std", "median", "min", "max"],
    "experiment_time_wall": ["mean", "std", "median", "min", "max"],
    "expanded": ["mean", "std", "median", "min", "max"],
    "generated": ["mean", "std", "median", "min", "max"],
    "init_sol_makespan": ["mean", "std", "median", "min", "max"],
    "init_sol_sum_of_cost": ["mean", "std", "median", "min", "max"],
    "sum_of_dist": ["mean", "std", "median", "min", "max"],
    "delay": ["mean", "std", "median", "min", "max"],
    "feasible" : ["mean"] 
}).reset_index()

# Define the metrics to plot
metric_names = ["feasible", "experiment_time_cpu", "experiment_time_wall", "expanded", "init_sol_sum_of_cost", "delay"]

# Get unique maps
unique_maps = stats["map_name"].unique()
num_maps = len(unique_maps)

# Iterate over each metric
for metric in metric_names:
    fig, axes = plt.subplots(nrows=1, ncols=num_maps, figsize=(5 * num_maps, 5), sharey=False)

    if num_maps == 1:  # If only one map, axes is not an array
        axes = [axes]

    # Iterate over each map
    if metric == "feasible":
        for i, (map_name, map_group) in enumerate(stats.groupby('map_name')):
            ax = axes[i]

            # Plot each algorithm
            for algo_name, algo_group in map_group.groupby('algo_name'):
                ax.plot(algo_group["num_agents"], algo_group[(metric, "mean")], marker='o', label=f"{algo_name}")

            ax.set_title(map_name)
            ax.set_xlabel("Number of Agents")
            if i == 0:
                ax.set_ylabel(metric)  # Only label Y-axis on first subplot

            ax.legend()
            ax.grid(True)
    else:
        for i, (map_name, map_group) in enumerate(stats_feasible.groupby('map_name')):
            # print("map_name:", map_name)
            ax = axes[i]

            # Plot each algorithm
            for algo_name, algo_group in map_group.groupby('algo_name'):
                ax.plot(algo_group["num_agents"], algo_group[(metric, "mean")], marker='o', label=f"{algo_name}")
                # print("Algo name:", algo_name)
                # print(algo_group[(metric, "mean")])

            ax.set_title(map_name)
            ax.set_xlabel("Number of Agents")
            if i == 0:
                ax.set_ylabel(metric)  # Only label Y-axis on first subplot

            ax.legend()
            ax.grid(True)

    fig.suptitle(f"{metric} Across Maps", fontsize=16)
    plt.tight_layout()
    plt.show()


