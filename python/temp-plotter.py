import matplotlib.pyplot as plt, matplotlib.cm as cm
import bezier
import numpy

sol = [[0, 0], [3.39799e-38, 2064.75], [2.37892e-38, 33163.2], [72306.4, 43772.9], [89162.2, 47655.1], [29960.4, 42457.8], [71743.2, 51101.4], [70000, 80000]]
curve = bezier.Curve(numpy.array(sol), len(sol) - 1)
curve.plot(3700)
plt.savefig("plot.png") 