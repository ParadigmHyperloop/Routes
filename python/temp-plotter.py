import matplotlib.pyplot as plt, matplotlib.cm as cm
import bezier
import numpy

sol = [[0, 0], [66394.6, 81776.6], [18273.2, 59360.3], [107062, 76032], [63581.9, 54737.8], [40466.7, 16056.2], [79927.1, 79426.2], [63442.1, 61509.3], [25116.9, 25442.3], [70000, 80000]]
curve = bezier.Curve(numpy.array(sol), len(sol) - 1)
curve.plot(3700)
plt.savefig("plot.png") 