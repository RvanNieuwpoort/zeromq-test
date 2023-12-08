import matplotlib.pyplot as plt
import numpy as np
import matplotlib as mpl

# only column1 data is imported into numpy array from text file 
sizes = np.loadtxt("test1.txt", usecols=1, skiprows=0, dtype=int) 
times = np.loadtxt("test1.txt", usecols=4, skiprows=0, dtype=int) 
throughput = np.loadtxt("test1.txt", usecols=7, skiprows=0, dtype=float) 

print(sizes)
print(times)
print(throughput)


fig, ax = plt.subplots()  # Create a figure containing a single axes.

ax.set_xscale('log')
ax.set_xlabel('message size (bytes)')
ax.set_ylabel('throughput (MB/s)')

ax.plot(sizes, throughput)
plt.savefig("plot.pdf", format="pdf", bbox_inches="tight")
