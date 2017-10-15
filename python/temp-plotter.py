import matplotlib.pyplot as plt, matplotlib.cm as cm
import bezier
import numpy

sol = [[0, 0], [2.33831e-38, 2048.09], [1.46098e-38, 33024], [69159.9, 79028.9], [71270.1, 13151.3], [60990.4, 32566.8], [67129.7, 83561], [67050.9, 54918], [54911.1, 31221], [70000, 80000]]
curve = bezier.Curve(numpy.array(sol), len(sol) - 1)
curve.plot(3700)
plt.savefig("plot.png") 