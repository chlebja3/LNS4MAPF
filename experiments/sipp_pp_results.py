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
        plt.plot([x_positions[i], x_positions[i]], [min_values[i], max_values[i]], color='gray', linestyle='--', zorder=3)

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
df = pd.read_sql_query("SELECT * FROM experiments", conn)
df["delay"] = df["sum_of_cost"] - df["sum_of_dist"]

# close the database
conn.close()

stats = df.groupby("algo_name").agg({
    "computation_time_cpu": ["mean", "std", "median", "min", "max"],
    "computation_time_wall": ["mean", "std", "median", "min", "max"],
    "expanded": ["mean", "std", "median", "min", "max"],
    "generated": ["mean", "std", "median", "min", "max"],
    "makespan": ["mean", "std", "median", "min", "max"],
    "sum_of_cost": ["mean", "std", "median", "min", "max"],
    "sum_of_dist": ["mean", "std", "median", "min", "max"],
    "delay": ["mean", "std", "median", "min", "max"],
    "feasible" : ["mean"] 
}).reset_index()

# the algorithms

algo_names = df.groupby("algo_name").groups.keys()
metrics = ["computation_time_cpu", "computation_time_wall", "expanded", "generated", "makespan", "sum_of_cost", "delay"]
for metric in metrics:
    plot_algorithm_metrics(algo_names, stats[metric]["mean"], stats[metric]["median"], stats[metric]["std"], stats[metric]["min"], stats[metric]["max"], metric)



