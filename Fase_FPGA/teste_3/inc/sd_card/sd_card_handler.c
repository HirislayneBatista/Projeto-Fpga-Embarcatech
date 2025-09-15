// Arquivo: inc/sd_card_handler.c
// Chame: Sdh_SendFileToHost("photo_test.jpg");
// envia: "SIZE:<n>\n" seguido de n bytes binários e então "\nDONE\n"

#include "sd_card_handler.h"
#include "hw_config.h"      // Para sd_get_by_num() e sd_init_driver()
#include "f_util.h"         // Para FRESULT_str()
#include "rtc.h"
#include "pico/stdlib.h"
#include <string.h>         // Para memcpy
#include <stdio.h>          // Para printf
#include "no-OS-FatFS-SD-SPI-RPi_Pico/Fats_SPI/ff15/source/ff.h"


#define SEND_CHUNK_BYTES (16 * 1024) // 16 KiB (múltiplo de 512)
static uint8_t send_buf[SEND_CHUNK_BYTES];

// Instância global do sistema de arquivos para esta biblioteca
FATFS fs_global;

/**
 * @brief Inicializa o hardware SPI para o cartão SD e monta o sistema de arquivos.
 */

bool Sdh_Init(void){
    // Esta função da biblioteca de exemplo lê a configuração em hw_config.c e prepara o SPI
    sd_init_driver(); 
    // time_init();

    printf("[INIT] Obtendo configuracao para o cartao SD '0:'...\n");
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("ERRO FATAL: Configuracao do SD '0:' nao encontrada em hw_config.c!\n");
        return false;
    }
    printf(">> SUCESSO: Configuracao do SD carregada.\n");

    printf("[INIT] Tentando montar o cartao SD (f_mount)...\n");
    // f_mount usa o nome do drive (ex: "0:") que está em pSD->pcName
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1); 
    if (fr != FR_OK) {
        printf("ERRO: Nao foi possivel montar o cartao SD! Codigo FatFs: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }
    printf(">> SUCESSO: Cartao SD montado com sucesso.\n");
    
    // Copia a instância montada para a variável global para uso em outras funções
    memcpy(&fs_global, &pSD->fatfs, sizeof(FATFS));

    return true;
}

static int wait_for_host_ready_ms(int timeout_ms){
    // espera "READY\n" vindo do host via stdin (USB CDC), opcional
    int waited = 0;
    const int step_ms = 100;
    char line[32];
    int pos = 0;
    while (waited < timeout_ms) {
        int c = getchar_timeout_us(1000 * step_ms);
        if (c == PICO_ERROR_TIMEOUT) {
            waited += step_ms;
            continue;
        }
        if (c == '\r') continue;
        if (c == '\n') {
            line[pos] = '\0';
            if (pos > 0 && strcmp(line, "READY") == 0) return 1;
            pos = 0;
            continue;
        }
        if (pos < (int)sizeof(line)-1) line[pos++] = (char)c;
    }
    return 0;
}

bool Sdh_SendFileToHost(const char *fname){  //função para enviar arquivo via USB CDC
    FIL f;
    FRESULT fr;
    UINT br;
    FSIZE_t total = 0;

    fr = f_open(&f, fname, FA_READ);
    if (fr != FR_OK) {
        printf("Sdh_SendFileToHost: f_open('%s') falhou: %d\n", fname, fr);
        return false;
    }

    FSIZE_t fsize = f_size(&f);
    if (fsize == 0) {
        printf("Sdh_SendFileToHost: arquivo vazio\n");
        f_close(&f);
        return false;
    }

    // envia cabeçalho com tamanho
    printf("SIZE:%llu\n", (unsigned long long)fsize);
    fflush(stdout);

    // opcional: esperar READY do host (até 5s)
    if (wait_for_host_ready_ms(5000)) {
        printf("Sdh: Host READY recebido. Iniciando...\n");
    } else {
        // sem ack, prossegue após pequena pausa
        sleep_ms(200);
    }

    absolute_time_t t0 = get_absolute_time();

    while (total < fsize) {
        UINT to_read = (fsize - total) > SEND_CHUNK_BYTES ? SEND_CHUNK_BYTES : (UINT)(fsize - total);
        fr = f_read(&f, send_buf, to_read, &br);
        if (fr != FR_OK) {
            printf("Sdh_SendFileToHost: erro f_read: %d\n", fr);
            f_close(&f);
            return false;
        }
        if (br == 0) break;

        size_t wrote = fwrite(send_buf, 1, br, stdout); // envia pelo USB CDC
        fflush(stdout); // força envio imediato
        if (wrote != br) {
            printf("Sdh_SendFileToHost: aviso wrote=%zu expected=%u\n", wrote, br);
        }
        total += br;
    }

    absolute_time_t t1 = get_absolute_time();
    double secs = absolute_time_diff_us(t0,t1) / 1e6;
    double mb = (double)total / (1024.0*1024.0);
    printf("\nDONE\n");
    printf("Sdh_SendFileToHost: enviado %llu bytes em %.3f s -> %.3f MB/s\n",
           (unsigned long long)total, secs, mb / secs);

    f_close(&f);
    return true;
}

//funções anteriores originais excluidas