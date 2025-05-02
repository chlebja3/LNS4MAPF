import pandas as pd
import sqlite3
import matplotlib.pyplot as plt
import numpy as np
import json
import re
from scipy.optimize import curve_fit
import itertools

def mean_of_runs(x): # x is a list of lists
    # make them equal length
    length = max(map(len, x))
    y=np.array([list(xi)+[np.nan]*(length-len(xi)) for xi in x])
    return np.nanmean(y, axis=0)

def average_runs(x, times, time_limit):
    times_np = np.asarray([t[0] for t in times])
    t_start = np.mean(times_np)
    t = np.linspace(t_start, time_limit, 1000)
    costs = np.array([np.interp(t, time, run) for time, run in zip(times, x)])
    average_costs = np.mean(costs, axis=0)
    
    return t, average_costs

def func(x, a, b, c):
    return a * np.exp(-b * x) + c

# def func(x, a, b, c):
#     return a * np.power(x, 2) + b * x + c 

def str_vec_to_float_vec(vec):
    return [float(x) for x in vec]

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Load data into Pandas DataFrame
df = pd.read_sql_query("SELECT * FROM overall_improvement", conn)

# close the database
conn.close()

# convert the json strings to vectors vector
df['iteration_time_cpu'] = df['iteration_time_cpu'].apply(json.loads)
df['iteration_time_wall'] = df['iteration_time_wall'].apply(json.loads)
df['time_cumsum'] = df['iteration_time_wall'].apply(np.cumsum)
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
x_axis_iteration = False
# Iterate over each metric
for metric in metric_names:

    # iterate over neighborhood sizes
    fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(9, 10), sharey=False)
    axes = axes.flatten()

    if num_maps == 1:  # If only one map, axes is not an array
        axes = [axes]

    # Iterate over each map
    for j, (map_name, map_group) in enumerate(df_filtered.groupby('map_name')):
        # print("map_name:", map_name)
        ax = axes[j]
        
        # Plot each algorithm
        for algo_name, algo_group in map_group.groupby('algo_name'):
            metric_list = algo_group[metric].to_list()
            time_list = algo_group["time_cumsum"].to_list()
            for i in range(len(metric_list)):
                x = np.asarray(metric_list[i])
                time = np.asarray(time_list[i])
                x_new = x[x != -1]
                time = time[x != -1]
                metric_list[i] = x_new.tolist()
                time_list[i] = time.tolist()
            if map_name == "empty-8-8":
                time_limit = 0.2
            elif map_name in {"ost003d", "den520d"}:
                time_limit = 30
            else:
                time_limit = 5
            mean_time, mean_run = average_runs(metric_list, time_list, time_limit)
            ax.plot(mean_time, mean_run, label=f"{algo_name}", linewidth=2.0)
            # ydata = list(itertools.chain(*algo_group[metric].to_list()))
            # xdata = list(itertools.chain(*algo_group["time_cumsum"].to_list()))
            # popt, pcov = curve_fit(func, xdata, ydata)
            # print(popt)
            # newx = np.linspace(0, 20, 1000)
            # ax.plot(newx, func(newx, *popt), label=f"{algo_name}")
            # metric_list = algo_group[metric].to_list()
            # # print(metric_list)
            # min_len = np.min(np.asarray([len(x) for x in metric_list]))
            # # print(algo_name, min_len, np.argmin(np.asarray([len(x) for x in metric_list])))
            # metric_list = [x[:min_len] for x in metric_list]
            # metric_np = np.asarray(metric_list)
            # print(metric_np.shape)
            # # exit()
            # mean_vec = np.mean(metric_np, axis=0)
            # if (x_axis_iteration):
            #     ax.plot(np.asarray(range(mean_vec.shape[0])), mean_vec, label=f"{algo_name}")
            # else:
            #     time_list = algo_group["iteration_time_wall"].to_list()  # TODO clipping and averaging time does not make sense
            #     min_len = np.min(np.asarray([len(x) for x in time_list]))
            #     time_list = [x[:min_len] for x in time_list]
            #     time_np = np.asarray(time_list)
            #     time_np = np.mean(time_np, axis=0)
            #     time_np = np.cumsum(time_np)
            #     print(time_np[-1])
            #     ax.plot(time_np, mean_vec, label=f"{algo_name}")
            # print("Algo name:", algo_name)
            # print(algo_group[(metric, "mean")])

        ax.set_title(map_name)
        ax.set_xlabel("Elapsed time (cpu) [s]")
        ax.set_ylabel(metric)  # Only label Y-axis on first subplot

        # ax.legend()
        # ax.legend(loc='center down', bbox_to_anchor=(0.5, 1))

        ax.grid(True)

# Put a legend below current axis
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.01),ncol=2)
    plt.tight_layout()
    plt.subplots_adjust(bottom=0.10)
    # plt.tight_layout()
    #plt.subplots_adjust(bottom=0.2)  # Add vertical space for the legend

    #fig.suptitle(f"{metric} Across Maps", fontsize=16)
    #plt.tight_layout()
    plt.show()
