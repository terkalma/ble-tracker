import paho.mqtt.client as mqtt
from models import Tag, Anchor
from datetime import datetime 
import matplotlib.pyplot as plt
import numpy as np
import time

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
plt.xlim([0, 1000])
plt.ylim([0, 1000])

HOST = "127.0.0.1"
UNAME = "tag"
PW = "potato666"

client = mqtt.Client("tagserver")


a1 = Anchor(0, 0)
a2 = Anchor(150, 0)
t = Tag(a1, a2)
data = ax.scatter(t.dss[0], t.dss[0], alpha=0.5)

def on_message(client, userdata, msg, properties=None):
    # print(msg.payload.decode('utf-8'))
    components = msg.payload.decode('utf-8').split(":")        
    try:        
        tag_id = components[0]

        for i in range((len(components) - 1) // 2):
            anchor_id = int(components[1 + 2 * i])
            distance = abs(int(float(components[1 + 2 * i + 1]) * 100))
            t.set_distance(anchor_id, distance)
    
        print(datetime.utcnow().strftime('%s'), anchor_id, 
            min(t.dss[0]),max(t.dss[0]),
            min(t.dss[1]),max(t.dss[1]))
        data.set_offsets(np.c_[t.dss[0],t.dss[1]])
        fig.canvas.draw_idle()
        plt.pause(0.01)
    except Exception as ex:
        print(components)
  

client.username_pw_set(username=UNAME, password=PW)
client.connect(HOST)
client.subscribe("tag")
client.on_message = on_message


client.loop_forever()
