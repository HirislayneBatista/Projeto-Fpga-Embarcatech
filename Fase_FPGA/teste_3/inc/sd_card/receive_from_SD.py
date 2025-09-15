import serial
from pathlib import Path

SERIAL_PORT = "COM10"   # porta correta
BAUD = 115200

with serial.Serial(SERIAL_PORT, BAUD, timeout=1) as ser:
    filename = None
    filesize = 0
    received = 0
    file = None

    while True:
        line = ser.readline().decode(errors="ignore").strip()
        if line.startswith("FILE_START"):
            _, fname, fsize = line.split()
            filename = Path(fname).name  # só o nome, sem "D:" nem caminho estranho
            filesize = int(fsize)
            file = open(filename, "wb")
            print(f"[PC] Recebendo {filename} ({filesize} bytes)...")
            received = 0
            continue

        if line == "FILE_END":
            if file:
                file.close()
            print(f"[PC] Arquivo {filename} salvo com sucesso.")
            break

        # Se não estamos lendo cabeçalhos, são dados brutos
        if file:
            data = ser.read(filesize - received)
            file.write(data)
            received += len(data)
            if received >= filesize:
                print(f"[PC] Recebimento concluído ({received} bytes).")
                file.close()
                break
