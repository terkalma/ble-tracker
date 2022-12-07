import numpy as np
from scipy.stats import multivariate_normal, norm

POINTS = 5000
KEEP = 100
WIDTH = 6
HEIGHT = 6



anchor_points = np.array([
    [3.06, 0.0, 1.49],
    [5.61, 2.43, 2.02],
    [0.0, 1.69, 2.64],
    [1.01, 5.07, 0.77],
])

anchor_count = anchor_points.shape[0]
SENSITIVITY = .2
MAX_HEIGHT = 2.2



class Particle():

    def __init__(self):
        points, anchor_distances, weights = self.initialize()

        assert anchor_distances.shape == (anchor_count, POINTS)
        assert points.shape == (3,POINTS)


        self.points = points
        self.anchor_distances = anchor_distances
        self.weights = weights


    def initialize(self, x_min=0, x_max=WIDTH, y_min=0, y_max=HEIGHT, z_min=0, z_max=MAX_HEIGHT):
        x = np.random.uniform(x_min, x_max, (1, POINTS))
        y = np.random.uniform(y_min, y_max, (1, POINTS))
        z = np.random.uniform(z_min, z_max, (1, POINTS))
        points = np.concatenate((x,y,z))
        weights = np.ones((1, POINTS)) / POINTS
        anchor_distances = np.zeros((anchor_count, POINTS))
        for i in range(0, anchor_points.shape[0]):        
            anchor_distances[i,:] = np.linalg.norm(anchor_points[i,:].reshape(3,1) - points, axis=0)
        return points, anchor_distances, weights

    def estimate(self):
        oned_weights = self.weights.reshape(POINTS,)
        mean = np.average(self.points.T, weights=oned_weights, axis=0)
        var  = np.average((self.points.T - mean) ** 2, weights=oned_weights, axis=0)
        return mean, var


    def update(self, distances):
        assert len(distances) == anchor_count

        for ind in range(self.anchor_distances.shape[0]):        
            self.weights *= norm(self.anchor_distances[ind, :], SENSITIVITY).pdf(distances[ind])    
        self.weights += 1.e-300              # avoid round-off to zero    
        self.weights /= np.sum(self.weights) # normalize


    def resample_around_good_points(self):
        indexes = np.argsort(self.weights)
        new_points = np.zeros(self.points.shape)

        best_points = np.take_along_axis(self.points, indexes, axis=1)[:, -KEEP:]
        assert best_points.shape == (3, KEEP)

        new_points[:, :KEEP] = best_points
        for i in range(1, POINTS // KEEP):
            new_points[:, i * KEEP:(i+1) * KEEP] = new_points[:, :KEEP] + (np.random.normal(0, 0.05, (3,KEEP)))

        new_weights = np.ones((1, POINTS)) / POINTS
        self.anchor_distances = np.zeros((anchor_count, POINTS))
        for i in range(0, anchor_points.shape[0]):        
            self.anchor_distances[i,:] = np.linalg.norm(anchor_points[i,:].reshape(3,1) - new_points, axis=0)

        self.points = new_points
        self.weights = new_weights

    def resample(self, mean):
        x = np.random.normal(mean[0], 0.1, (1, POINTS))
        y = np.random.normal(mean[1], 0.1, (1, POINTS))
        z = np.random.normal(mean[2], 0.1, (1, POINTS))
        points = np.concatenate((x,y,z))
        weights = np.ones((1, POINTS)) / POINTS
        anchor_distances = np.zeros((anchor_count, POINTS))
        for i in range(0, anchor_points.shape[0]):        
            anchor_distances[i,:] = np.linalg.norm(anchor_points[i,:].reshape(3,1) - points, axis=0)
        
        self.points = points
        self.weights = weights
        self.anchor_distances = anchor_distances

        
