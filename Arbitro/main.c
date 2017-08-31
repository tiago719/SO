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

//void atualiza_campo(serv_clie * j) {
//    char tempo[20], clientes[20];
//    if (j->equipa == 'a')
//        attron(COLOR_PAIR(2));
//    else if (j->equipa == 'b')
//        attron(COLOR_PAIR(1));
//    else
//        attron(COLOR_PAIR(3));
//
//    mvaddch(j->xant, j->yant, ' ');
//
//    mvaddch(j->xnovo, j->ynovo, j->jogador);
//
//    sprintf(tempo, "Tempo: %d", j->resultados.tempo);
//
//    attron(COLOR_PAIR(2)); //TODO:meter a cor a 3
//    mvaddstr(22, 0, tempo);
//
//    attron(COLOR_PAIR(2));
//    mvaddch(22, 23, '0' + j->resultados.res_eq1);
//    attron(COLOR_PAIR(3));
//    mvaddch(22, 24, '-');
//    attron(COLOR_PAIR(1));
//    mvaddch(22, 25, '0' + j->resultados.res_eq2);
//
//
//    attron(COLOR_PAIR(1)); //TODO:meter a cor a 3
//    sprintf(clientes, "Numero Clientes: %d", j->resultados.numClientes);
//    mvaddstr(22, 41, clientes);
//
//    refresh();
//}
//

//void *recebe(void * dados) {
//    //    int * flag_log = (int *) dados;
//    int i, fd, flagCampo = 0;
//    char str[80], vencedor, msgFimJogo[80];
//    serv_clie jogada, ant;
//
//    sprintf(str, "/tmp/ccc%d", getpid());
//
//    while (1) //TODO: define condicao de paragem
//    {
//        fd = open(str, O_RDONLY);
//        i = read(fd, &jogada, sizeof (serv_clie));
//        //printf("\nXant: %d Yant: %d Xnovo: %d Ynovo: %d", jogada.xant, jogada.yant, jogada.xnovo, jogada.ynovo);
//
//        //printf("\nChegou fl:%d, fc %d", jogada.flag_logado, jogada.flag_campo);
//
//        if (i == sizeof (serv_clie)) {
//            //            if (jogada.flag_logado) 
//            //            {
//            //                *flag_log = 1;
//            //            }
//            if (jogada.flag_stop) {
//                //                clear();
//                //                attron(COLOR_PAIR(2)); //TODO:Mudar para branco
//                flagCampo = 0;
//
//                if (jogada.resultados.res_eq1 == jogada.resultados.res_eq2) {
//                    //                    sprintf(msgFimJogo, "O jogo terminou empatado (%d - %d).", jogada.resultados.res_eq1, jogada.resultados.res_eq2);
//                    //                    addstr(msgFimJogo);
//                } else {
//                    if (jogada.resultados.res_eq1 > jogada.resultados.res_eq2)
//                        vencedor = 'a';
//                    else
//                        vencedor = 'b';
//                    //                    sprintf(msgFimJogo, "O jogo terminou vencendo a equipa %c (%d - %d)", vencedor, jogada.resultados.res_eq1, jogada.resultados.res_eq2);
//                    //                    addstr(msgFimJogo);
//                }
//                //                refresh();
//            }
//        }
//        close(fd);
//    }
//    pthread_exit(0);
//}

void ligacao() {
    clie_serv novo;
    char str[80];

    novo.id = getpid();
    novo.flag_log = 0;
    novo.flag_con = 1;
    novo.flag_desliga = 0;
    novo.flag_operacao = 0;
    novo.flag_arbitro = 1;
    int fd = open(FIFO, O_WRONLY);
    sprintf(str, "/tmp/ccc%d", getpid());
    mkfifo(str, 0600);

    int a = write(fd, &novo, sizeof (clie_serv));

    if (a != sizeof (clie_serv)) {
        printf("\nERRO A ENVIAR DADOS(LOGIN)!!\n");
        close(fd);
        return;
    }
    printf("\n{LIGACAO} ENVIEI O %d\n. Tamanho da info enviada: %d", novo.id, a);
    close(fd);
}

//void logar(int * flag_log) {
//
//    clie_serv novo;
//    char user[80];
//    char pass[80];
//
//
//    int fd, fd_resp;
//    serv_clie flag;
//
//    do {
//        printf("\nuser: ");
//        scanf("%s", user); //TODO:DESCOMENTAR
//        //strcpy(user, "user1");//TODO:COMENTAR
//        printf("\npass: "); 
//        scanf("%s", pass); //TODO:DESCOMENTAR
//        //strcpy(pass, "pass1"); //TODO:COMENTAR
//        strcpy(novo.user, user);
//        strcpy(novo.pass, pass);
//        novo.id = getpid();
//        novo.flag_log = 1;
//        novo.flag_con = 0;
//        novo.flag_desliga = 0;
//        novo.flag_operacao = 0;
//
//        fd = open(FIFO, O_WRONLY);
//        if (write(fd, &novo, sizeof (clie_serv)) != sizeof (clie_serv)) {
//            printf("\nERRO A ENVIAR DADOS(LOGIN)!!\n");
//            close(fd);
//            return;
//        }
//        close(fd);
//        sleep(1);
//
//    } while ((*flag_log) == 0);
//    //printf("\nOLAOL!!\n");
//}

void desconetar(int s) {
    char str[80];
    sprintf(str, "/tmp/ccc%d", getpid());

    unlink(str);
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

        printf("%d\n", write(fd, &des, sizeof (clie_serv)));
        printf("%d\n", sizeof (clie_serv));

        close(fd);
        sair = 1;
        usleep(100);

        exit(3);
    } else if (s == SIGUSR1) {
        //        endwin();
        //        clear();
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

    clie_serv p;
    pthread_t tarefa1, tarefa2, tarefa3;


    if (access(FIFO_Arbitro, F_OK) != 0) {
        printf("servidor off\n");
        return 3;
    }

    envia_comando();
    return 0;

}

