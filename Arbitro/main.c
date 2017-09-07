/*
 * ARBITRO
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "../estrutura.h"

int sair = 0;

void desconetar(int s) {
    if (s == SIGINT) {

        int fd;
        arbitro_serv des;
        sair = 1;

        fd = open(FIFO_Arbitro, O_WRONLY);
        des.pid = getpid();
        des.op = -1;//desligar

        write(fd, &des, sizeof (clie_serv));

        close(fd);
        usleep(100);
        exit(3);
    } else if (s == SIGUSR1) {
        printf("\nDesconetado pelo Servidor!\n");
        exit(3);
    }
}

void envia_comando() {
    char cmd[80];
    char *primeiro;
    int i, fd;
    arbitro_serv p;

    do {
        printf("\nComando: ");

        scanf(" %[^\n]", cmd);

        char comandos[5][30] = {"inicio", "intervalo", "recomeca", "falta", "termina"};

        primeiro = strtok(cmd, " ");

        for (i = 0; i < 5; i++) {
            if (strcmp(primeiro, comandos[i]) == 0) {
                break;
            }
        }

        if (i > 5) {
            printf("\nComando Invalido!\n");
            continue;
        }
        p.op = i;
        p.pid = getpid();

        fd = open(FIFO_Arbitro, O_WRONLY);

        i = write(fd, &p, sizeof (arbitro_serv));
        close(fd);

    } while (!sair);


}

int main(int argc, char** argv) {

    signal(SIGINT, SIG_IGN);

    signal(SIGINT, desconetar);

    if (access(FIFO, F_OK) != 0) {
        printf("servidor off\n");
        return 3;
    }
    
    envia_comando();
    return 0;

}

