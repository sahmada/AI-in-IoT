import paho.mqtt.client as mqtt
import threading as t
import json

value = 10
is_running = True
is_timer_running = False


def send_periodic_data():
    if is_running:
        client.publish("v1/devices/me/telemetry",
                       payload=json.dumps({"value": value}))
        t.Timer(1, send_periodic_data).start()


def on_connect(client, userdata, flags, rc):
    print("connected with result code : " + str(rc))
    client.subscribe("v1/devices/me/rpc/request/+")
    global is_timer_running
    if is_timer_running == False:
        is_timer_running = True
        send_periodic_data()


# The callback for a PUBLISH message received from server
def on_message(client, userdata, msg):
    rpc_topic = "v1/devices/me/rpc/request/"
    if msg.topic.startswith(rpc_topic):
        print(msg.topic + " " + str(msg.payload))
        id = msg.topic[len(rpc_topic):]
        p = json.loads(msg.payload)
        global value
        if p["method"] == "getValue":
            client.publish("v1/devices/me/rpc/response/" + id, payload=value)
        elif p["method"] == "setValue":
            value = p["params"]


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set(username="value-test")

client.connect("tb-srv.abrema.ir", 1883, 60)

try:
    client.loop_forever()
except KeyboardInterrupt:
    is_running = False
