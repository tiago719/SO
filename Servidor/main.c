/*
 * SERVIDOR
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
#include "../estrutura.h"

typedef struct {
    int y, x;
} POSICAO;

typedef struct {
    POSICAO orig;
    POSICAO dest;
} POSICOES_PASSE;

typedef struct {
    char equi;
    int tempo, num, humano, precisao_remate, falta;
    pthread_t thread;
    POSICAO posicao;
} JOGADOR;

typedef struct {
    int id, logado;
    char equi, username[80];
    JOGADOR *jogador;
} CLIENTE;

typedef struct {
    CLIENTE * c;
    int tam;
    char nome_ficheiro[80];
} CLIENTES;

POSICAO ball;
pthread_mutex_t trinco;
JOGADOR ** JOG;
JOGADOR * posse_bola;
int TOTAL;
CLIENTES clientes;
CLIENTE Arbitro;
RESULTADOS resultados;
int TempoJogo = 500;
int Ndefesa = -1, Navanc = -1, sair = 0;
pthread_t tempo, tarefa_bola, receber_cliente, receber_arbitro;

void bolaParaMeio() {
    ball.x = 11;
    ball.y = 26;
}

/*
 * 0 - NAO OCUPADO
 * 1 - OCUPADO
 * 2 - OCUPADO POR BOLA
 */
int isOcupado(int x, int y) {
    int a, i;

    for (a = 0; a < 2; a++) {
        for (i = 0; i < TOTAL; i++) {
            if (JOG[a][i].posicao.x == x && JOG[a][i].posicao.y == y) {
                return 1;
            }
        }
    }
    if (ball.x == x && ball.y == y)
        return 2;

    return 0;
}

void removeCliente(int index) {
    char str[80];
    sprintf(str, "/tmp/ccc%d", clientes.c[index].id);
    unlink(str);
    memset(&clientes.c[index], 0, sizeof (CLIENTE));
    if (index != clientes.tam - 1) {
        clientes.c[index] = clientes.c[clientes.tam - 1];
    }

    clientes.c = (CLIENTE *) realloc(clientes.c, sizeof (CLIENTE)*(--clientes.tam));

}

void resetClientes() {
    int i;
    for (i = 0; i < clientes.tam; i++) {
        clientes.c[i].equi = '-';
        clientes.c[i].jogador = NULL;
    }
}

void acabaJogo() {

    int i, fd;
    serv_clie j;

    resultados.fim = 1;
    posse_bola = NULL;
    for (i = 0; i < TOTAL; i++) {
        pthread_join(JOG[0][i].thread, NULL);
        pthread_join(JOG[1][i].thread, NULL);
    }

    pthread_join(tarefa_bola, NULL);

    resetClientes();

    usleep(100);

    for (i = 0; i < clientes.tam; i++) {
        if (clientes.c[i].logado) {
            char str[80];
            sprintf(str, "/tmp/ccc%d", clientes.c[i].id);

            if (access(str, F_OK) != 0) {
                removeCliente(i);
                continue;
            }
            fd = open(str, O_WRONLY);

            j.flag_campo = 0;
            j.flag_logado = 0;
            j.flag_stop = 1;
            j.resultados = resultados;

            write(fd, &j, sizeof (serv_clie));
            close(fd);
        }
    }
}

void atualizaCampo(serv_clie * j) {
    int i, fd;

    for (i = 0; i < clientes.tam; i++) {
        if (clientes.c[i].logado) {
            char str[80];
            sprintf(str, "/tmp/ccc%d", clientes.c[i].id);

            if (access(str, F_OK) != 0) {
                resultados.numClientes--;
                removeCliente(i);
                continue;
            }
            fd = open(str, O_WRONLY);
            j->flag_campo = 1;
            j->flag_logado = 1;
            j->flag_stop = 0;
            j->resultados = resultados;

            write(fd, j, sizeof (serv_clie));
            close(fd);
        }
    }
}

//void inicializaCampo() {
//    int i;
//    serv_clie j;
//
//    for (i = 0; i < TOTAL; i++) {
//        j.xnovo = JOG[0][i].posicao.x;
//        j.ynovo = JOG[0][i].posicao.y;
//        j.xant = JOG[0][i].posicao.x;
//        j.yant = JOG[0][i].posicao.y;
//        j.jogador = '0' + (JOG[0][i].num);
//        j.equipa = JOG[0][i].equi;
//
//        usleep(100000);
//
//        atualizaCampo(&j);
//        usleep(100);
//
//        j.xnovo = JOG[1][i].posicao.x;
//        j.ynovo = JOG[1][i].posicao.y;
//        j.xant = JOG[1][i].posicao.x;
//        j.yant = JOG[1][i].posicao.y;
//        j.jogador = '0' + (JOG[1][i].num);
//        j.equipa = JOG[1][i].equi;
//
//        atualizaCampo(&j);
//        usleep(100);
//    }
//
//    j.xnovo = ball.x;
//    j.ynovo = ball.y;
//    j.xant = ball.x;
//    j.yant = ball.y;
//    j.jogador = 'o';
//    j.equipa = 'n';
//    atualizaCampo(&j);
//    usleep(100);
//}
//

void montaCampo() {
    int i = 0;

    JOG[0][i].posicao.x = 11;
    JOG[0][i].posicao.y = 1;
    JOG[1][i].posicao.x = 11;
    JOG[1][i].posicao.y = 50;

    i++;
    switch (Ndefesa) {
        case 1:
            JOG[0][i].posicao.x = 11;
            JOG[0][i].posicao.y = 7;
            JOG[1][i].posicao.x = 11;
            JOG[1][i].posicao.y = 44;

            i++;
            break;
        case 2:
            JOG[0][i].posicao.x = 5;
            JOG[0][i].posicao.y = 7;
            JOG[1][i].posicao.x = 5;
            JOG[1][i].posicao.y = 44;

            i++;

            JOG[0][i].posicao.x = 16;
            JOG[0][i].posicao.y = 7;
            JOG[1][i].posicao.x = 16;
            JOG[1][i].posicao.y = 44;

            i++;
            break;
        case 3:
            JOG[0][i].posicao.x = 5;
            JOG[0][i].posicao.y = 7;
            JOG[1][i].posicao.x = 5;
            JOG[1][i].posicao.y = 44;

            i++;

            JOG[0][i].posicao.x = 16;
            JOG[0][i].posicao.y = 7;
            JOG[1][i].posicao.x = 16;
            JOG[1][i].posicao.y = 44;

            i++;

            JOG[0][i].posicao.x = 11;
            JOG[0][i].posicao.y = 9;
            JOG[1][i].posicao.x = 11;
            JOG[1][i].posicao.y = 42;

            i++;
            break;
        case 4:
            JOG[0][i].posicao.x = 5;
            JOG[0][i].posicao.y = 7;
            JOG[1][i].posicao.x = 5;
            JOG[1][i].posicao.y = 44;

            i++;

            JOG[0][i].posicao.x = 16;
            JOG[0][i].posicao.y = 7;
            JOG[1][i].posicao.x = 16;
            JOG[1][i].posicao.y = 44;

            i++;

            JOG[0][i].posicao.x = 5;
            JOG[0][i].posicao.y = 9;
            JOG[1][i].posicao.x = 5;
            JOG[1][i].posicao.y = 42;

            i++;

            JOG[0][i].posicao.x = 16;
            JOG[0][i].posicao.y = 9;
            JOG[1][i].posicao.x = 16;
            JOG[1][i].posicao.y = 42;

            i++;
            break;
    }

    switch (Navanc) {
        case 1:
            JOG[0][i].posicao.x = 11;
            JOG[0][i].posicao.y = 22;
            JOG[1][i].posicao.x = 11;
            JOG[1][i].posicao.y = 29;

            i++;
            break;
        case 2:
            JOG[0][i].posicao.x = 6;
            JOG[0][i].posicao.y = 22;
            JOG[1][i].posicao.x = 6;
            JOG[1][i].posicao.y = 29;

            i++;

            JOG[0][i].posicao.x = 17;
            JOG[0][i].posicao.y = 22;
            JOG[1][i].posicao.x = 17;
            JOG[1][i].posicao.y = 29;

            i++;
            break;
        case 3:
            JOG[0][i].posicao.x = 6;
            JOG[0][i].posicao.y = 22;
            JOG[1][i].posicao.x = 6;
            JOG[1][i].posicao.y = 29;

            i++;

            JOG[0][i].posicao.x = 17;
            JOG[0][i].posicao.y = 22;
            JOG[1][i].posicao.x = 17;
            JOG[1][i].posicao.y = 29;

            i++;

            JOG[0][i].posicao.x = 11;
            JOG[0][i].posicao.y = 24;
            JOG[1][i].posicao.x = 11;
            JOG[1][i].posicao.y = 27;

            i++;
            break;
        case 4:
            JOG[0][i].posicao.x = 9;
            JOG[0][i].posicao.y = 24;
            JOG[1][i].posicao.x = 9;
            JOG[1][i].posicao.y = 27;

            i++;

            JOG[0][i].posicao.x = 12;
            JOG[0][i].posicao.y = 24;
            JOG[1][i].posicao.x = 12;
            JOG[1][i].posicao.y = 27;

            i++;

            JOG[0][i].posicao.x = 6;
            JOG[0][i].posicao.y = 22;
            JOG[1][i].posicao.x = 6;
            JOG[1][i].posicao.y = 29;

            i++;

            JOG[0][i].posicao.x = 15;
            JOG[0][i].posicao.y = 22;
            JOG[1][i].posicao.x = 15;
            JOG[1][i].posicao.y = 29;

            i++;
            break;
    }
}

void inicializaJogadores() {
    int cont = 0, i;

    //INICIAOLIZACAO REDES
    JOG[0][0].humano = 0;
    JOG[0][0].num = cont;
    JOG[0][0].precisao_remate = 25;
    JOG[0][0].tempo = 300000;
    JOG[0][0].equi = 'a';

    JOG[1][0].humano = 0;
    JOG[1][0].num = cont;
    JOG[1][0].precisao_remate = 25;
    JOG[1][0].tempo = 300000;
    JOG[1][0].equi = 'b';



    cont++;
    //---------------------
    //INICIAOLIZACAO DEFESAS
    for (i = 1; i <= Ndefesa; i++) {
        JOG[0][i].humano = 0;
        JOG[0][i].num = cont;
        JOG[0][i].precisao_remate = 80;
        JOG[0][i].tempo = 400000;
        JOG[0][i].equi = 'a';

        JOG[1][i].humano = 0;
        JOG[1][i].num = cont;
        JOG[1][i].precisao_remate = 80;
        JOG[1][i].tempo = 400000;
        JOG[1][i].equi = 'b';

        cont++;
    }



    //---------------------
    //INICIAOLIZACAO AVANC
    cont = 6;
    for (; i < TOTAL; i++) {
        JOG[0][i].humano = 0;
        JOG[0][i].num = cont;
        JOG[0][i].precisao_remate = 60;
        JOG[0][i].tempo = 300000;
        JOG[0][i].equi = 'a';

        JOG[1][i].humano = 0;
        JOG[1][i].num = cont;
        JOG[1][i].precisao_remate = 60;
        JOG[1][i].tempo = 300000;
        JOG[1][i].equi = 'b';

        cont++;
    }

    //--------------------- 

    bolaParaMeio();
    montaCampo();
}

void verificaGolo() {
    int x = ball.x, y = ball.y;
    if ((y <= 0 || y >= 50) && (x > 5 && x < 14)) {

        if (y <= 0) {
            resultados.res_eq2++;
            posse_bola = &JOG[0][0];
        } else {
            resultados.res_eq1++;
            posse_bola = &JOG[1][0];
        }

        montaCampo();
        int i;
        char str[80];

        for (i = 0; i < clientes.tam; i++) {
            if (clientes.c[i].jogador != NULL) {
                clientes.c[i].jogador->humano = 0;
                clientes.c[i].jogador = NULL;
            }
            sprintf(str, "/tmp/ccc%d", clientes.c[i].id);
            //            inicializaCampo(str);
        }
    }
}

void * fazerPasse(void * dados) {
    POSICOES_PASSE * p = (POSICOES_PASSE *) dados;
    POSICAO orig = p->orig;
    POSICAO dest = p->dest;

    POSICAO d;

    JOGADOR *temp = posse_bola;
    posse_bola = NULL;

    int flag = 1;
    int ultX = 0, ultY = 0;

    while (ball.x > 0 && ball.x < MaxX
            && ball.y > 0 && ball.y < MaxY && posse_bola == NULL) {
        verificaGolo();
        d.x = 0;
        d.y = 0;

        if (flag) {
            if (orig.x > dest.x)
                d.x--;
            else if (orig.x < dest.x)
                d.x++;
            if (orig.y > dest.y)
                d.y--;
            else if (orig.y < dest.y)
                d.y++;
        } else {
            d.x += ultX;
            d.y += ultY;

        }

        ball.x += d.x;
        ball.y += d.y;

        if (ball.x == dest.x && ball.y == dest.y) {
            flag = 0;
            ultX = d.x;
            ultY = d.y;
        }

        orig.x += d.x;
        orig.y += d.y;

        usleep(temp->tempo);
    }
    pthread_exit(0);

}

void passe(POSICAO orig, POSICAO dest) {
    POSICOES_PASSE p;
    p.dest = dest;
    p.orig = orig;
    pthread_t tar;
    pthread_create(&tar, NULL, &fazerPasse, (void *) &p);
    return;

}

void * bola() {
    serv_clie j;
    int i, a;

    POSICAO anterior;
    anterior.x = ball.x;
    anterior.y = ball.y;

    j.jogador = 'o';
    j.equipa = 'n';

    while (!resultados.fim) {
        if (resultados.intervalo)
            continue;

        verificaGolo();
        j.xant = anterior.x;
        j.yant = anterior.y;
        j.xnovo = ball.x;
        j.ynovo = ball.y;


        if (posse_bola != NULL) {
            if (posse_bola->equi == 'a') {
                j.equipa = 'a';
                ball.y = posse_bola->posicao.y + 1;
            } else {
                j.equipa = 'b';
                ball.y = posse_bola->posicao.y - 1;

            }

            ball.x = posse_bola->posicao.x;
            j.ynovo = ball.y;
            j.xnovo = ball.x;

            for (a = 0; a < 2; a++) {
                for (i = 0; i < TOTAL; i++) {
                    if (JOG[a][i].posicao.x == ball.x && JOG[a][i].posicao.y == ball.y) {
                        posse_bola = &JOG[a][i];
                        break;
                    }
                }
            }
            j.resultados = resultados;
        }
        for (a = 0; a < 2; a++) {
            for (i = 0; i < TOTAL; i++) {
                if (JOG[a][i].posicao.x == ball.x && JOG[a][i].posicao.y == ball.y) {
                    posse_bola = &JOG[a][i];
                    break;
                }
            }
        }

        if (j.xant == j.xnovo && j.ynovo == j.yant)
            continue;
        if (posse_bola != NULL)
            if (j.xant == posse_bola->posicao.x && j.yant == posse_bola->posicao.y) {
                j.xant = 99;
                j.yant = 99;
            }

        atualizaCampo(&j);

        anterior.x = ball.x;
        anterior.y = ball.y;
        usleep(100);
    }
    pthread_exit(0);
}

void * move_jogador(void * dados) {
    JOGADOR *jog;
    jog = (JOGADOR *) dados;
    POSICAO d;
    serv_clie j;
    j.flag_campo = 1;
    j.flag_logado = 0;

    while (!resultados.fim) {
        if (jog->falta)
            continue;

        if (!jog->humano) {

            int r = rand() % 4;
            d.x = 0;
            d.y = 0;
            switch (r) {
                case 0:
                    d.x++;
                    break;
                case 1:
                    d.x--;
                    break;
                case 2:
                    d.y++;
                    break;
                case 3:
                    d.y--;
                    break;

            }

            if (jog->posicao.y + d.y >= 0 && jog->posicao.y + d.y < MaxY &&
                    jog->posicao.x + d.x >= 0 && jog->posicao.x + d.x < MaxX) {
                pthread_mutex_lock(&trinco);

                if (isOcupado(jog->posicao.x + d.x, jog->posicao.y + d.y) == 1) 
                {
                    pthread_mutex_unlock(&trinco);
                    continue;
                }

                j.jogador = '0' + jog->num;
                j.equipa = jog->equi;

                j.xant = jog->posicao.x;
                j.yant = jog->posicao.y;

                jog->posicao.y += d.y;
                jog->posicao.x += d.x;

                j.xnovo = jog->posicao.x;
                j.ynovo = jog->posicao.y;
                j.resultados = resultados;
                pthread_mutex_unlock(&trinco);

                atualizaCampo(&j);
            }
            usleep(jog->tempo);
        }

    }
    pthread_exit(0);
}

void * move_redes(void * dados) {
    JOGADOR *jog;
    jog = (JOGADOR *) dados;
    POSICAO d;
    serv_clie j;

    while (!resultados.fim) {
        if (jog->falta)
            continue;

        if (!jog->humano) {
            
            
            
            d.x = 0;
            int r = rand() % 2;

            if (r == 0) {
                d.x = 1;
            } else {
                d.x = -1;
            }
            if (jog->posicao.x + d.x >= limInfXRedes && jog->posicao.x + d.x <= limSupXRedes) {
                pthread_mutex_lock(&trinco);
                if (isOcupado(jog->posicao.x + d.x, jog->posicao.x + d.y) == 1) {
                    pthread_mutex_unlock(&trinco);
                    continue;
                }

                j.xant = jog->posicao.x;
                j.yant = jog->posicao.y;

                jog->posicao.y += d.y;
                jog->posicao.x += d.x;

                j.xnovo = jog->posicao.x;
                j.ynovo = jog->posicao.y;

                j.jogador = '0' + jog->num;
                j.equipa = jog->equi;

                pthread_mutex_unlock(&trinco);
                atualizaCampo(&j);
            }
            if (jog == posse_bola){
                sleep(2.5);
               
                
                int a = posse_bola->equi == 'a' ? 0 : 1;
                int b = rand() % TOTAL;
                POSICAO p = JOG[a][b].posicao;
                passe(ball, p);
                
            }
        }
        usleep(jog->tempo);
    }
    pthread_exit(0);
}

JOGADOR * procuraJogador(char equi, int op) {
    int i, eq;
    if (equi == 'a')
        eq = 0;
    else if (equi == 'b')
        eq = 1;
    else
        return NULL;

    for (i = 0; i < TOTAL; i++) {
        if (JOG[eq][i].num == op)
            return &JOG[eq][i];
    }


    return NULL;
}

void interpreta_comando(int cam, CLIENTES * cli, clie_serv * novo) {
    int i, a, flag = 0;
    POSICAO temp;
    JOGADOR * aux;

    for (i = 0; i < cli->tam; i++) {
        if (novo->id == cli->c[i].id) {
            if (cli->c[i].equi != '-') {
                if (cli->c[i].jogador == NULL) {
                    aux = procuraJogador(cli->c[i].equi, cam);

                    for (a = 0; a < cli->tam; a++) {
                        if (cli->c[a].jogador == aux) {
                            //o jogador pretendido ja e controlado 
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 0) {
                        cli->c[i].jogador = aux;
                        cli->c[i].jogador->humano = 1;
                    }
                } else if (cli->c[i].jogador->num == cam) {
                    cli->c[i].jogador->humano = 0;
                    cli->c[i].jogador = NULL;
                } else {
                    //passa para o cam
                    if (cli->c[i].jogador == posse_bola) {
                        if ((rand() % 100) < cli->c[i].jogador->precisao_remate) {

                            passe(ball, procuraJogador(cli->c[i].equi, cam)->posicao);
                        } else {
                            int aux = rand() % 4;
                            if (aux == 0) {
                                temp.x = 0;
                                temp.y = rand() % (MaxY - 1);
                            } else
                                if (aux == 1) {
                                temp.y = 0;
                                temp.x = rand() % (MaxX - 1);
                            } else if (aux == 2) {
                                temp.y = MaxY - 1;
                                temp.x = rand() % (MaxX - 1);
                            } else {
                                temp.x = MaxX - 1;
                                temp.y = rand() % (MaxY - 1);
                            }
                            passe(ball, temp);
                        }
                    }
                }
            }
            break;
        }
    }
}

void controlaJogador(int op, CLIENTE * cliente) {

    int xSum = 0, ySum = 0, maxXJog = MaxX, minXJog = 0;
    serv_clie j;

    switch (op) {
        case 'u':
            xSum = -1;
            break;
        case 'd':
            xSum = 1;
            break;
        case 'l':
            ySum = -1;
            break;
        case 'r':
            ySum = 1;
            break;
    }

    if (cliente->jogador == NULL)
        return;

    if (cliente->jogador->falta == 1)
        return;

    if (cliente->jogador->num == 0)//se redes
    {
        ySum = 0;
        maxXJog = limSupXRedes;
        minXJog = limInfXRedes;
    }

    xSum += cliente->jogador->posicao.x;
    ySum += cliente->jogador->posicao.y;

    if (xSum < minXJog || xSum > maxXJog || ySum < 0 || ySum > MaxY)
        return;

    switch (isOcupado(xSum, ySum)) {
        case 1:
            return;
        case 2:
            if (!resultados.intervalo)
                posse_bola = cliente->jogador;
            break;
        default:
            break;
    }

    j.xant = cliente->jogador->posicao.x;
    j.yant = cliente->jogador->posicao.y;

    cliente->jogador->posicao.x = xSum;
    cliente->jogador->posicao.y = ySum;



    j.jogador = '0' + cliente->jogador->num;
    j.equipa = cliente->jogador->equi;

    j.xnovo = cliente->jogador->posicao.x;
    j.ynovo = cliente->jogador->posicao.y;
    atualizaCampo(&j);

    usleep(cliente->jogador->tempo);
}

void operacao(clie_serv *cliente, CLIENTES * cli) {
    int i, fd;
    serv_clie j;
    //    printf("\n A operacao e %d\n", cliente->op);
    switch (cliente->op) {
        case 'a':
            //printf("\nEntrei no case A\n");
            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente->id) {
                    if (cli->c[i].equi == '-') {
                        //printf("\nESCOLHEU EQUIPA A\n");
                        cli->c[i].equi = 'a';
                    }
                    break;
                }
            }
            break;
        case 'b':
            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente->id) {
                    //printf("\nESCOLHEU EQUIPA B\n");
                    if (cli->c[i].equi == '-') {
                        cli->c[i].equi = 'b';
                    }
                    break;
                }
            }
            break;
        case '0':
            interpreta_comando(0, cli, cliente);
            break;
        case '1':
            interpreta_comando(1, cli, cliente);
            break;
        case '2':
            interpreta_comando(2, cli, cliente);
            break;
        case '3':
            interpreta_comando(3, cli, cliente);
            break;
        case '4':
            interpreta_comando(4, cli, cliente);
            break;
        case '5':

            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente->id) {
                    //printf("\nEncontreime no array de cli\n");
                    if (cli->c[i].logado == 1) {
                        if (cli->c[i].jogador != NULL) {
                            //printf("\nO jogador existe %d\n a posse e de %d" ,cli->c[i].jog, posse_bola->num);
                            POSICAO baliza;
                            POSICAO temp;
                            if (posse_bola == cli->c[i].jogador) {
                                //printf("\nI jogador tem posse\n");
                                if (cli->c[i].equi == 'a') {
                                    baliza.y = 50;
                                } else {
                                    baliza.y = 0;
                                }

                                baliza.x = rand() % (15 - 6 + 1) + 6;
                                if ((rand() % 100) < cli->c[i].jogador->precisao_remate) {
                                    //printf("\nVou para o sitio certo\n");
                                    passe(ball, baliza);
                                } else {
                                    //printf("\nVou para o sitio aleatorio\n");
                                    int aux = rand() % 4;
                                    if (aux == 0) {
                                        temp.x = 0;
                                        temp.y = rand() % MaxY - 1;
                                    } else
                                        if (aux == 1) {
                                        temp.y = 0;
                                        temp.x = rand() % MaxX - 1;
                                    } else
                                        if (aux == 2) {
                                        temp.y = MaxY - 1;
                                        temp.x = rand() % MaxX - 1;
                                    } else {
                                        temp.x = MaxX - 1;
                                        temp.y = rand() % MaxY - 1;
                                    }
                                    passe(ball, baliza);
                                }
                            }
                        }
                    }
                }
            }

            break;
        case '6':
            interpreta_comando(6, cli, cliente);
            break;
        case '7':
            interpreta_comando(7, cli, cliente);
            break;
        case '8':
            interpreta_comando(8, cli, cliente);
            break;
        case '9':
            interpreta_comando(9, cli, cliente);
            break;

        case 'u':

            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente->id) {
                    break;
                }
            }
            //printf("\ncontrolas este %d\n", cli->c[i].jog);
            if (cli->c[i].jogador == NULL || cli->c[i].equi == '-') {
                //printf("\nNao controlas ninguem, vou sair\n");
                break;
            } else {
                controlaJogador(cliente->op, &cli->c[i]);
            }
            break;

        case 'd':

            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente->id) {
                    break;
                }
            }
            if (cli->c[i].jogador == NULL || cli->c[i].equi == '-') {
                break;
            } else {
                controlaJogador(cliente->op, &cli->c[i]);
            }
            break;
        case 'l':

            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente->id) {
                    break;
                }
            }
            if (cli->c[i].jogador == NULL || cli->c[i].equi == '-')
                break;
            else {
                controlaJogador(cliente->op, &cli->c[i]);
            }
            break;

        case 'r':
            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente->id) {
                    break;
                }
            }
            if (cli->c[i].jogador == NULL || cli->c[i].equi == '-')
                break;
            else {
                controlaJogador(cliente->op, &cli->c[i]);
            }
            break;
            //ultimo case
        default:
            break;
    }
}

void comecaJogo() {
    int i;

    inicializaJogadores();
    resultados.res_eq1 = 0;
    resultados.res_eq2 = 0;
    resultados.fim = 0;

    pthread_create(&tarefa_bola, NULL, &bola, NULL);

    pthread_create(&JOG[0][0].thread, NULL, &move_redes, (void *) &JOG[0][0]);
    pthread_create(&JOG[1][0].thread, NULL, &move_redes, (void *) &JOG[1][0]);

    for (i = 1; i < TOTAL; i++) {
        pthread_create(&JOG[0][i].thread, NULL, &move_jogador, (void *) &JOG[0][i]);
        pthread_create(&JOG[1][i].thread, NULL, &move_jogador, (void *) &JOG[1][i]);
    }
}

void * contar_seg() {
    while (resultados.tempo > 0 && !resultados.fim) {
        if (resultados.intervalo)
            continue;

        resultados.tempo--;
        sleep(1);
    }
}

void shutdown() {
    acabaJogo();

    int i;
    char str[20];

    resultados.fim = 1;



    for (i = 0; i < clientes.tam; i++) {
        kill(clientes.c[i].id, SIGUSR1);
        sprintf(str, "/tmp/ccc%d", clientes.c[i].id);
        unlink(str);
    }
    if (Arbitro.id != -1) {
        kill(Arbitro.id, SIGUSR1);
        unlink(FIFO_Arbitro);
    }
    if (clientes.tam > 0)
        free(clientes.c);

    free(JOG[0]);
    free(JOG[1]);
    free(JOG);

    unlink(FIFO);
//    unlink(FIFO_Arbitro);

    pthread_cancel(receber_arbitro);
    pthread_cancel(receber_cliente);

    pthread_join(tempo, NULL);

    exit(3);
}

void trataSinal(int s) {

    if (s == SIGALRM) {
        if (resultados.intervalo == 1)
            return;
        printf("\nACABOU TEMPO\n");
        acabaJogo();
        return;
    }

    if (s == SIGINT || s == SIGUSR1) {
        shutdown();
    }
}

void mandaBola(char str[]) {
    int fd;
    serv_clie j;
    j.flag_campo = 1;
    j.flag_stop = 0;
    j.flag_logado = 1;

    j.xant = ball.x;
    j.yant = ball.y;
    j.xnovo = ball.x;
    j.ynovo = ball.y;
    j.equipa = 'n';
    j.jogador = 'o';
    j.resultados = resultados;

    fd = open(str, O_WRONLY);
    write(fd, &j, sizeof (serv_clie));
    close(fd);

}

//------ARBITRO------

void AR_Inicio() {
    if (resultados.fim != 1) {
        printf("Nao pode dar incio a partida\n");
        return;
    }
    resultados.tempo = TempoJogo;
    alarm(TempoJogo);
    pthread_create(&tempo, NULL, &contar_seg, NULL);
    comecaJogo();
}

void AR_Itervalo() {
    int i;
    char str[20];
    if (resultados.intervalo == 1) {
        printf("\nJa esta em intervalo!\n");
        return;
    }
    bolaParaMeio();

    for (i = 0; i<clientes.tam; i++) {
        sprintf(str, "/tmp/ccc%d", clientes.c[i].id);
        mandaBola(str);
    }

    resultados.intervalo = 1;
    alarm(0);
    posse_bola = NULL;
}

void AR_Recomeca() {
    int i;
    char str[20];
    if (resultados.intervalo != 1) {
        printf("\nNao esta em intervalo!\n");
        return;
    }
    resultados.intervalo = 0;
    alarm(resultados.tempo);
    for (i = 0; i<clientes.tam; i++) {
        sprintf(str, "/tmp/ccc%d", clientes.c[i].id);
        mandaBola(str);
    }
}

void AR_Falta() {
    if (resultados.fim == 1) {
        printf("Nenhum jogo a decorrer!\n");
        return;
    }

    JOGADOR * aux;
    if (posse_bola == NULL)
        return;
    posse_bola->falta = 1;
    aux = posse_bola;
    posse_bola = NULL;
    sleep(5);
    aux->falta = 0;
}

void AR_Termina() {
    if (resultados.fim == 1) {
        printf("Nao esta a decorrer nenhum jogo!\n");
        return;
    }
    acabaJogo();
}

//-------------------



void * ReceberCliente(void * dados) {

    clie_serv cliente;
    char str[80];
    int fd, fd_resp, i;

    serv_clie j;
    j.flag_logado = 0;
    j.flag_campo = 0;

    char temp_user[80], temp_pass[80];
    int a;
    while (!sair) {
        if (access(FIFO_Arbitro, F_OK) != 0)
            continue;
        fd = open(FIFO, O_RDONLY);

        a = read(fd, &cliente, sizeof (clie_serv));

        if (a != sizeof (clie_serv)) {
            close(fd);
            continue;
        }
        close(fd);

        if (cliente.flag_con) {
            if (clientes.tam < 19) {
                if (clientes.tam == 0)
                    clientes.c = (CLIENTE *) malloc(sizeof (CLIENTE));
                else
                    clientes.c = (CLIENTE *) realloc(clientes.c, sizeof (CLIENTE)*(clientes.tam + 1));

            } else {
                printf("Mais que 19 clientes!\n");
                continue;
            }

            clientes.c[clientes.tam].id = cliente.id;
            clientes.c[clientes.tam].jogador = NULL;
            clientes.c[clientes.tam].equi = '-';
            clientes.c[clientes.tam].logado = 0;

            clientes.tam++;

        } else if (cliente.flag_log) {//logar
            FILE * f = fopen(clientes.nome_ficheiro, "rt");
            if (!f) {
                printf("Nao encontra ficheiro!!\n");
                pthread_exit(0);
            }
            int aux = 0, t;
            j.flag_logado = 0;
            j.flag_campo = 0;
            j.flag_stop = 0;

            for (i = 0; i < clientes.tam; i++) {
                if (clientes.c[i].id == cliente.id) {
                    while (fscanf(f, " %s %s", temp_user, temp_pass) == 2) {
                        if (strcmp(temp_user, cliente.user) == 0 && strcmp(temp_pass, cliente.pass) == 0) {
                            for (t = 0; t < clientes.tam; t++) {
                                if (strcmp(cliente.user, clientes.c[t].username) == 0) {

                                    j.flag_logado = 0;
                                    aux = 1;
                                }
                            }
                            if (aux == 0) {
                                clientes.c[i].logado = 1;
                                j.flag_logado = 1;
                                resultados.numClientes++;
                                strcpy(clientes.c[i].username, temp_user);
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            sprintf(str, "/tmp/ccc%d", cliente.id);
            if (access(str, F_OK) != 0) {
                printf("cliente off\n");
            }
            fd_resp = open(str, O_WRONLY);

            write(fd_resp, &j, sizeof (serv_clie));
            close(fd_resp);

            fclose(f);
            sleep(1);
            mandaBola(str);
        } else if (cliente.flag_desliga) {

            for (i = 0; i < clientes.tam; i++) {
                if (clientes.c[i].id == cliente.id) {
                    removeCliente(i);
                    break;
                }
            }
            resultados.numClientes--;
        } else if (cliente.flag_operacao) {
            operacao(&cliente, &clientes);
        }

    }//while
    pthread_exit(0);
}

void * ReceberArbitro(void * dados) {
    int fd;
    arbitro_serv op;
    while (!sair) {
        if (access(FIFO_Arbitro, F_OK) != 0)
            continue;
        fd = open(FIFO_Arbitro, O_RDONLY);
        read(fd, &op, sizeof (arbitro_serv));

        Arbitro.id = op.pid;
        switch (op.op) {
            case 0:
                AR_Inicio();
                break;
            case 1:
                AR_Itervalo();
                break;
            case 2:
                AR_Recomeca();
                break;
            case 3:
                AR_Falta();
                break;
            case 4:
                AR_Termina();
                break;
            case -1:
                Arbitro.id = -1;
                break;
        }
    }
    pthread_exit(0);
}

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("SINTAXE: %s {nome_ficheiro}\n", argv[0]);
        return 1;
    }

    strcpy(clientes.nome_ficheiro, argv[1]);

    srand((unsigned int) time(NULL));
    signal(SIGINT, trataSinal);
    signal(SIGALRM, trataSinal);
    signal(SIGUSR1, trataSinal);

    char *primeiro;
    char *segundo;
    FILE *f;
    Arbitro.id = -1;

    int i, flag = 0;
    char cmd[80];

    clientes.tam = 0;
    resultados.fim = 1;
    resultados.numClientes = clientes.tam;

    const char* AVndefesa = getenv("NDEFESAS");
    const char* AVnavanc = getenv("NAVANCADOS");

    if (AVndefesa != NULL)
        Ndefesa = atoi(AVndefesa);

    if (AVnavanc != NULL)
        Navanc = atoi(AVnavanc);

    if (Ndefesa < 0 || Ndefesa > 4) {
        printf("Numero de DEFESAS invalido atribuido 2\n");
        Ndefesa = 2;
    }
    if (Navanc < 0 || Navanc > 4) {
        printf("Numero de AVANCADOS invalido atribuido 2\n");
        Navanc = 2;
    }

    TOTAL = (1 + Navanc + Ndefesa);

    JOG = (JOGADOR **) malloc(sizeof (JOGADOR) * 2);

    JOG[0] = (JOGADOR *) malloc(sizeof (JOGADOR) * TOTAL);
    JOG[1] = (JOGADOR *) malloc(sizeof (JOGADOR) * TOTAL);

    if (JOG == NULL || JOG[0] == NULL || JOG[1] == NULL) {
        printf("\nNao foi possivel alocar memoria\n");
        return 0;
    }

    inicializaJogadores();

    if (access(FIFO, F_OK) == 0) {
        printf("Ja esta um servidor em execução\n");
        return 3;
    }

    mkfifo(FIFO, 0600);
    mkfifo(FIFO_Arbitro, 0600);

    pthread_create(&receber_cliente, NULL, &ReceberCliente, NULL);
    pthread_create(&receber_arbitro, NULL, &ReceberArbitro, NULL);




    do {
        printf("\nComando: ");

        scanf(" %[^\n]", cmd);

        char comandos[7][30] = {"start", "stop", "user", "users", "result", "red", "shutdown"};

        primeiro = strtok(cmd, " ");

        for (i = 0; i < 7; i++) {
            if (strcmp(primeiro, comandos[i]) == 0) {
                break;
            }
        }

        switch (i) {
            case 0://START
                primeiro = strtok(NULL, " "); // tempo
                if (primeiro == NULL) {
                    printf("\nErro de sintaxe: start {n}\n");
                    break;

                }
                if (resultados.fim == 1) {
                    resultados.tempo = atoi(primeiro);
                    TempoJogo = resultados.tempo;
                    sleep(1);
                } else {
                    printf("\nEsta a decorrer um jogo!\n");
                }
                break;

            case 1://STOP
                if (!resultados.fim)
                    acabaJogo();
                else
                    printf("\nNao esta nenhum jogo a decorrer\n");
                break;

            case 2://USER
                flag = 0;
                primeiro = strtok(NULL, " ");
                segundo = strtok(NULL, " ");

                if (primeiro == NULL || segundo == NULL) {
                    printf("\nErro de sintaxe: user {username} {password}\n");
                    break;
                }

                f = fopen(clientes.nome_ficheiro, "rt");

                if (!f) {
                    printf("erro ao abrir o ficheiro de utilizadores");
                    break;
                }
                char user[80];
                char pass[80];

                while (fscanf(f, "%s %s", user, pass) == 2) {
                    if (strcmp(user, primeiro) == 0) {

                        printf("\nUtilizador ja existe\n");
                        fclose(f);
                        flag = 1;
                        break;
                    }
                }

                if (!flag) {
                    f = fopen(clientes.nome_ficheiro, "at");

                    if (!f) {
                        printf("Erro ao abrir o ficheiro de utilizadores");
                        break;
                    }

                    fprintf(f, "\n%s %s", primeiro, segundo);
                    printf("Utilizador registado!");

                    fclose(f);
                }
                break;
            case 3://USERS
                printf("\nClientes Logados: \n");
                for (i = 0; i < clientes.tam; i++) {
                    if (clientes.c[i].logado == 1) {
                        printf("Username: %s  Pid: %d\n", clientes.c[i].username, clientes.c[i].id);
                    }
                }
                break;
            case 4://RESULT
                if (resultados.fim == 1) {
                    printf("\nNao esta nenhum jogo a decorrer\n");
                    break;
                }
                printf("\nEquipa a: %d - Equipa b: %d\n", resultados.res_eq1, resultados.res_eq2); //mostrar o resultado ao admin
                break;
            case 5://RED
                flag = 0;
                if (segundo == NULL) {
                    printf("\nErro de sintaxe: red {username}\n");
                    break;
                }
                segundo = strtok(NULL, " ");
                for (i = 0; i < clientes.tam; i++) {
                    if (strcmp(clientes.c[i].username, segundo) == 0) {
                        if (clientes.c[i].jogador != NULL) {
                            clientes.c[i].jogador->humano = 0;
                            clientes.c[i].jogador = NULL;
                        }
                        flag = 1;

                        break;
                    }
                }
                if (!flag)
                    printf("\nUtilizador nao logado\n");
                break;

            case 6://shutdown
                sair = 1;
                shutdown();
                break;
            default:
                printf("\nComando Invalido!\n");
                break;
        }

    } while (!sair);
}

