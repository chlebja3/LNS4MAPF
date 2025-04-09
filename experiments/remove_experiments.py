import os
import json
import sqlite3

# Connect to SQLite database (or create it)
conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

experiment_name = "sipp_pp"
algo_names = ["SIPP_lns_orig"]
# algo_names = [""]

# Count total rows before deletion
cursor.execute("SELECT COUNT(*) FROM " + experiment_name)
total_before = cursor.fetchone()[0]
deleted = 0

for algo_name in algo_names:
    cursor.execute("DELETE FROM " + experiment_name + """ WHERE 
        algo_name = ?
    """, (algo_name,))
    deleted += cursor.rowcount  # Number of deleted rows

conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Removed: " + str(deleted) + "/" + str(total_before) + " experiments")
