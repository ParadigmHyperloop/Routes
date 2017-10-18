import matplotlib.pyplot as plt, matplotlib.cm as cm
import bezier
import numpy

#speed of pod
MACH1 = 343 #m/s

# The solution points for the bezier curve
sol = [[-119.002,35.0017],[-118.891,34.6044],[-118.658,34.77],[-118.743,34.7233],[-118.632,34.5244],[-118.432,34.4072],[-118.344,34.4622],[-118.373,34.2831],]

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