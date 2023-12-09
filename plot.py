import matplotlib.pyplot as plt
import numpy as np
import matplotlib as mpl
import sys


if(len(sys.argv) < 2):
    print("usage: python3 plot.py <filename>")
    sys.exit(1)
    
filename = sys.argv[1]

# only column1 data is imported into numpy array from text file 
sizes = np.loadtxt(filename, usecols=1, skiprows=0, dtype=int) 
times = np.loadtxt(filename, usecols=4, skiprows=0, dtype=int) 
throughput = np.loadtxt(filename, usecols=7, skiprows=0, dtype=float) 

fig, axs = plt.subplots(1, 2)  # Create a figure containing a single axes.
axs[0].set_title('latency')
axs[0].set_xscale('log')
axs[0].set_xlabel('message size (bytes)')
axs[0].set_yscale('log')
axs[0].set_ylabel('latency (ns)')

axs[1].set_title('throughput')
axs[1].set_xscale('log')
axs[1].set_xlabel('message size (bytes)')
axs[1].set_yscale('log')
axs[1].set_ylabel('throughput (MB/s)')

axs[0].plot(sizes, times)
axs[1].plot(sizes, throughput)
plt.savefig("plot.pdf", format="pdf", bbox_inches="tight")
