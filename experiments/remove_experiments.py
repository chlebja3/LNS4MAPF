import os
import json
import sqlite3

# Connect to SQLite database (or create it)
conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

experiment_name = "sipp_pp"
algo_name = "SIPP_lns_orig"

# Count total rows before deletion
cursor.execute("SELECT COUNT(*) FROM experiments")
total_before = cursor.fetchone()[0]

cursor.execute("""
DELETE FROM experiments WHERE 
    algo_name = ?
""", (algo_name,))
deleted = cursor.rowcount  # Number of deleted rows

conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Removed: " + str(deleted) + "/" + str(total_before) + " experiments")
