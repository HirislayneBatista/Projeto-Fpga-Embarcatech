import serial

SERIAL_PORT = "COM11"
BAUD = 115200

with serial.Serial(SERIAL_PORT, BAUD, timeout=1) as ser:
    while True:
        line = ser.readline().decode('utf-8').strip()
        if line:
            print(line)
