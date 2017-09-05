/*
 * ARBITRO
 */


#include <stdio.h>
#include <stdlib.h>
//#include <ncurses.h>
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

void ligacao() {
    clie_serv novo;

    novo.id = getpid();
    novo.flag_log = 0;
    novo.flag_con = 1;
    novo.flag_desliga = 0;
    novo.flag_operacao = 0;
    novo.flag_arbitro = 1;
    int fd = open(FIFO, O_WRONLY);

    int a = write(fd, &novo, sizeof (clie_serv));
    close(fd);

    if (a != sizeof (clie_serv)) {
        printf("\nERRO A ENVIAR DADOS(LOGIN)!!\n");
        return;
    }
}

void desconetar(int s) {
    if (s == SIGINT) {

        int fd;
        clie_serv des;

        fd = open(FIFO, O_WRONLY);
        des.id = getpid();
        des.flag_desliga = 1;
        des.flag_arbitro = 1;
        des.flag_con = 0;
        des.flag_log = 0;
        des.flag_operacao = 0;

        write(fd, &des, sizeof (clie_serv));

        close(fd);
        sair = 1;
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
    clie_serv p;

    do {
        printf("\nComando: ");

        scanf(" %[^\n]", cmd);

        //printf("\ncomando: %s\n", cmd);

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


        fd = open(FIFO_Arbitro, O_WRONLY);

        i = write(fd, &i, sizeof (int));
        close(fd);

    } while (!sair);


}

int main(int argc, char** argv) {//TODO: AVISAR SERVER QUE SE CONETOU

    signal(SIGINT, SIG_IGN);

    signal(SIGINT, desconetar);

    if (access(FIFO_Arbitro, F_OK) != 0) {
        printf("servidor off\n");
        return 3;
    }
    ligacao();
    envia_comando();
    return 0;

}

