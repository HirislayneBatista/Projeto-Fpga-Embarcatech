# receive_from_pico.py
import serial
import time
from pathlib import Path

# Ajuste a porta e o arquivo de destino
SERIAL_PORT = "COM5"            # Windows exemplo: "COM5" ; Linux ex: "/dev/ttyACM0"
OUTPUT_FOLDER = Path("C:\Users\55999\Pictures")
OUTPUT_FOLDER.mkdir(parents=True, exist_ok=True)
OUTPUT_FILENAME = OUTPUT_FOLDER / "foto_teste_1.jpg"

def read_line(ser):
    line = b""
    while True:
        c = ser.read(1)
        if not c:
            return None
        if c == b'\n':
            return line.decode().strip()
        line += c

def main():
    with serial.Serial(SERIAL_PORT, 115200, timeout=5) as ser:
        print("Conectado:", ser.portstr)
        # ler cabeçalho
        print("Aguardando cabeçalho SIZE...")
        header = read_line(ser)
        if header is None:
            print("Nenhum cabeçalho recebido.")
            return
        print("Header bruto:", header)
        if not header.startswith("SIZE:"):
            print("Header inesperado:", header)
            # pode ter mensagens de log antes; tentar achar a linha correta
            while header and not header.startswith("SIZE:"):
                header = read_line(ser)
                if header is None:
                    print("Sem header SIZE")
                    return
        size = int(header.split(":",1)[1])
        print(f"Tamanho a receber: {size} bytes")

        # agora receber exactly 'size' bytes
        with open(OUTPUT_FILENAME, "wb") as f:
            remaining = size
            start = time.perf_counter()
            while remaining > 0:
                chunk = ser.read(min(65536, remaining))
                if not chunk:
                    print("Timeout / sem dados")
                    break
                f.write(chunk)
                remaining -= len(chunk)
            end = time.perf_counter()

        received = size - remaining
        elapsed = end - start
        rate = (received / (1024.0*1024.0)) / elapsed if elapsed > 0 else 0.0
        print(f"Recebido: {received} bytes em {elapsed:.3f} s -> {rate:.3f} MB/s")
        if remaining == 0:
            print("Arquivo salvo em:", OUTPUT_FILENAME)
        else:
            print("Recepção incompleta.")

if __name__ == "__main__":
    main()
