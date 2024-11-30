import subprocess
import pandas as pd

# Parameters
s_values = [8, 16, 32, 64, 128, 256]
n_values = [2, 4, 8]
cpp_file = "proj_tomasulo.cpp"
executable = "./a.out"

# # Compile the C++ file
compile_command = ["g++", "-o", executable, cpp_file, "-O3", "-Wall"]
subprocess.run(compile_command, check=True)

# Initialize an empty list to store results
results = []
trace = "perl_trace"
# Loop over all combinations of S and N
for n in n_values:
    for s in s_values:
        # Run the executable with the current parameters
        run_command = [executable, str(n), str(s), trace+".txt"]
        process = subprocess.run(run_command, capture_output=True, text=True)
        
        # Parse the output to extract IPC (assuming the output contains "IPC: <value>")
        output = process.stdout
        try:
            ipc = float(output.split(" IPC\t=")[1].split(sep="=")[0])
            print(f"Completed S={s}, N={n}")
        except (IndexError, ValueError):
            # print(output.split(" IPC\t=")[1].split(sep="=")[0])
            print(f"Error parsing IPC for S={s}, N={n}")
            continue
        
        # Append the result to the list
        results.append({"S": s, "N": n, "IPC": ipc})

# Convert the results to a DataFrame
df = pd.DataFrame(results)

# Save the DataFrame to a CSV file for further use
df.to_csv(f"ipc_results_{trace}.csv", index=False)

# Display the DataFrame to the user
# import ace_tools as tools; tools.display_dataframe_to_user(name="IPC Results DataFrame", dataframe=df)

print("Data collection complete. Use the DataFrame or CSV file for plotting.")
