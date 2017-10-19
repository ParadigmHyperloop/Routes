import matplotlib.pyplot as plt, matplotlib.cm as cm
import bezier
import numpy

#speed of pod
MACH1 = 343 #m/s

# The solution points for the bezier curve
sol = [[-119.002,35.0017],[-118.08,34.1428],[-118.89,34.8503],[-118.864,34.8639],[-118.769,34.8672],[-118.902,34.9447],[-118.941,34.8525],[-118.373,34.2831],]

#define the bezier curve
curve = bezier.Curve(numpy.array(sol), len(sol) - 1)

#calculate the length of the bezier curve
arcLength = curve.length

#calculate the time to travel the length of the tube
time = (arcLength / (.95 * MACH1)) / 60 

#define the current plot
currentPlot = curve.plot(3700)

#create the string to be overlayed on the graph
timeString = "Time to travel the route at Mach1 in minutes: " + str(time)
distanceString = "Distance traveled in meters: " + str(arcLength)

#annotate the graph
currentPlot.annotate(timeString, xy=(7000,0), xytext=(7000,0))
currentPlot.annotate(distanceString, xy=(7000,4000), xytext=(7000,4000))

#output the graph to a file
plt.savefig("plot.png") 

for i in range(0, 100):
	arr = curve.evaluate(i / 99.0)
	print("[{}, {}],".format(arr[0][0], arr[0][1]))