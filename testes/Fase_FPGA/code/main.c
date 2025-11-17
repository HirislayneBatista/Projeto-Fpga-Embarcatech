// sd_to_pc.c
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "ff.h"        // FatFS
#include "pico/time.h"

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define CHUNK_BYTES (8*1024) // 8 KB chunks

FATFS fs;
FIL fil;

static uint8_t buffer[CHUNK_BYTES];

void spi_init_sd(void) {
    spi_init(SPI_PORT, 12 * 1000 * 1000); // comece em 12 MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
}

int main() {
    stdio_usb_init();
    sleep_ms(1500); // tempo para o host configurar a CDC
    printf("\n=== Pico SD -> PC Transfer Test ===\n");

    spi_init_sd();

    // monta FATFS
    FRESULT fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("f_mount failed: %d\n", fr);
        return 0;
    }
    printf("FAT mount ok\n");

    // Nome do arquivo no SD (ajuste se necessário)
    const char *fname = "foto_teste_1.jpg";

    fr = f_open(&fil, fname, FA_READ);
    if (fr != FR_OK) {
        printf("f_open failed: %d (arquivo '%s')\n", fr, fname);
        return 0;
    }

    // obtém tamanho do arquivo
    UINT bw;
    FSIZE_t fsize = f_size(&fil);
    if (fsize == 0) {
        printf("Arquivo vazio ou não encontrado\n");
        f_close(&fil);
        return 0;
    }

    // envia cabeçalho com o tamanho (ASCII)
    printf("SIZE:%llu\n", (unsigned long long)fsize);
    fflush(stdout);

    // aguarda o host preparar-se (opcional)
    sleep_ms(100);

    // leitura e envio
    FSIZE_t total = 0;
    absolute_time_t t0 = get_absolute_time();

    while (total < fsize) {
        UINT to_read = (fsize - total) > CHUNK_BYTES ? CHUNK_BYTES : (UINT)(fsize - total);
        fr = f_read(&fil, buffer, to_read, &bw);
        if (fr != FR_OK || bw == 0) {
            printf("f_read erro: %d read=%u\n", fr, bw);
            break;
        }

        // envia pelo USB (stdout)
        size_t wrote = fwrite(buffer, 1, bw, stdout);
        fflush(stdout); // garante envio imediato via CDC
        if (wrote != bw) {
            printf("warning: wrote %u expected %u\n", (unsigned)wrote, bw);
        }

        total += bw;
    }

    absolute_time_t t1 = get_absolute_time();
    double secs = absolute_time_diff_us(t0, t1) / 1e6;
    double mb = (double)total / (1024.0 * 1024.0);
    double mbps = secs > 0 ? mb / secs : 0.0;

    printf("\nDONE: sent %llu bytes in %.3f s -> %.3f MB/s\n", (unsigned long long)total, secs, mbps);

    f_close(&fil);
    // desmonta
    f_mount(NULL, "0:", 1);

    while(1) tight_loop_contents();
    return 0;
}
