/*
 * CLIENTE
 */


#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "estrutura.h"

void atualiza_campo(serv_clie * j) 
{  
    if(j->equipa=='a')
        attron(COLOR_PAIR(2));
    else if(j->equipa=='b')
        attron(COLOR_PAIR(1));
    else
    {    
        attron(COLOR_PAIR(3));
        j->jogador='o';
    }
                
    mvaddch(j->xant, j->yant, ' ');

    mvaddch(j->xnovo, j->ynovo, j->jogador);
    
    attron(COLOR_PAIR(2));//TODO:meter a cor a 3
    mvaddch(22,0,'T');
    mvaddch(22,1,'e');
    mvaddch(22,2,'m');
    mvaddch(22,3,'p');
    mvaddch(22,4,'o');
    mvaddch(22,5,':');
    if(j->resultados.tempo<9)
        mvaddch(22, 6, '0' + j->resultados.tempo);
    else
    {
        mvaddch(22, 7, '1');
        mvaddch(22, 6, '0' + j->resultados.tempo%10);

    }

    attron(COLOR_PAIR(2));
    mvaddch(22, 23, '0' + j->resultados.res_eq1);
    attron(COLOR_PAIR(3));
    mvaddch(22, 24, '-');
    attron(COLOR_PAIR(1));
    mvaddch(22, 25, '0' + j->resultados.res_eq2);
    
    attron(COLOR_PAIR(1));//TODO:meter a cor a 3
    mvaddch(22,41,'C');
    mvaddch(22,42,'l');
    mvaddch(22,43,'i');
    mvaddch(22,44,'e');
    mvaddch(22,45,'n');
    mvaddch(22,46,'t');
    mvaddch(22,47,'e');
    mvaddch(22,48,'s');
    mvaddch(22,49,':');
    if(j->resultados.numClientes<9)
        mvaddch(22, 50, '0' + j->resultados.numClientes);
    else
    {
        mvaddch(22, 50, '1');
        mvaddch(22, 51, '0' + j->resultados.numClientes%10);
    }

    refresh();
}

void *recebe(void * dados) {
    int * flag_log = (int *) dados;

    int i, fd;
    char str[80], vencedor;
    serv_clie jogada, ant;


    sprintf(str, "/tmp/ccc%d", getpid());


    do {
        fd = open(str, O_RDONLY);
        i = read(fd, &jogada, sizeof (serv_clie));
        //printf("\nChegou fl:%d, fc %d", jogada.flag_logado, jogada.flag_campo);

        if (i == sizeof (serv_clie)) {
            //printf("\n{CLIENTE} Os dados recebidos do servidor tem o tamanho pretendido.\n");
            if (jogada.flag_logado) {
                //printf("\nVou meter a flag a 1\n");
                *flag_log = 1;
            }
            if (jogada.flag_campo) 
            {
                //printf("Cheguei ao atualiza campo");

                if(jogada.resultados.fim==1)
                {
                    clear();
                    if(jogada.resultados.res_eq1==jogada.resultados.res_eq2)
                    {
                        printf("O jogo terminou empatado (%d - %d).", jogada.resultados.res_eq1, jogada.resultados.res_eq2);
                    }
                    else
                    {
                        if(jogada.resultados.res_eq1>jogada.resultados.res_eq2)
                            vencedor='a';
                        else
                            vencedor='b';
                        printf("O jogo terminou vencendo a equipa %c (%d - %d)", vencedor, jogada.resultados.res_eq1, jogada.resultados.res_eq2);
                    }
                }
                if (jogada.resultados.res_eq1 != ant.resultados.res_eq1
                        || jogada.resultados.res_eq2 != ant.resultados.res_eq2) {
                    clear();
                    atualiza_campo(&jogada);
                } else {
                    atualiza_campo(&jogada);

                }
                ant = jogada;
            }//else if (jogada.flag_pedido) {
            //TODO: qq coisa
            //}

        } else {
            //printf("\n{CLIENTE} Os dados recebidos do servidor nao tem o tamanho pretendido.\n"
            //      "Tamanho pretendido: %d Tamanho recebido: %d \n", sizeof(serv_clie), i);
        }
        close(fd);
    } while (1);

    //printf("\nOLAOL!!\n");


    pthread_exit(0);
}

void ligacao() {
    clie_serv novo;
    char str[80];

    novo.id = getpid();
    novo.flag_log = 0;
    novo.flag_con = 1;
    novo.flag_desliga = 0;
    novo.flag_operacao = 0;
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

void logar(int * flag_log) {

    clie_serv novo;
    char user[80];
    char pass[80];


    int fd, fd_resp;
    serv_clie flag;

    do {
        printf("\nuser: ");
        scanf("%s", user);
        printf("\npass: ");
        scanf("%s", pass);
        strcpy(novo.user, user);
        strcpy(novo.pass, pass);
        novo.id = getpid();
        novo.flag_log = 1;
        novo.flag_con = 0;
        novo.flag_desliga = 0;
        novo.flag_operacao = 0;

        fd = open(FIFO, O_WRONLY);
        if (write(fd, &novo, sizeof (clie_serv)) != sizeof (clie_serv)) {
            printf("\nERRO A ENVIAR DADOS(LOGIN)!!\n");
            close(fd);
            return;
        }
        close(fd);
        sleep(1);

    } while ((*flag_log) == 0);
    //printf("\nOLAOL!!\n");
}

void desconetar(int s) {
    char str[80];
    sprintf(str, "/tmp/ccc%d", getpid());

    unlink(str);
    if (s == SIGINT) {
        endwin();

        int fd;
        clie_serv des;

        fd = open(FIFO, O_WRONLY);
        des.id = getpid();
        des.flag_desliga = 1;
        des.flag_con = 0;
        des.flag_log = 0;
        des.flag_operacao = 0;

        printf("%d\n", write(fd, &des, sizeof (clie_serv)));
        printf("%d\n", sizeof (clie_serv));

        close(fd);

        exit(3);
    } else if (s == SIGUSR1) {
        endwin();
        clear();
        printf("\nDesconetado pelo Servidor!\n");

        exit(3);
    }
}

void envia_comando() {

    int tecla;
    struct timeval tv;
    int ret, i, fd;
    clie_serv p;
    fd_set rfds;

    while (1) {

        tecla = getch();

        switch (tecla) {
            case 'a':
                p.op = 'a';
                break;
            case 'b':
                p.op = 'b';
                break;
            case '0':
                p.op = '0';
                break;
            case '1':
                p.op = '1';
                break;
            case '2':
                p.op = '2';
                break;
            case '3':
                p.op = '3';
                break;
            case '4':
                p.op = '4';
                break;
            case '5':
                p.op = '5';
                break;
            case '6':
                p.op = '6';
                break;
            case '7':
                p.op = '7';
                break;
            case '8':
                p.op = '8';
                break;
            case '9':
                p.op = '9';
                break;

            case KEY_UP:
                p.op = 'u';
                break;
            case KEY_DOWN:
                p.op = 'd';
                ;
                break;
            case KEY_LEFT:
                p.op = 'l';
                break;
            case KEY_RIGHT:
                p.op = 'r';
                break;
            default:
                p.op = 'h';
                break;
        }
        //printf("teclado");
        p.id = getpid();
        p.flag_operacao = 1;
        p.flag_con = 0;
        p.flag_log = 0;
        p.flag_desliga = 0;

        fd = open(FIFO, O_WRONLY);

        i = write(fd, &p, sizeof (clie_serv));
        close(fd);
        //}

        //printf("enviei um pedido... %d bytes\n", i);

    }
}

int main(int argc, char** argv) {//TODO: AVISAR SERVER QUE SE CONETOU

    int maxX, maxY;
    signal(SIGINT, SIG_IGN);

    signal(SIGINT, desconetar);
    //    signal(SIGUSR1, diz);

    int fd, i, res, fim = 0;
    clie_serv p;
    pthread_t tarefa, log;
    int flag_log = 0;


    if (access(FIFO, F_OK) != 0) {
        printf("servidor off\n");
        return 3;
    }
    ligacao();

    pthread_create(&tarefa, NULL, &recebe, (void *) &flag_log);
    logar(&flag_log);

    initscr();
    use_default_colors();
    start_color();
    init_pair(1, COLOR_BLUE, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_WHITE, -1);
    curs_set(0);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    clear();


    envia_comando();



    pthread_join(tarefa, NULL);

    endwin();
    return 0;

}

