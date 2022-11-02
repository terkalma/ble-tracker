class Anchor:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class Tag: 

    def __init__(self, anchor_1, anchor_2):
        self.a1 = anchor_1
        self.a2 = anchor_2

        self.ad = ((self.a1.x - self.a2.x) ** 2 + (self.a1.y - self.a2.y) ** 2) ** 0.5        
        self.ds = [0] * 2
        self.ms = [False, False]

    def set_distance(self, anchor_id, distance):
        self.ds[anchor_id - 1] = distance
        self.ms[anchor_id - 1] = True

    def calculate(self):        
        cos_alpha = max((self.ds[0] ** 2 + self.ad ** 2 - self.ds[1] ** 2) / (2 * self.ds[0] * self.ad), 0)
        cos_alpha = min(cos_alpha, 1)
        sin_alpha = max((1 - cos_alpha ** 2), 0) ** 0.5

        return sin_alpha * self.ds[0], cos_alpha * self.ds[0]