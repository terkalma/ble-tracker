import redis
import os
import time
import numpy as np

from animation import Animation
import random
from utils import logging 

from typing import Optional, List
import particle as p

logger = logging.getLogger(__name__)

PW = os.environ.get('PW')
MAX_COST = 1

redis_client = redis.StrictRedis(host='localhost', password=PW, port=6379, db=0)

anchour_count = 4
tag_index = 2
PLOT = True

particle = p.Particle()
animation = Animation(p.WIDTH, p.HEIGHT)

ds = [-1] * anchour_count

def get_distances() -> Optional[List[float]]:
    valid = True
    for anchor_index in range(1, anchour_count + 1):
        redis_client.set(f"anchor{anchor_index}", tag_index)
        result = get_result(anchor_index)        
        if result:
            m_comps = result.decode('utf-8').split(":")
            dist = float(m_comps[1])

            if dist > 0:
                ds[anchor_index - 1] = dist
            else:
                valid = False
        else:
            valid = False


def process():
    get_distances()    
    if all(x>0 for x in ds):
        particle.update(ds)
        mean, var = particle.estimate()
        logger.info(mean)        

        if PLOT:            
            animation.draw_cloud(particle.points.T)
            animation.draw_point(mean[0], mean[1])                    
        particle.resample(mean)

def get_result(index):
    counter = 0
    while True:
        value = redis_client.get(f"anchor{index}r")        
        if value:
            redis_client.delete(f"anchor{index}r")
            return value
        time.sleep(0.05)
        counter += 1
        if counter > 5:
            return None

if __name__ == '__main__':           
    if PLOT:
        animation.create_image() 
    while True:
        process()
        

        if PLOT:
            animation.animate()
        else:
            time.sleep(0.1)
        



