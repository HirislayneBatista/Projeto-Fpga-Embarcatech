#include <stdio.h>
#include "pico/stdlib.h"
#include "ff.h"              // FatFs
#include "inc/sd_card/sd_card_handler.h"         // Interface SPI+SD
#include "no-OS-FatFS-SD-SPI-RPi-Pico/simple_example/simple_example.cpp"

FATFS fs;   // Estrutura do sistema de arquivos
FIL fil;   // Estrutura de arquivo
FRESULT fr;

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Aguarda USB conectar

    //printf("Iniciando SD...\n");

    //Sdh_Init();

    //printf("Enviando arquivo...\n");
    //Sdh_SendFileToHost("foto_teste_1.jpg");   // Chama a função que envia via USB/Serial

    //printf("Transferencia concluida.\n");

    exemplo_text();

    return 0;
}