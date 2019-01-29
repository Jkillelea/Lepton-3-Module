#!/usr/bin/env python3
import numpy
import sys

WIDTH = 160
HEIGHT = 120

data = numpy.zeros((HEIGHT, WIDTH))

filename = sys.argv[1]

with open(filename, "r") as f:
    row = 0
    for line in f.read().split("\n"):
        try:
            numbers = [float(i) for i in line.strip().split(" ")]
            data[row, :] = numbers
        except Exception as e:
            print(e)
        row += 1

print(data)
