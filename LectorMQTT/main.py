# This is a sample Python script.

# Press Mayús+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.


import paho.mqtt.client as mqtt

def on_message(client, userdata, message):
    print(f"Recibido del ESP32: {message.payload.decode()}")

if __name__ == "__main__":
    try:
        client = mqtt.Client()
        client.connect("localhost", 1883)
        client.subscribe("casa/esp32/sensor")
        client.on_message = on_message
        client.loop_forever()
    except KeyboardInterrupt:
        client.loop_stop()
        client.disconnect()
