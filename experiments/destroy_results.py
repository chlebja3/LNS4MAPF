import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np
import json
import re

def str_vec_to_float_vec(vec):
    return [float(x) for x in vec]

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
df = pd.read_sql_query("SELECT * FROM destroy", conn)

# close the database
conn.close()

# convert the json strings to vectors vector
df['iteration_time_cpu'] = df['iteration_time_cpu'].apply(json.loads)
df['iteration_time_wall'] = df['iteration_time_wall'].apply(json.loads)
df['makespan'] = df['makespan'].apply(json.loads)
df['sum_of_cost'] = df['sum_of_cost'].apply(json.loads)

df_feasible = df[df["feasible"] == 1]

group_sizes = df_feasible.groupby(["map_name", "num_agents", "seed"]).size()
total_algorithms = df["algo_name"].nunique()
valid_groups = group_sizes[group_sizes == total_algorithms].index
df_filtered = df_feasible.set_index(["map_name", "num_agents", "seed"]).loc[valid_groups]

df_feasible_grouped = df_filtered.groupby(["map_name", "num_agents", "algo_name"])

# Define the metrics to plot
metric_names = ["sum_of_cost"]

# Get unique maps
unique_maps = df["map_name"].unique()
num_maps = len(unique_maps)

neighborhood_sizes = [4, 8, 16]
# the influence of adaptivness
fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(10, 10), sharey=False)
axes = axes.flatten()

if num_maps == 1:  # If only one map, axes is not an array
    axes = [axes]

# Iterate over each map
for i, (map_name, map_group) in enumerate(df_filtered.groupby('map_name')):
    # print("map_name:", map_name)
    ax = axes[i]

    # Plot each algorithm
    for algo_name, algo_group in map_group.groupby('algo_name'):
        if "Adaptive" in algo_name or "ADAPTIVE" in algo_name or "RANDOM_CHOOSE" in algo_name:
            last_int = int(re.search(r'_([0-9]+)$', algo_name).group(1))
            if (last_int != 8):
                continue
            metric_list = algo_group["sum_of_cost"].to_list()
            min_len = np.min(np.asarray([len(x) for x in metric_list]))
            metric_list = [x[:min_len] for x in metric_list]
            metric_np = np.asarray(metric_list)
            mean_vec = np.mean(metric_np, axis=0)
            algo_name_shorter = algo_name.replace("_ORIGO", '')
            algo_name_shorter = algo_name_shorter.replace("neighborhood", 'neigh')
            ax.plot(np.asarray(range(mean_vec.shape[0])), mean_vec, label=f"{algo_name_shorter}", linewidth=2)

    ax.set_title(map_name)
    ax.set_xlabel("Iteration")
    ax.set_ylabel("sum_of_cost")  # Only label Y-axis on first subplot

    # ax.legend()
    # ax.legend(loc='center down', bbox_to_anchor=(0.5, 1))

    ax.grid(True)

# Put a legend below current axis
handles, labels = axes[0].get_legend_handles_labels()
fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=2)
plt.tight_layout()
plt.subplots_adjust(bottom=0.20)
# plt.tight_layout()
#plt.subplots_adjust(bottom=0.2)  # Add vertical space for the legend

# fig.suptitle(f"{metric} Across Maps", fontsize=16)
#plt.tight_layout()
plt.show()

# Iterate over each metric
for metric in metric_names:

    # iterate over neighborhood sizes
    for neighborhood_size in neighborhood_sizes:
        fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(10, 10), sharey=False)
        axes = axes.flatten()

        if num_maps == 1:  # If only one map, axes is not an array
            axes = [axes]

        # Iterate over each map
        for i, (map_name, map_group) in enumerate(df_filtered.groupby('map_name')):
            # print("map_name:", map_name)
            ax = axes[i]

            # Plot each algorithm
            for algo_name, algo_group in map_group.groupby('algo_name'):
                if "Adaptive" in algo_name or "ADAPTIVE" in algo_name or "RANDOM_CHOOSE" in algo_name: continue
                last_int = int(re.search(r'_([0-9]+)$', algo_name).group(1))
                if (last_int != neighborhood_size):
                    continue
                metric_list = algo_group[metric].to_list()
                min_len = np.min(np.asarray([len(x) for x in metric_list]))
                metric_list = [x[:min_len] for x in metric_list]
                metric_np = np.asarray(metric_list)
                mean_vec = np.mean(metric_np, axis=0)
                algo_name_shorter = algo_name.replace("_ORIGO", '')
                algo_name_shorter = algo_name_shorter.replace("neighborhood", 'neigh')
                ax.plot(np.asarray(range(mean_vec.shape[0])), mean_vec, label=f"{algo_name_shorter}", linewidth=2)
                # print("Algo name:", algo_name)
                # print(algo_group[(metric, "mean")])

            ax.set_title(map_name)
            ax.set_xlabel("Iteration")
            ax.set_ylabel(metric)  # Only label Y-axis on first subplot

            # ax.legend()
            # ax.legend(loc='center down', bbox_to_anchor=(0.5, 1))

            ax.grid(True)

# Put a legend below current axis
        handles, labels = axes[0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=2)
        plt.tight_layout()
        plt.subplots_adjust(bottom=0.20)
        # plt.tight_layout()
        #plt.subplots_adjust(bottom=0.2)  # Add vertical space for the legend

        # fig.suptitle(f"{metric} Across Maps", fontsize=16)
        #plt.tight_layout()
        plt.show()


