import cv2
import numpy as np

DELAY_MSEC = 50
WINDOW_NAME = 'particles'


class Animation:
    def __init__(self, width, height):
        self.width = int(width * 100)
        self.height = int(height * 100)
        self.img = np.zeros((self.height, self.width,3))        


    def create_image(self):
        cv2.namedWindow(WINDOW_NAME)        
        cv2.imshow(WINDOW_NAME, self.img)

    def draw_cloud(self, points):
        self.img *= 0.95
        g = 0.8
        for p in points:
            self.img[int(p[1] * 100), int(p[0] * 100)] = (g, g, g)

    def draw_point(self, x, y):
        d = 2
        t = 1        
        ctrx = int(x * 100)
        ctry = int(y * 100)

        cv2.line(self.img, (ctrx - d, ctry - d), (ctrx + d, ctry + d), (1, 1, 1), t, cv2.LINE_AA)
        cv2.line(self.img, (ctrx + d, ctry - d), (ctrx - d, ctry + d), (1, 1, 1), t, cv2.LINE_AA)

    def animate(self):
        cv2.imshow(WINDOW_NAME, self.img)
        if cv2.waitKey(DELAY_MSEC) & 0xFF == 27:
            1 / 0