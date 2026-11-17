#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      // open() e flags O_RDWR
#include <unistd.h>     // read(), write(), close(), lseek()
#include <errno.h>

#define DEVICE_PATH "/dev/caesar_driver"
#define BUFFER_SIZE 1024

int main(void)
{
    int fd;
    char read_buf[BUFFER_SIZE];
    ssize_t bytes_written, bytes_read;

    const char *mensagens[] = {
        "Sistemas Operacionais - UFPA",
        "Teste da Cifra de Cesar 2026!",
        "XYZ xyz -> deslocamento circular"
    };
    int total_msgs = 3;

    printf("=== Iniciando Teste do Driver de Caractere ===\n\n");

    /* 1. Abre o arquivo de dispositivo em /dev */
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Erro ao abrir /dev/caesar_driver (o modulo foi carregado com insmod?)");
        return EXIT_FAILURE;
    }
    printf("[OK] Dispositivo %s aberto com sucesso (fd = %d).\n\n", DEVICE_PATH, fd);

    /* 2. Envia e lê cada mensagem para validar a cifra e o contador de escritas */
    for (int i = 0; i < total_msgs; i++) {
        memset(read_buf, 0, sizeof(read_buf));

        printf("--- Teste %d ---\n", i + 1);
        printf("Texto original enviado (User Space):   \"%s\"\n", mensagens[i]);

        /* Escreve no driver */
        bytes_written = write(fd, mensagens[i], strlen(mensagens[i]));
        if (bytes_written < 0) {
            perror("Erro na chamada write()");
            close(fd);
            return EXIT_FAILURE;
        }

        /* Reposiciona o offset do descritor para o início antes de ler */
        lseek(fd, 0, SEEK_SET);

        /* Lê o texto cifrado retornado pelo kernel */
        bytes_read = read(fd, read_buf, sizeof(read_buf) - 1);
        if (bytes_read < 0) {
            perror("Erro na chamada read()");
            close(fd);
            return EXIT_FAILURE;
        }
        read_buf[bytes_read] = '\0';

        printf("Texto cifrado retornado (Kernel Space): \"%s\" (%zd bytes)\n\n",
               read_buf, bytes_read);
    }

    /* 3. Fecha o dispositivo */
    close(fd);
    printf("[OK] Dispositivo fechado. Verifique 'sudo dmesg | tail -n 15' para ver o contador de escritas.\n");

    return EXIT_SUCCESS;
}