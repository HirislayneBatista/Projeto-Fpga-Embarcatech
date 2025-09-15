import serial, time
from pathlib import Path

SERIAL_PORT = "COM11"   # ajuste
BAUD = 115200
OUTPUT_PATH = Path("C:/Users/SeuUsuario/Desktop/pico_received")
OUTPUT_PATH.mkdir(parents=True, exist_ok=True)

def read_line(ser, timeout=30):
    end = time.time() + timeout
    b = b""
    while time.time() < end:
        c = ser.read(1)
        if not c:
            continue
        if c == b'\n':
            return b.decode().strip()
        if c != b'\r':
            b += c
    return None

with serial.Serial(SERIAL_PORT, BAUD, timeout=0.1) as ser:
    print("Aguardando header SIZE...")
    line = read_line(ser, timeout=30)
    if not line or not line.startswith("SIZE:"):
        print("Header não recebido:", line)
        raise SystemExit
    size = int(line.split(":",1)[1])
    print(f"Tamanho = {size} bytes. Enviando READY...")
    ser.write(b"READY\n")

    out_file = OUTPUT_PATH / f"received_{int(time.time())}.jpg"
    remaining = size
    t0 = time.perf_counter()
    with open(out_file, "wb") as f:
        while remaining > 0:
            chunk = ser.read(min(65536, remaining))
            if not chunk:
                continue
            f.write(chunk)
            remaining -= len(chunk)
    t1 = time.perf_counter()
    elapsed = t1 - t0
    mb = size / (1024*1024)
    print(f"Recebido {size} bytes em {elapsed:.3f}s -> {mb/elapsed:.3f} MB/s")
    print("Salvo:", out_file)
