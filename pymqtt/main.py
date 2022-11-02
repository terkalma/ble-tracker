import paho.mqtt.client as mqtt
from models import Tag, Anchor

HOST = "127.0.0.1"
UNAME = "tag"
PW = "potato666"

client = mqtt.Client("tagserver")




a1 = Anchor(0, 0)
a2 = Anchor(105, 0)
t = Tag(a1, a2)

def on_message(client, userdata, msg, properties=None):
    # print(msg.payload.decode('utf-8'))
    components = msg.payload.decode('utf-8').split("|")        
    try:        
        tag_id = components[0]
        anchor_id = int(components[1])
        distance = abs(int(float(components[2]) * 100))
        t.set_distance(anchor_id, distance)
        if all(t.ms):
            t.ms = [False, False]
            x, y = t.calculate()
            print(t.ds[0], t.ds[1], x, y)
    except:
        print(components)
  

client.username_pw_set(username=UNAME, password=PW)
client.connect(HOST)
client.subscribe("tag")
client.on_message = on_message
client.loop_forever()
connect(host, port=1883)