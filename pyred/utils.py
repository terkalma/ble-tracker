import logging
import os
import sys

logging.basicConfig(
    format="%(asctime)s: %(message)s",
    level=int(os.environ.get("LEVEL", logging.INFO)),
    stream=sys.stdout,
)
