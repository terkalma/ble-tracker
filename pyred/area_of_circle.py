import numpy as np



for no_points in [100, 1000, 10000, 1000000]:
	def is_in(x, y, r_sq):
		return x ** 2 + y ** 2 < r_sq

	R = 1


	x_points = np.random.uniform(-R, R, (no_points, 1))
	y_points = np.random.uniform(-R, R, (no_points, 1))

	print(f"{no_points}: {((np.sum(x_points ** 2 + y_points ** 2 < R ** 2) / no_points * 4) - np.pi) ** 2:0.5f}")