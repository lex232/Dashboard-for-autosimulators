"""Main telemtry reader -> serial"""
import os
import time

from dotenv import load_dotenv, set_key
import serial

from games.asetto_corsa import AssettoCorsaData
from games.fh5 import ForzaTelemetry

dotenv_path = ".env"
load_dotenv(dotenv_path)

COM_PORT = os.environ.get("COM")
SELECTED_GAME = os.environ.get("GAME")

START = b'\xAA'


def pack_to_bytes(speed, rpm, gear):
    if rpm < 0:
        rpm = 0
    if speed < 0:
        speed = 0
    if gear < 0:
        gear = 0        
    packed = (speed << 24) | (rpm << 7) | gear
    return START + packed.to_bytes(5, byteorder='big')


if __name__ == '__main__':
    actions = {
        "ASETTO_CORSA": AssettoCorsaData,
        "FORZA_HORIZON_5": ForzaTelemetry,
    }
    TelemetryReaderClass = actions.get(SELECTED_GAME)
    TelemetryReaderInstance = TelemetryReaderClass()
    TelemetryReaderInstance.start()
    connected = False
    while not connected:
        try:
            print(f"connecting to COM-{COM_PORT}")
            ser = serial.Serial(f'COM{COM_PORT}', 115200)
            connected = True
        except serial.serialutil.SerialException:
            com_input = input(f"Failed to connect to COM-{COM_PORT}\nEnter COM port (number only): ")
            COM_PORT = com_input

    if connected:
        set_key(dotenv_path, "COM", COM_PORT)

    while True:
        telemetry = TelemetryReaderInstance.get_data()
        speed = int(telemetry.get("speed"))
        rpm = int(telemetry.get("rpm"))
        gear = int(telemetry.get("gear"))
        print(speed, rpm, gear)
        data = pack_to_bytes(speed, rpm, gear)
        ser.write(data)
        time.sleep(0.02)
