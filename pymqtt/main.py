import paho.mqtt.client as mqtt
from models import Tag, Anchor

HOST = "127.0.0.1"
UNAME = "tag"
PW = "potato666"

client = mqtt.Client("tagserver")




a1 = Anchor(0, 0)
a2 = Anchor(150, 0)
t = Tag(a1, a2)

def on_message(client, userdata, msg, properties=None):
    # print(msg.payload.decode('utf-8'))
    components = msg.payload.decode('utf-8').split(":")        
    try:        
        tag_id = components[0]

        for i in range((len(components) - 1) // 2):
            anchor_id = int(components[1 + 2 * i])
            distance = abs(int(float(components[1 + 2 * i + 1]) * 100))
            t.set_distance(anchor_id, distance)
            if all(t.ms):
                t.ms = [False, False]
                x, y = t.calculate()
                print(anchor_id, t.ds[0], t.ds[1], x, y)
    except:
        print(components)
  

client.username_pw_set(username=UNAME, password=PW)
client.connect(HOST)
client.subscribe("tag")
client.on_message = on_message
client.loop_forever()
connect(host, port=1883)