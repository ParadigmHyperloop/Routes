import matplotlib.pyplot as plt, matplotlib.cm as cm
import bezier
import numpy

sol = [[0, 0], [81767.2, 56754.4], [14715.6, 56011.5], [39098.7, 61914], [90311.1, 76620], [11291.1, 79726.1], [96767.8, 24481.6], [70000, 80000]]
curve = bezier.Curve(numpy.array(sol), len(sol) - 1)
curve.plot(3700)
plt.savefig("plot.png") 