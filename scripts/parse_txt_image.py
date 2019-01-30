#!/usr/bin/env python3
import numpy
import matplotlib.pyplot as plt
import sys

WIDTH = 160
HEIGHT = 120
data = numpy.zeros((HEIGHT, WIDTH))

def display_image(filename):
    # Parse file of floating point numbers into 2D array
    with open(filename, "r") as f:
        row = 0
        for line in f.read().strip().split("\n"):
            try:
                numbers = [float(i) for i in line.strip().split(" ")]
                data[row, :] = numbers
            except Exception as e:
                print("Excption: {}".format(e))
            row += 1
    # Show the result
    fig = plt.figure()
    plt.imshow(data)
    plt.show()


if len(sys.argv) < 2:
    print("Usage: {} filename1 [filename2...filenameN]".format(sys.argv[0]))
    sys.exit(1)

for filename in sys.argv[1:]:
    display_image(filename)

