import matplotlib.pyplot as plt, matplotlib.cm as cm
import bezier
import numpy

sol = [[0, 0], [21873.1, 31872.5], [47215.7, 14431.9], [27216.6, 52574.7], [66803.6, 47882.7], [72061.1, 58684.2], [93529.5, 59533.7], [70000, 80000]]
curve = bezier.Curve(numpy.array(sol), len(sol) - 1)
curve.plot(3700)
plt.savefig("plot.png") 