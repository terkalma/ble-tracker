import redis
import time
import numpy as np
from utils import logging

PW = os.environ.get('PW')

redis_client = redis.StrictRedis(host='localhost', password=PW, port=6379, db=0)
logger = logging.getLogger(__name__)


def process(anchor_index, tag_index):	
	redis_client.set(f"anchor{anchor_index}", tag_index)

	result = get_result(anchor_index)		
	if result:
		try:
			m_comps = result.decode('utf-8').split(":")
			dist = float(m_comps[1])
			corr_dist = float(m_comps[2])
			power = float(m_comps[3])
			logger.info(f'{anchor_index} -> {tag_index}: {dist}, {corr_dist}, {power}')
		except Exception as e:
			logger.exception(f"Unable to parse data from Redis: {result}")			


def get_result(index):
	counter = 0
	while True:
		value = redis_client.get(f"anchor{index}r")		
		if value:
			redis_client.delete(f"anchor{index}r")
			return value
		time.sleep(0.1)
		counter += 1
		if counter > 5:
			return None


if __name__ == '__main__':
	while True:
		process(1, 2)
		



