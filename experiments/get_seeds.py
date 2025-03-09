import sqlite3
import pandas as pd

conn = sqlite3.connect("experiment_database.db")
cursor = conn.cursor()

df = pd.read_sql_query("SELECT * FROM experiments WHERE file_name = 'sipp_pp_den520d_random-0_SIPP_mapf_lns_0.json'", conn)
print(df["seed"].values)
conn.close()
