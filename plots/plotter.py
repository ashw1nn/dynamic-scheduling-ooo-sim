import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Load the data
data_file = "ipc_results_gcc_trace.csv"
df = pd.read_csv(data_file)

# Add a new column for log2(S)
df['log2_S'] = np.log2(df['S'])

# Plot 1: IPC vs S for different shades of N
plt.figure(figsize=(7, 5))
for n in sorted(df['N'].unique()):
    subset = df[df['N'] == n]
    plt.plot(subset['log2_S'], subset['IPC'], label=f'N={n}', marker='o', linestyle='-')
    
plt.title("IPC vs Scheduling Queue Size (S) for Different N")
plt.xlabel("Scheduling Queue Size (S)")
plt.ylabel("IPC")
plt.legend(title="N (Peak Issue Bandwidth)")
plt.grid(True)
plt.savefig("ipc_vs_s_gcc_logS.png")  # Save the plot as a PNG file
plt.show()

# Plot 2: IPC vs N for different shades of S
# plt.figure(figsize=(7, 5))
# for s in sorted(df['S'].unique()):
#     subset = df[df['S'] == s]
#     plt.plot(subset['N'], subset['IPC'], label=f'S={s}', marker='o', linestyle='-')
    
# plt.title("IPC vs Peak Issue Bandwidth (N) for Different S")
# plt.xlabel("Peak Issue Bandwidth (N)")
# plt.ylabel("IPC")
# plt.legend(title="S (Scheduling Queue Size)")
# plt.grid(True)
# plt.savefig("ipc_vs_n_gcc.png")  # Save the plot as a PNG file
# plt.show()

print("Plots saved as 'ipc_vs_s.png' and 'ipc_vs_n.png'.")
