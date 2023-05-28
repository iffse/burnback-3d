import json
import numpy as np
import matplotlib.pyplot as plt

with open('results.json') as f:
	data = json.load(f)
	x = data['x']
	# x = data['y']
	z = data['z']
	uVertex = data['uVertex']

fig = plt.figure()
ax = fig.add_subplot(projection='3d')
ax.scatter(x, z, uVertex, c='r', marker='o')
ax.set_xlabel('x')
ax.set_ylabel('z')
ax.set_zlabel('uVertex')

plt.show()
