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
    char tempo[20],clientes[20]; 
    if(j->equipa=='a')
        attron(COLOR_PAIR(2));
    else if(j->equipa=='b')
        attron(COLOR_PAIR(1));
    else   
        attron(COLOR_PAIR(3));
                
    mvaddch(j->xant, j->yant, ' ');

    mvaddch(j->xnovo, j->ynovo, j->jogador);
    
    sprintf(tempo, "Tempo: %d", j->resultados.tempo);
    
    attron(COLOR_PAIR(2));//TODO:meter a cor a 3
    mvaddstr(22,0,tempo);

    attron(COLOR_PAIR(2));
    mvaddch(22, 23, '0' + j->resultados.res_eq1);
    attron(COLOR_PAIR(3));
    mvaddch(22, 24, '-');
    attron(COLOR_PAIR(1));
    mvaddch(22, 25, '0' + j->resultados.res_eq2);
    
    
    attron(COLOR_PAIR(1));//TODO:meter a cor a 3
    sprintf(clientes, "Numero Clientes: %d", j->resultados.numClientes);
    mvaddstr(22,41,clientes);
        
    refresh();
}

void *recebe(void * dados) {
    int * flag_log = (int *) dados;
    int i, fd, flagCampo=0;
    char str[80], vencedor, msgFimJogo[80];
    serv_clie jogada, ant;

    sprintf(str, "/tmp/ccc%d", getpid());

     while (1) //TODO: define condicao de paragem
     {
        fd = open(str, O_RDONLY);
        i = read(fd, &jogada, sizeof (serv_clie));
        //printf("\nXant: %d Yant: %d Xnovo: %d Ynovo: %d", jogada.xant, jogada.yant, jogada.xnovo, jogada.ynovo);

        //printf("\nChegou fl:%d, fc %d", jogada.flag_logado, jogada.flag_campo);

        if (i == sizeof (serv_clie)) 
        {
            if (jogada.flag_logado) 
            {
                *flag_log = 1;
            }
            if (jogada.flag_campo) 
            {
                if(flagCampo==0)
                {
                    clear();
                    flagCampo=1;
                }
                if (jogada.resultados.res_eq1 != ant.resultados.res_eq1
                        || jogada.resultados.res_eq2 != ant.resultados.res_eq2) {
                    clear();
                    atualiza_campo(&jogada);
                } else {
                    atualiza_campo(&jogada);

                }
                ant = jogada;
            }
            else if (jogada.flag_stop) 
            {
                clear();
                attron(COLOR_PAIR(2));//TODO:Mudar para branco
                flagCampo=0;
                
                if(jogada.resultados.res_eq1==jogada.resultados.res_eq2)
                    {
                        sprintf(msgFimJogo, "O jogo terminou empatado (%d - %d).", jogada.resultados.res_eq1, jogada.resultados.res_eq2);
                        addstr(msgFimJogo);
                    }
                    else
                    {
                        if(jogada.resultados.res_eq1>jogada.resultados.res_eq2)
                            vencedor='a';
                        else
                            vencedor='b';
                        sprintf(msgFimJogo, "O jogo terminou vencendo a equipa %c (%d - %d)", vencedor, jogada.resultados.res_eq1, jogada.resultados.res_eq2);
                        addstr(msgFimJogo);
                    }
                refresh();
            }
        } 
        close(fd);
    }
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

    if (a != sizeof (clie_serv)) 
    {
        close(fd);
        return;
    }
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
        scanf("%s", user); //TODO:DESCOMENTAR
        //strcpy(user, "user1");//TODO:COMENTAR
        printf("\npass: "); 
        scanf("%s", pass); //TODO:DESCOMENTAR
        //strcpy(pass, "pass1"); //TODO:COMENTAR
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
    
    if (s == SIGINT) {
        endwin();
        clear();
        refresh();

        int fd;
        clie_serv des;

        fd = open(FIFO, O_WRONLY);
        des.id = getpid();
        des.flag_desliga = 1;
        des.flag_con = 0;
        des.flag_log = 0;
        des.flag_operacao = 0;

        close(fd);
        
        sprintf(str, "/tmp/ccc%d", getpid());

        unlink(str);

        exit(3);
    } else if (s == SIGUSR1) 
    {
        endwin();
        clear();
        refresh();
        printf("\nDesconetado pelo Servidor!\n");
        
        sprintf(str, "/tmp/ccc%d", getpid());

        unlink(str);

        exit(3);
    }
}

void envia_comando() {

    int tecla;
    int i, fd;
    clie_serv p;

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

        p.id = getpid();
        p.flag_operacao = 1;
        p.flag_con = 0;
        p.flag_log = 0;
        p.flag_desliga = 0;
        p.flag_arbitro = 0;

        fd = open(FIFO, O_WRONLY);

        i = write(fd, &p, sizeof (clie_serv));
        close(fd);
    }
}

int main(int argc, char** argv) {//TODO: AVISAR SERVER QUE SE CONETOU

    signal(SIGINT, SIG_IGN);

    signal(SIGINT, desconetar);
    //    signal(SIGUSR1, diz);

    clie_serv p;
    pthread_t tarefa1, tarefa2, tarefa3;
    int flag_log = 0;


    if (access(FIFO, F_OK) != 0) {
        printf("servidor off\n");
        return 3;
    }
    ligacao();

    pthread_create(&tarefa1, NULL, &recebe, (void *) &flag_log);
    pthread_create(&tarefa2, NULL, &recebe, (void *) &flag_log);
    //pthread_create(&tarefa3, NULL, &recebe, (void *) &flag_log);

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

    pthread_join(tarefa1, NULL);
    pthread_join(tarefa2, NULL);


    endwin();
    return 0;

}

