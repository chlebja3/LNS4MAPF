import os
import json
import sqlite3

# Connect to SQLite database (or create it)
conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Create a table for the experiments
cursor.execute("""
CREATE TABLE IF NOT EXISTS experiments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_name TEXT UNIQUE, -- each experiment can be stored onyl once
    algo_name TEXT,
    algo_parameters TEXT,  -- Stored as JSON string
    computation_time_cpu REAL,
    computation_time_wall REAL,
    expanded INTEGER,
    experiment_name TEXT,
    feasible BOOLEAN,
    generated INTEGER,
    makespan INTEGER,
    map_name TEXT,
    num_agents INTEGER,
    preprocessing_time_cpu REAL,
    preprocessing_time_wall REAL,
    scene_name TEXT,
    seed INTEGER,
    sum_of_cost INTEGER,
    sum_of_dist INTEGER
)
""")

conn.commit()
experiment_name = "sipp_pp"

# path to the directory with the experiments
experiment_dir = "../my_code/my_solver/experiments/"+ experiment_name

# go through the experiments and add them to the database
map_dirs = os.listdir(experiment_dir)

added = 0
skipped = 0
for map_dir in map_dirs:
    map_path = os.path.join(experiment_dir, map_dir)
    #print("Map:", map_path)
    files = os.listdir(map_path)
    for file in files:
        #print(file)
        if (file[0:len(experiment_name)] != experiment_name): continue # skip files that are not experiments
        with open(os.path.join(map_path, file), "r") as f:
            data = json.load(f) # loads the json as dictionary
            data["file_name"] = file

            # convert the algorithm parameters to json string
            data["algo_parameters"] = json.dumps(data["algo_parameters"])

            # check whether the experiment is already in the database
            cursor.execute("""
            SELECT 1 FROM experiments WHERE 
                file_name = ?
            """, (file,))
            if cursor.fetchone() is None:  # If no record found, insert new entry
                # Dynamically create the SQL query
                columns = ", ".join(data.keys())  # Get column names from JSON keys
                placeholders = ", ".join(["?"] * len(data))  # Create placeholders (?, ?, ?, ...)

                sql = f"INSERT INTO experiments ({columns}) VALUES ({placeholders})"
                cursor.execute(sql, tuple(data.values()))
                added += 1
            else:
                skipped += 1
conn.commit()
conn.close()
print("Experiment database updated successfully.")
print("Added: " + str(added) + "/" + str(added + skipped) + " experiments")
print("Skipped: " + str(skipped) + "/" + str(added + skipped) + " experiments")
