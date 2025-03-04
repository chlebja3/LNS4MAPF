import sqlite3

# Connect to SQLite database (or create it)
conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

# Create a table with a JSON column
cursor.execute("""
CREATE TABLE IF NOT EXISTS experiments (
    experiment_id INTEGER PRIMARY KEY,
    experiment_name TEXT,
    map_name TEXT,
    scen_name TEXT,
    num_agents INTEGER
)
""")

experiment = {
    "experiment_id": 1,
    "experiment_name": "dummy",
    "map_name": "A",
    "scen_name": "B",
    "num_agents": 10,
}

# Insert into table
cursor.execute("""
INSERT INTO experiments (experiment_id, experiment_name, map_name, scen_name, num_agents)
VALUES (:experiment_id, :experiment_name, :map_name, :scen_name, :num_agents)
""", experiment)

# Save changes
conn.commit()

# retrieve the data
cursor.execute("SELECT * FROM experiments WHERE experiment_name = 'dummy'")
results = cursor.fetchall()

# Print results
for row in results:
    print(row)
conn.close()
