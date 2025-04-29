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

def average_runs(x, times):
    t = np.linspace(0, 20, 1000)
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
df = pd.read_sql_query("SELECT * FROM repair", conn)

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
    fig, axes = plt.subplots(nrows=3, ncols=2, figsize=(6, 10), sharey=False)
    axes = axes.flatten()

    if num_maps == 1:  # If only one map, axes is not an array
        axes = [axes]

    # Iterate over each map
    for i, (map_name, map_group) in enumerate(df_filtered.groupby('map_name')):
        # print("map_name:", map_name)
        ax = axes[i]

        # Plot each algorithm
        for algo_name, algo_group in map_group.groupby('algo_name'):
            metric_list = algo_group[metric].to_list()
            time_list = algo_group["time_cumsum"].to_list()
            mean_time, mean_run = average_runs(metric_list, time_list)
            ax.plot(mean_time, mean_run, label=f"{algo_name}")
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
    plt.subplots_adjust(bottom=0.20)
    # plt.tight_layout()
    #plt.subplots_adjust(bottom=0.2)  # Add vertical space for the legend

    #fig.suptitle(f"{metric} Across Maps", fontsize=16)
    #plt.tight_layout()
    plt.show()

exit()

metric_names_averaged = ["iteration_time_cpu"]
#show metrics that are averaged over the time interval
for metric in metric_names_averaged:
    fig = plt.figure()
    ax = fig.add_subplot(111)

    vals = []
    map_names = []
    # Iterate over each map
    for i, (map_name, map_group) in enumerate(df_filtered.groupby('map_name')):
        map_names.append(map_name)
        # Plot each algorithm
        for j, (algo_name, algo_group) in enumerate(map_group.groupby('algo_name')):
            if (i == 0):
                vals.append([])
            metric_list = algo_group[metric].to_list()
            # discard the first iteration
            metric_list = metric_list[1:]
            min_len = np.min(np.asarray([len(x) for x in metric_list]))
            metric_list = [x[:min_len] for x in metric_list]
            metric_np = np.asarray(metric_list)
            mean_vec = np.mean(metric_np)
            vals[j].append(mean_vec)
            # print("Algo name:", algo_name)
            # print(algo_group[(metric, "mean")])


        # ax.legend()
        # ax.legend(loc='center down', bbox_to_anchor=(0.5, 1))

        ax.grid(True)
    i = 0
    for algo_name, algo_group in map_group.groupby('algo_name'):
        ax.plot(list(range(len(vals[i]))), vals[i], marker="x", label=f"{algo_name}")
        i += 1
    ax.set_title("Average replan time comparison")
    ax.set_xlabel("Maps")
    ax.set_ylabel("Average replan time [s]")  # Only label Y-axis on first subplot
    # ax.set_xticks(list(range(len(map_names))), labels=map_names)
    ax.set_xticklabels(map_names)

# Put a legend below current axis
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, 0.05),ncol=2)
    plt.tight_layout()
    plt.subplots_adjust(bottom=0.20)
    # plt.tight_layout()
    #plt.subplots_adjust(bottom=0.2)  # Add vertical space for the legend

    fig.suptitle(f"{metric} Across Maps", fontsize=16)
    #plt.tight_layout()
    plt.show()
