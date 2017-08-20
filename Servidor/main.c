/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   servidor.c
 *
 * Created on 4 de Janeiro de 2017, 17:11
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

typedef struct {
    int y, x;
} POSICAO;
;
typedef struct {
    char fim, humano, equi;
    int tempo, num, x, y, posse_bola;
    int precisao_remate;
    POSICAO *posicao;
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

POSICAO *ele;
pthread_mutex_t trinco;
JOGADOR ** JOG;
JOGADOR * posse_bola;
int total;
CLIENTES clientes;
RESULTADOS resultados;
int Ndefesa, Navanc, sair = 0, flag_fimJogo = 1;
pthread_t jogo, tempo;

void atualiza_campo(serv_clie * j) {
    int i, fd, cont = 0;


    for (i = 0; i < clientes.tam; i++) {
        if (clientes.c[i].logado) {
            char str[80];
            sprintf(str, "/tmp/ccc%d", clientes.c[i].id);
            fd = open(str, O_WRONLY);
            j->flag_campo = 1;
            j->flag_logado = 1;
            j->resultados = resultados;
            int a = write(fd, j, sizeof (serv_clie));

            printf("\n{Servidor} Dados enviados para o cliente %d.\n" "Tamanho pretendido: %d Tamanho enviado: %d \n Jogador enviado: %c", clientes.c[i].id, sizeof (serv_clie), a, j->jogador);
            close(fd);
        }


    }
}

void inicializacao_campo(char * str) {
    int cont = 0, i;
    int fd;


    serv_clie j;
    j.flag_campo = 1;
    j.resultados = resultados;


    for (i = 0; i < total; i++) {
        j.xnovo = ele[cont].x;
        j.ynovo = ele[cont].y;
        j.xant = ele[cont].x;
        j.yant = ele[cont].y;
        j.jogador = '0' + (JOG[0][i].num);
        j.equipa='a';

        cont++;
        atualiza_campo(&j);
        usleep(100);

    }
    for (i = 0; i < total; i++) {
        j.xnovo = ele[cont].x;
        j.ynovo = ele[cont].y;
        j.xant = ele[cont].x;
        j.yant = ele[cont].y;
        j.jogador = '0' + (JOG[1][i].num);
        j.equipa='b';

        cont++;
        atualiza_campo(&j);
        usleep(100);


    }
    j.xnovo = ele[cont].x;
    j.ynovo = ele[cont].y;
    j.xant = ele[cont].x;
    j.yant = ele[cont].y;
    j.jogador = 'o';
    j.equipa='n';
    atualiza_campo(&j);
    usleep(100);
}

void inicializaJog(){
    int cont=0, i, id=0;
    JOG = (JOGADOR **) malloc(sizeof (JOGADOR) * 2);

    JOG[0] = (JOGADOR *) malloc(sizeof (JOGADOR) * total);
    JOG[1] = (JOGADOR *) malloc(sizeof (JOGADOR) * total);

    //INICIAOLIZACAO REDES
    JOG[0][0].fim = 0;
    JOG[0][0].humano = 0;
    JOG[0][0].num = cont;
    JOG[0][0].posicao = &ele[id];
    JOG[0][0].precisao_remate = 25;
    JOG[0][0].tempo = 300000;
    JOG[0][0].equi='a';
    
    JOG[1][0].fim = 0;
    JOG[1][0].humano = 0;
    JOG[1][0].num = cont;
    JOG[1][0].posicao = &ele[total+(id++)];
    JOG[1][0].precisao_remate = 25;
    JOG[1][0].tempo = 300000;
    JOG[1][0].equi='b';
        
    cont++;
    //---------------------
    //INICIAOLIZACAO DEFESAS
    for (i = 1; i <= Ndefesa; i++) {
        JOG[0][i].fim = 0;
        JOG[0][i].humano = 0;
        JOG[0][i].num = cont;
        JOG[0][i].posicao = &ele[id];
        JOG[0][i].precisao_remate = 80;
        JOG[0][i].tempo = 400000;
        JOG[0][i].equi='a';
        
        JOG[1][i].fim = 0;
        JOG[1][i].humano = 0;
        JOG[1][i].num = cont;
        JOG[1][i].posicao = &ele[total+(id++)];
        JOG[1][i].precisao_remate = 80;
        JOG[1][i].tempo = 400000;
        JOG[1][i].equi='b';

        cont++;
    }
    //---------------------
    //INICIAOLIZACAO AVANC

    cont = 6;
    for (; i < total; i++) {
        JOG[0][i].fim = 0;
        JOG[0][i].humano = 0;
        JOG[0][i].num = cont;
        JOG[0][i].posicao = &ele[id];
        JOG[0][i].precisao_remate = 60;
        JOG[0][i].tempo = 300000;
        JOG[0][i].equi='a';
        
        JOG[1][i].fim = 0;
        JOG[1][i].humano = 0;
        JOG[1][i].num = cont;
        JOG[1][i].posicao = &ele[total+(id++)];
        JOG[1][i].precisao_remate = 60;
        JOG[1][i].tempo = 300000;
        JOG[1][i].equi='b';
        
        cont++;
    }
    //--------------------- 
}

void montaCampo(){
    int i;
    ele[0].x = 11;
    ele[0].y = 1;
    ele[total].x = 11;
    ele[total].y = 50;

    i = 1;

    switch (Ndefesa) {
        case 1:
            ele[i].x = 11;
            ele[i].y = 7;
            ele[i + total].x = 11;
            ele[i + total].y = 44;
            break;
        case 2:
            ele[i].x = 5;
            ele[i].y = 7;
            ele[i + total].x = 5;
            ele[i + total].y = 44;
            i++;
            ele[i].x = 16;
            ele[i].y = 7;
            ele[i + total].x = 16;
            ele[i + total].y = 44;

            break;
        case 3:
            ele[i].x = 5;
            ele[i].y = 7;
            ele[i + total].x = 5;
            ele[i + total].y = 44;
            i++;
            ele[i].x = 16;
            ele[i].y = 7;
            ele[i + total].x = 16;
            ele[i + total].y = 44;
            i++;
            ele[i].x = 11;
            ele[i].y = 9;
            ele[i + total].x = 11;
            ele[i + total].y = 42;
            break;
        case 4:
            ele[i].x = 5;
            ele[i].y = 7;
            ele[i + total].x = 5;
            ele[i + total].y = 44;
            i++;
            ele[i].x = 16;
            ele[i].y = 7;
            ele[i + total].x = 16;
            ele[i + total].y = 44;
            i++;
            ele[i].x = 5;
            ele[i].y = 9;
            ele[i + total].x = 5;
            ele[i + total].y = 42;
            i++;
            ele[i].x = 16;
            ele[i].y = 9;
            ele[i + total].x = 16;
            ele[i + total].y = 42;
            break;

    }

    i++;
    switch (Navanc) {
        case 1:
            ele[i].x = 11;
            ele[i].y = 22;
            ele[i + 10].x = 11;
            ele[i + 10].y = 29;
            break;
        case 2:
            ele[i].x = 6;
            ele[i].y = 22;
            ele[i + 10].x = 6;
            ele[i + 10].y = 29;
            i++;
            ele[i].x = 17;
            ele[i].y = 22;
            ele[i + 10].x = 17;
            ele[i + 10].y = 29;
            break;
        case 3:
            ele[i].x = 6;
            ele[i].y = 22;
            ele[i + 10].x = 6;
            ele[i + 10].y = 29;
            i++;
            ele[i].x = 17;
            ele[i].y = 22;
            ele[i + 10].x = 17;
            ele[i + 10].y = 29;
            i++;
            ele[Ndefesa + 3 + 1].x = 11;
            ele[Ndefesa + 3 + 1].y = 24;
            ele[i + 10].x = 11;
            ele[i + 10].y = 27;
            break;
        case 4:
            ele[i].y = 24;
            ele[i].x = 9;
            ele[i + total].x = 9;
            ele[i + total].y = 27;
            i++;
            ele[i].x = 12;
            ele[i].y = 24;
            ele[i + total].x = 12;
            ele[i + total].y = 27;
            i++;
            ele[i].x = 6;
            ele[i].y = 22;
            ele[i + total].x = 6;
            ele[i + total].y = 29;
            i++;
            ele[i].x = 15;
            ele[i].y = 22;
            ele[i + total].x = 15;
            ele[i + total].y = 29;
            break;

    }
    i++;
    ele[i * 2].x = 11; //bola
    ele[i * 2].y = 26;
}

void golo() {
    int x = ele[total * 2].x, y = ele[total * 2].y;
    if ((y <= 0 || y >= 50) && (x > 5 && x < 14)) {

        if (y == 0)
            resultados.res_eq2++;
        else 
            resultados.res_eq1++;
        
        montaCampo();
        int i;
        char str[80];
        serv_clie j;
        j.flag_campo = 1;
        j.xant = 99;
        j.xnovo = 99;
        j.yant = 99;
        j.ynovo = 99;
        posse_bola = NULL;
        
        montaCampo();
//        ele[total * 2].x = 11; //bola
//        ele[total * 2].y = 26;
        atualiza_campo(&j);
        sleep(1);

        for (i = 0; i < clientes.tam; i++) {
            clientes.c[i].jogador = NULL;
            
            sprintf(str, "/tmp/ccc%d", clientes.c[i].id);
            inicializacao_campo(str);
        }
    }
}

void passe(POSICAO orig, POSICAO * dest) {
    golo();

    POSICAO d;
    serv_clie j;

    JOGADOR *temp = posse_bola;

    posse_bola = NULL;
    while (orig.x != dest->x || orig.y != dest->y && posse_bola == NULL) {
        d.x = 0;
        d.y = 0;

        if (orig.x > dest->x)
            d.x--;
        else if (orig.x < dest->x)
            d.x++;
        if (orig.y > dest->y)
            d.y--;
        else if (orig.y < dest->y)
            d.y++;

        j.xant = ele[total * 2].x;
        j.yant = ele[total * 2].y;

        ele[total * 2].x += d.x;
        ele[total * 2].y += d.y;

        orig.x += d.x;
        orig.y += d.y;

        j.xnovo = ele[total * 2].x;
        j.ynovo = ele[total * 2].y;
        j.jogador = 'o';
        //printf("\nxant: %d yant: %d | xnovo: %d  ynovo: %d \n", j.xant, j.yant, j.xnovo, j.ynovo);
        atualiza_campo(&j);
        usleep(temp->tempo);


    }



}

void * bola(void * dados) {
    //int *FIM = (int *) dados;
    POSICAO d;
    serv_clie j;
    int i;
    int num;

    while (!resultados.fim) {
        golo();

        if (posse_bola != NULL) {
            if (posse_bola->equi == 'a') {
                j.jogador = 'o' + 1;

            } else {
                j.jogador = 'o' + 2;

            }
            //            j.jogador = 'o';

            j.xant = ele[total * 2].x;
            j.yant = ele[total * 2].y;
            
            if (posse_bola->equi == 'a') {
                ele[total * 2].y = posse_bola->posicao->y - 1;

            } else {
                ele[total * 2].y = posse_bola->posicao->y + 1;
            }
            ele[total * 2].x = posse_bola->posicao->x;
            j.ynovo = ele[total * 2].y;
            j.xnovo = ele[total * 2].x;
            
            //golo(j.xnovo, j.ynovo);
            atualiza_campo(&j);
        } else {
            JOGADOR *temp;

            for (i = 0; i < (total)*2; i++) {
                if (ele[i].x == ele[total * 2].x
                        && ele[i].y == ele[total * 2].y) {
                    if (i > total) {
                        temp = &JOG[1][i - total];
                    } else {
                        temp = &JOG[0][i];

                    }

                    posse_bola = temp;
                    break;
                }
            }
            //golo(ele[total * 2].x, ele[total * 2].y);

        }

        usleep(100);
    }
}

//int diferenca(int jog) {
//    if (jog > Ndefesa) 
//    {
//        if(jog<15)
//        {
//            if (Ndefesa == 4)
//            return 1;
//        else if (Ndefesa == 3)
//            return 2;
//        else if (Ndefesa == 2)
//            return 3;
//        else
//            return 4;
//        }
//        else
//        {
//            if (Ndefesa == 4)
//            return 2;
//        else if (Ndefesa == 3)
//            return 3;
//        else if (Ndefesa == 2)
//            return 4;
//        else
//            return 5;
//        }
//    }
//}

void * move_jogador(void * dados) {
    int i, fd;
    JOGADOR *jog;
    jog = (JOGADOR *) dados;
    POSICAO d;
    serv_clie j;
    j.flag_campo = 1;
    j.flag_logado = 0;

    int num = 0;

    if (jog->equi == 'b')
        num = total;

     while (!jog->fim)//TODO: Esta variavel nao faz sentido
     {
        if (!jog->humano) 
        {
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
            
            if (jog->posicao->y + d.y >= 0 && jog->posicao->y + d.y < MaxY &&
                    jog->posicao->x + d.x >= 0 && jog->posicao->x + d.x < MaxX) {
                int ye = 1;
                for (i = 0; i < (total)*2; i++) {
                    if (ele[i].x == jog->posicao->x + d.x &&
                            ele[i].y == jog->posicao->y + d.y) {
                        ye = 0;
                        break;
                    }

                }
                if (!ye) {
                    continue;
                }

                j.jogador = '0' + jog->num;
                if(jog->equi=='a')
                    j.equipa='a';
                else 
                    j.equipa='b';

                j.xant = jog->posicao->x;
                j.yant = jog->posicao->y;

                jog->posicao->y += d.y;
                jog->posicao->x += d.x;

                j.xnovo = jog->posicao->x;
                j.ynovo = jog->posicao->y;

                atualiza_campo(&j);

            //pthread_mutex_t_lock(&trinco);
            
            
            //if (posse_bola != NULL) {
            //    if (posse_bola->equi != jog->equi) {

            //        if (ele[jog->num].y < ele[total * 2].y) {
            //            d.y++;
            //        } else {
            //            d.y--;
            //        }

            //        if (ele[jog->num].x < ele[total * 2].x) {
            //            d.x++;
            //        } else {
            //            d.x--;
            //        }

            //    } else if (posse_bola->equi == jog->equi && jog != posse_bola) {

            //        if (jog->equi == 'a') {
            //            d.x++;
            //        } else {
            //            d.x--;

            //        }
            //    } else {
            //        int x = 0;
            //        if (posse_bola->equi == 'a') {
            //            int ye = 1;

            //            if (ele[jog->num].x <= 51 - 8 && ele[jog->num].y >= 11 - 8 && ele[jog->num].y <= 11 + 8) {
            //                //remata
            //            } else {
            //                for (i = 1; i < (total); i++) {
            //                    if (ele[i].x == ele[jog->num].x + 2 && ele[i].y == ele[jog->num].y) {
            //                        ye = 0;
            //                        break;
            //                    }
            //                }
            //                if (ye) {
            //                    d.x++;
            //                } else {
            //                    //passa bola

            //                }
            //            }


            //        } else {
            //            int ye = 1;

            //            if (ele[jog->num].x <= 8 && ele[jog->num].y >= 11 - 8 && ele[jog->num].y <= 11 + 8) {
            //                //remata
            //            } else {
            //                for (i = total + 1; i < (total)*2; i++) {
            //                    if (ele[i].x == ele[jog->num].x - 2 && ele[i].y == ele[jog->num].y) {
            //                        ye = 0;
            //                        break;
            //                    }
            //                }
            //                if (ye) {
            //                    d.x--;
            //                } else {
            //                    //passa bola

            //                }
            //            }

            //        }
            //    }
            //} else {
            //    if (ele[jog->num].y < ele[total * 2].y) {
            //        d.y++;
            //    } else {
            //        d.y--;
            //    }

            //    if (ele[jog->num].x < ele[total * 2].x) {
            //        d.x++;
            //    } else {
            //        d.x--;
            //    }
            //}

            //if (ele[jog->num].y == ele[total * 2].y && ele[jog->num].x == ele[total * 2].x) {//se esta na bola
            //    //                int num, num2;
            //    //                if(jog->equi=='a')
            //    //                    num=0;
            //    //                else
            //    //                    num=1;
            //    //                if(jog->num>=10)
            //    //                    num2=jog->num-10;
            //    //                else
            //    //                    num2=jog->num;
            //    //                posse_bola = &JOG[num][num2];
            //    posse_bola = jog;


            //}
            
            }
            //pthread_mutex_t_unlock(&trinco);
        }
        usleep(jog->tempo);


    }
    pthread_exit(0);

}

void * move_redes(void * dados) {
    JOGADOR *jog;
    jog = (JOGADOR *) dados;
    POSICAO d;
    serv_clie j;
    //printf("\nestou no move redes \n");

    while (!resultados.fim)
    {
        if (!jog->humano) 
        {
            //pthread_mutex_t_lock(&trinco);
            d.x = 0;
            int r = rand() % 2;
            
            if(r==0)
            {
                d.x=1;
            }
            else
            {
                d.x=-1;
            }
            if(jog->posicao->x + d.x >= limInfXRedes && jog->posicao->x + d.x <= limSupXRedes)
            {
                j.xant = jog->posicao->x;
                j.yant = jog->posicao->y;

                jog->posicao->y += d.y;
                jog->posicao->x += d.x;

                j.xnovo = jog->posicao->x;
                j.ynovo = jog->posicao->y;

                j.jogador = '0' + jog->num ; 
                
                if(jog->equi=='a')
                    j.equipa='a';
                else 
                    j.equipa='b';
            }    
            //pthread_mutex_t_unlock(&trinco);
            atualiza_campo(&j);
            
            /*
            if (ele[jog->num].y < ele[0].y) 
            {
                d.y++;
            } 
            else 
            {
                d.y--;
            }

            if (ele[jog->num].x < ele[0].x) 
            {
                d.x++;
            } else 
            {
                d.x--;
            }
            int area_min = 0, area_max = 7;
            //if (jog->num == 1)printf(" %d\n", ele[jog->num].x);
            if (jog->num == 2) 
            {
                area_min = 44;
                area_max = 51;
            }*/
        }
        usleep(jog->tempo);
    } 
    pthread_exit(0);
}

JOGADOR * procuraJogador(char equi, int op){
    int i, eq;
    if (equi == 'a')
        eq = 0;
    else if(equi == 'b')
        eq = 1;
    else 
        return NULL;
    
    for (i = 0; i< total; i++){
        if (JOG[eq][i].num == op)
            return &JOG[eq][i];
    }
    
    
    return NULL;
}

void interpreta_comando(int cam, CLIENTES * cli, clie_serv * novo) {
    int i, a, flag = 0;
    POSICAO temp;
    JOGADOR * aux;

    for (i = 0; i < cli->tam; i++) 
    {
        if (novo->id == cli->c[i].id) 
        {
            //printf("\nENcontreime no meio dos clientes\n");
            if (cli->c[i].equi != '-') 
            {              
                if(cli->c[i].jogador==NULL)
                {
                    aux=procuraJogador(cli->c[i].equi, cam);

                    for(a=0;a<cli->tam;a++)
                    {
                        if(cli->c[a].jogador==aux)
                        {
                            //o jogador pretendido ja e controlado 
                            flag=1;
                            break;
                        }                                             
                    }
                    if(flag==0)
                    {
                        cli->c[i].jogador=procuraJogador(cli->c[i].equi,cam);
                        cli->c[i].jogador->humano=1;
                    }
                }
                else if(cli->c[i].jogador->num==cam)
                {
                    cli->c[i].jogador->humano = 0;
                    cli->c[i].jogador=NULL;
                }
                else
                {
                    //passa para o cam
                    if (cli->c[i].jogador == posse_bola) 
                    {
                        if ((rand() % 100) < cli->c[i].jogador->precisao_remate) 
                        {
                            passe(ele[total * 2], cli->c[i].jogador->posicao);
                            //printf("\nVou para o sitio certo\n");
                        } 
                        else 
                        {
                            //printf("\nVou para o sitio aleatorio\n");
                            int aux = rand() % 4;
                            if (aux == 0)
                            {
                                temp.x = 0;
                                temp.y = rand() % (MaxY - 1);
                            } 
                            else
                                if (aux == 1) 
                                {
                                temp.y = 0;
                                temp.x = rand() % (MaxX - 1);
                                } 
                                else if (aux == 2) {
                                temp.y = MaxY - 1;
                                temp.x = rand() % (MaxX - 1);
                            } else {
                                temp.x = MaxX - 1;
                                temp.y = rand() % (MaxY - 1);
                            }
                            //printf("\nVou para ali %d %d \n", temp.x, temp.y);
                            passe(ele[total * 2], &temp);
                        }
                    }
                }     
            } 
            break;
        }
    }
}

void controlaJogador(int op, CLIENTE * cliente){
    int xSum=0, ySum=0, i, maxXJog=MaxX, minXJog=0;
    serv_clie j;
    
    switch (op){
        case 'u':
            xSum=-1;
            break;
        case 'd':
            xSum=1;
            break;
        case 'l':
            ySum=-1;
            break;
        case 'r':
            ySum=1;
            break;
    }
    
    if (cliente->jogador == NULL)
        return;
    
    if(cliente->jogador->num==0)
    {
        ySum=0;
        maxXJog=limSupXRedes;
        minXJog=limInfXRedes;
    }
    
    j.xant = cliente->jogador->posicao->x;
    j.yant = cliente->jogador->posicao->y;
    
    xSum += cliente->jogador->posicao->x;
    ySum += cliente->jogador->posicao->y;
    
    if (xSum < minXJog|| xSum > maxXJog || ySum < 0 || ySum > MaxY)
        return;
    
    for (i = 0; i < (total * 2) + 2; i++){
  
        if (ele[i].x == xSum && ele[i].y == ySum)
            break;
    }
    
    if (i==(total * 2))
        posse_bola = cliente->jogador;
    else if (i != (total * 2) + 2)
        return;
    
    cliente->jogador->posicao->x = xSum;
    cliente->jogador->posicao->y = ySum;
    
    j.jogador = '0' + cliente->jogador->num;   
    j.equipa=cliente->jogador->equi;

    j.xnovo = cliente->jogador->posicao->x;
    j.ynovo = cliente->jogador->posicao->y;

    atualiza_campo(&j);
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
                if (cli->c[i].id == cliente->id) 
                {
                    if (cli->c[i].equi == '-') 
                    {
                        //printf("\nESCOLHEU EQUIPA A\n");
                        cli->c[i].equi = 'a';
                    } 
                    break;
                }
            }
            break;
        case 'b':
            for (i = 0; i < cli->tam; i++) 
            {
                if (cli->c[i].id == cliente->id) 
                {
                    //printf("\nESCOLHEU EQUIPA B\n");
                    if (cli->c[i].equi == '-') 
                    {
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
                                    passe(ele[total * 2 ], &baliza);
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
                                    //printf("\nVou para ali %d %d \n", temp.x, temp.y);
                                    passe(ele[total * 2 ], &baliza);
                                }
                                //passe(ele[total * 2], &baliza);
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

            for (i = 0; i < cli->tam; i++) 
            {
                if (cli->c[i].id == cliente->id) 
                {
                    break;
                }
            }
            //printf("\ncontrolas este %d\n", cli->c[i].jog);
            if (cli->c[i].jogador == NULL || cli->c[i].equi == '-') 
            {
                //printf("\nNao controlas ninguem, vou sair\n");
                break;
            } 
            else 
            {
                controlaJogador(cliente->op, &cli->c[i]);
                
                
                
                
                
//                
//                
//                int eq, ez;
//                if (cli->c[i].equi == 'a') 
//                {
//                    eq = 0;
//                    ez = 0;
//                } else if (cli->c[i].equi == 'b') 
//                {
//                    eq = total;
//                    ez = 1;
//                }
//
//                int p, menos = 0;
//                int flag = 0;
//
//                if (cli->c[i].jog == 0) 
//                {
//                    if (ele[eq + cli->c[i].jog].x - 1 < 3) 
//                    {
//                        break;
//                    }
//                }
//
//                //printf("\nVais aos elementos pos %d", eq * cli->c[i].jog - menos);
//                if (ele[eq + cli->c[i].jog].x - 1 >= 0) 
//                {
//                    for (p = 0; p < (total * 2) + 2; p++) 
//                    {
//                        if (ele[p].x == ele[eq + cli->c[i].jog].x - 1 &&
//                                ele[p].y == ele[eq + cli->c[i].jog].y) 
//                        {
//                            flag = 1;
//                            if ((total * 2) == p) 
//                                flag = 2;
//
//                            break;
//                        }
//                    }
//
//
//                    if (!flag || flag == 2) 
//                    {
//                        j.jogador = '0' + JOG[ez][cli->c[i].jog ].num;                  
//
//                        j.xant = ele[eq + cli->c[i].jog].x;
//                        j.yant = ele[eq + cli->c[i].jog].y;
//
//                        ele[eq + cli->c[i].jog].x -= 1;
//                        
//                        j.xnovo = ele[eq + cli->c[i].jog].x;
//                        j.ynovo = ele[eq + cli->c[i].jog].y;
//                      
//                        atualiza_campo(&j);
//                        usleep(JOG[ez][cli->c[i].jog - diferenca(cli->c[i].jog)].tempo);
//                    }
//                    if (flag == 2) {
//                        int equipa = cli->c[i].equi - 97;
//                        int nurm = cli->c[i].jog - diferenca(cli->c[i].jog);
//
//                        posse_bola = &JOG[equipa][nurm];
//                        //printf("olaola este [%d][%d] tem a posse \n", equipa, nurm);
//                    }
//
//                    //
//                }
//
//            }
            }
            break;

        case 'd':

            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente->id) {
                    break;
                }
            }
            if (cli->c[i].jogador == NULL || cli->c[i].equi == '-') {
                //printf("\nNao controlas ninguem, vou sair\n");
                break;
            } else {
                controlaJogador(cliente->op, &cli->c[i]);
//                int eq, ez;
//                if (cli->c[i].equi == 'a') {
//                    eq = 0;
//                    ez = 0;
//                } else if (cli->c[i].equi == 'b') {
//                    eq = total;
//                    ez = 1;
//                }
//
//                int p, menos = 0;
//                int flag = 0;
//
//                if (cli->c[i].jog == 0) {
//                    if (ele[eq + cli->c[i].jog].x + 1 > 18) {
//                        break;
//                    }
//                }
//
//                if (ele[eq + cli->c[i].jog].x + 1 < MaxX) {
//                    for (p = 0; p < (total * 2) + 2; p++) {
//                        if (ele[p].x == ele[eq + cli->c[i].jog ].x + 1 &&
//                                ele[p].y == ele[eq + cli->c[i].jog ].y) {
//                            flag = 1;
//                            if ((total * 2) == p) 
//                                flag = 2;
//                            break;
//                        }
//                    }
//
//                    //printf("\nflag: %d\n", flag);
//                    if (!flag || flag == 2) {
//                        //                        if (JOG[ez][cli->c[i].jog - diferenca(cli->c[i].jog)].num < 10)
//                        j.jogador = '0' + JOG[ez][cli->c[i].jog ].num;
//                        //                        else
//                        //                            j.jogador = '0' + JOG[ez][cli->c[i].jog - diferenca(cli->c[i].jog)].num - 10;
//
//                        j.xant = ele[eq + cli->c[i].jog].x;
//                        j.yant = ele[eq + cli->c[i].jog].y;
//
//                        ele[eq + cli->c[i].jog].x += 1;
//
//                        j.xnovo = ele[eq + cli->c[i].jog].x;
//                        j.ynovo = ele[eq + cli->c[i].jog].y;
//                        //                mvaddch(ele[jog->num].x, ele[jog->num].y, '0' + ((jog->num) / 2) + 1.5);
//                        /* printf("\njog: %d  equi: %c  xant: %d yant: %d | xnovo: %d  ynovo: %d \n",
//                                 cli->c[i].jog, cli->c[i].equi, j.xant, j.yant, j.xnovo, j.ynovo);*/
//                        atualiza_campo(&j);
//                        usleep(JOG[ez][cli->c[i].jog - diferenca(cli->c[i].jog)].tempo);
//                    }
//                    if (flag == 2) {
//                        int equipa = cli->c[i].equi - 97;
//                        int num = cli->c[i].jog - diferenca(cli->c[i].jog);
//
//                        posse_bola = &JOG[equipa][num];
//                        //                        printf("olaola este %d tem a posse \n", cli->c[i].jog);
//                    }
//
//                    //
//                }
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
//                int eq, ez;
//                if (cli->c[i].equi == 'a') {
//                    eq = 0;
//                    ez = 0;
//                } else if (cli->c[i].equi == 'b') {
//                    eq = total;
//                    ez = 1;
//                }
//
//                int p, menos = 0;
//                int flag = 0;
//
//                if (cli->c[i].jog == 0) {
//                    break;
//                }
//                if (ele[eq + cli->c[i].jog].y - 1 >= 0) {
//                    for (p = 0; p < (total * 2) + 2; p++) {
//                        if (ele[p].x == ele[eq + cli->c[i].jog ].x &&
//                                ele[p].y == ele[eq + cli->c[i].jog ].y - 1) {
//                            //printf("\nPosicao ocupada %d\n", p);
//                            flag = 1;
//                            if ((total * 2) == p) flag = 2;
//
//                            break;
//                        }
//                    }
//                    if (!flag || flag == 2) {
//                        //                        if (JOG[ez][cli->c[i].jog - diferenca(cli->c[i].jog)].num < 10)
//                        j.jogador = '0' + JOG[ez][cli->c[i].jog].num;
//                        //                        else
//                        //                            j.jogador = '0' + JOG[ez][cli->c[i].jog - diferenca(cli->c[i].jog)].num - 10;
//
//                        j.xant = ele[eq + cli->c[i].jog].x;
//                        j.yant = ele[eq + cli->c[i].jog].y;
//
//                        ele[eq + cli->c[i].jog].y -= 1;
//
//                        j.xnovo = ele[eq + cli->c[i].jog].x;
//                        j.ynovo = ele[eq + cli->c[i].jog].y;
//                        //                mvaddch(ele[jog->num].x, ele[jog->num].y, '0' + ((jog->num) / 2) + 1.5);
//                        //                        printf("\njog: %d  equi: %c  xant: %d yant: %d | xnovo: %d  ynovo: %d \n",
//                        //                                cli->c[i].jog, cli->c[i].equi, j.xant, j.yant, j.xnovo, j.ynovo);
//                        atualiza_campo(&j);
//                        usleep(JOG[ez][cli->c[i].jog - diferenca(cli->c[i].jog)].tempo);
//                    }
//                    if (flag == 2) {
//                        int equipa = cli->c[i].equi - 97;
//                        int num = cli->c[i].jog - diferenca(cli->c[i].jog);
//                        //printf("\nNumero jogo %d", num);
//                        posse_bola = &JOG[equipa][num];
//                        //                        printf("olaola este %d tem a posse \n", cli->c[i].jog);
//                    }
//                    //
//                }
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
//                int eq, ez, menos = 0;
//                if (cli->c[i].equi == 'a') {
//                    eq = 0;
//                    ez = 0;
//                } else if (cli->c[i].equi == 'b') {
//                    eq = total;
//                    ez = 1;
//                    //menos++;
//                }
//
//                int p;
//                int flag = 0;
//
//                if (cli->c[i].jog == 0) {
//                    break;
//                }
//
//                if (ele[eq + cli->c[i].jog].y + 1 < MaxY) {
//                    for (p = 0; p < (total * 2) + 2; p++) {
//                        if (ele[p].x == ele[eq + cli->c[i].jog].x &&
//                                ele[p].y == ele[eq + cli->c[i].jog].y + 1) {
//                            flag = 1;
//                            //                            printf("\nP: %d\n", p);
//                            if ((total * 2) == p) flag = 2;
//                            break;
//                        }
//
//                    }
//                    if (!flag || flag == 2) {
//                        j.jogador = '0' + JOG[ez][cli->c[i].jog].num;
//
//                        j.xant = ele[eq + cli->c[i].jog].x;
//                        j.yant = ele[eq + cli->c[i].jog].y;
//
//                        ele[eq + cli->c[i].jog].y += 1;
//
//                        j.xnovo = ele[eq + cli->c[i].jog].x;
//                        j.ynovo = ele[eq + cli->c[i].jog].y;
//                        //                mvaddch(ele[jog->num].x, ele[jog->num].y, '0' + ((jog->num) / 2) + 1.5);
//                        //                        printf("\njog: %d  equi: %c  xant: %d yant: %d | xnovo: %d  ynovo: %d \n",
//                        //                                cli->c[i].jog, cli->c[i].equi, j.xant, j.yant, j.xnovo, j.ynovo);
//                        atualiza_campo(&j);
//                        usleep(JOG[ez][cli->c[i].jog - diferenca(cli->c[i].jog)].tempo);
//                    }
//                    if (flag == 2) {
//                        int equipa = cli->c[i].equi - 97;
//                        int num = cli->c[i].jog - diferenca(cli->c[i].jog);
//
//                        posse_bola = &JOG[equipa][num];
//                        //                        printf("olaola este %d tem a posse \n", cli->c[i].jog);
//                    }
//                    //
//                }
            }
            break;
            //ultimo case
        default:
            break;
    }
}

void * Func_receber_cliente(void * dados) {

    CLIENTES * cli = (CLIENTES *) dados;
    clie_serv cliente;
    char str[80], cmd[80];
    int fd, fd_resp, i, r, rat;

    serv_clie j;
    FILE *f;
    fd_set rfds;
    struct timeval tv;
    j.flag_logado = 0;
    j.flag_campo = 0;

    char temp_user[80], temp_pass[80];
    int a;
    while (!resultados.fim) {

        fd = open(FIFO, O_RDONLY);

        a = read(fd, &cliente, sizeof (clie_serv));

        if (a != sizeof (clie_serv)) {
            printf("\n{SERVIDOR} Nao foi possivel receber um pedido de um cliente com o size pretendido\n");
            close(fd);
            continue;
        }
        close(fd);

        if (cliente.flag_con) {
            if (cli->tam < 19) {
                if (cli->tam == 0)
                    cli->c = (CLIENTE *) malloc(sizeof (CLIENTE));
                else
                    cli->c = (CLIENTE *) realloc(cli->c, sizeof (CLIENTE)*(cli->tam + 1));

            } else {
                printf("Mais que 19 clientes!\n");
                pthread_exit(0);
            }

            cli->c[cli->tam].id = cliente.id;
            cli->c[cli->tam].jogador = NULL;
            cli->c[cli->tam].equi = '-';
            cli->c[cli->tam].logado = 0;

            cli->tam++;

        } else if (cliente.flag_log) {//logar
            FILE * f = fopen(cli->nome_ficheiro, "rt");
            if (!f) {
                printf("Nao encontra ficheiro!!\n");
                pthread_exit(0);
            }


            //            while (1) {
            //                if (i != sizeof (clie_serv)) {
            //                    printf("\nERRO AO RECEBER LOGIN!!\n");
            //                    close(fd);
            //                    continue;
            //                } else {
            int aux, t;
            j.flag_logado = 0;
            j.flag_campo = 0;


            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente.id) {
                    while (fscanf(f, " %s %s", temp_user, temp_pass) == 2) {
                        if (strcmp(temp_user, cliente.user) == 0 && strcmp(temp_pass, cliente.pass) == 0) {
                            for (t = 0; t < cli->tam; t++) {
                                if (strcmp(cliente.user, cli->c[t].username) == 0) {

                                    //printf("\nUSERNAME: %s \n", cli->c[t].username);
                                    j.flag_logado = 0;
                                    aux = 1;
                                }
                            }
                            if (aux == 0) {
                                cli->c[i].logado = 1;
                                j.flag_logado = 1;
                                //printf("cliente %d logado %s\n", cliente.id, cliente.user);
                                strcpy(cli->c[i].username, temp_user);
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            //printf("\nVou enviar resposta: fl: %d, gc: %d\n", j.flag_logado, j.flag_campo);
            sprintf(str, "/tmp/ccc%d", cliente.id);
            if (access(str, F_OK) != 0) {
                printf("cliente off\n");
                //                return 3;
            }
            fd_resp = open(str, O_WRONLY);
            //            sleep(1);
            //printf("\nstr: %s \n", str);

            write(fd_resp, &j, sizeof (serv_clie));
            sleep(1);
            inicializacao_campo(str);

            close(fd_resp);

            fclose(f);
        } else if (cliente.flag_desliga) {
            //            printf("\ndesligado: olaoal\n");
            for (i = 0; i < cli->tam; i++) {
                if (cli->c[i].id == cliente.id) {
                    cli->c[i].logado = 0;
                    strcpy(cli->c[i].username, " ");
                    //printf("\ndesligado: %d \n", cli->c[i].id);

                }
            }
        } else if (cliente.flag_operacao) {
            operacao(&cliente, cli);

            //                fd = open(FIFO, O_RDONLY);
            //                if (read(fd, &cliente, sizeof (clie_serv)) != sizeof (clie_serv)) {
            //                    printf("\nERRO AO CONETAR NOVO CLIENTE!! \n");
            //                    close(fd);
            //                    continue;
            //                }
            //
            //                fclose(fd);


            //switch
            //if



        }//else

    }//while
    //close(fd);
}

//TODO: cli->c[i]->logado SE JA ESTIVER LOGADO O QUE FAZER....

void * func_jogo(void * dados) {

    //    const char* AVndefesa = getenv("NDEFESAS");
    //    const char* AVnavanc = getenv("NAVANCADOS");
    //
    //    Ndefesa = (int) AVndefesa;
    //    Navanc = (int) AVnavanc;
    int * FIM = (int *) dados;
    int i;

    Ndefesa = 4;
    Navanc = 4;


    clientes.tam = 0;

    total = (1 + Navanc + Ndefesa);

    ele = (POSICAO*) malloc(sizeof (POSICAO)*(total * 2 + 2));

    inicializaJog();
    montaCampo();

    pthread_t receber_cliente;

    pthread_create(&receber_cliente, NULL, &Func_receber_cliente, (void *) &clientes);

    pthread_t tarefa[2][total];
    pthread_t tarefa_bola;

    pthread_create(&tarefa_bola, NULL, &bola, FIM);
    pthread_create(&tarefa[0][0], NULL, &move_redes, (void *) &JOG[0][0]);
    pthread_create(&tarefa[1][0], NULL, &move_redes, (void *) &JOG[1][0]);


    for (i = 1; i < total; i++) {
        pthread_create(&tarefa[0][i], NULL, &move_jogador, (void *) &JOG[0][i]);
        pthread_create(&tarefa[1][i], NULL, &move_jogador, (void *) &JOG[1][i]);
    }
    

    while (!resultados.fim) {

    }

    for (i = 0; i < total; i++) {
        pthread_join(tarefa[0][i], NULL);
        pthread_join(tarefa[1][i], NULL);
    }
    pthread_join(tarefa[0][0], NULL);
    pthread_join(tarefa[0][0], NULL);
    pthread_join(tarefa_bola, NULL);

    pthread_join(receber_cliente, NULL);
    /*for (i = 0; i < total; i++) {
            free(&JOG[0][i]);
            free(&JOG[1][i]);
    }
    for (i = 0; i < clientes.tam; i++) {
            free(&clientes.c[i]);
    }

free(&JOG[0]);
free(&JOG[1]);
free(ele);*/

}

void * contar_seg(void * dados) {
    int * tempo = (int *) dados;
    serv_clie j;
    j.flag_campo = 1;
    j.xant = 99;
    j.xnovo = 99;
    j.yant = 99;
    j.ynovo = 99;

    while (tempo > 0) {
        resultados.tempo--;
        j.resultados = resultados;

        atualiza_campo(&j);
        sleep(1);
        tempo--;
    }
    resultados.fim = 1;
}

void acabar_jogo(int s) {
    //if (s == SIGINT) {
    flag_fimJogo = 1;
    resultados.fim = 1;
    unlink(FIFO);

    //pthread_join(tempo, NULL);
    //}
    if (s == SIGINT) {
        int i;
        for (i = 0; i < clientes.tam; i++) {
            kill(clientes.c[i].id, SIGUSR1);
        }
        sair = 1;

        exit(3);

    }
}

int main(int argc, char** argv) {//TODO:

    if (argc != 2) {
        printf("SINTAXE: %s {nome_ficheiro}\n", argv[0]);
        return 1;
    }

    strcpy(clientes.nome_ficheiro, argv[1]);


    srand((unsigned int) time(NULL));
    signal(SIGINT, SIG_IGN);

    signal(SIGINT, acabar_jogo);
    signal(SIGALRM, acabar_jogo);
    char *primeiro;
    char *segundo;
    FILE *f;


    int i, cont = 0;
    char tecla, cmd[80];
    
    /*if (access(FIFO, F_OK) == 0) {
        printf("Ja esta um servidor em execução\n");
        return 3;
    }*///TODO: Descomentar isto

    int z = mkfifo(FIFO, 0600);

    do {
        printf("\nComando: ");

        scanf(" %[^\n]", cmd);
        //printf("\ncomando: %s\n", cmd);

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
                    printf("\nErro de sintaxe start {n}\n");
                    break;

                }
                if (flag_fimJogo == 1) {
                    flag_fimJogo = 0;

                    resultados.tempo = atoi(primeiro);

                    //alarm(res.tempo);
                    resultados.res_eq1 = 0;
                    resultados.res_eq2 = 0;
                    resultados.fim = 0;


                    pthread_create(&jogo, NULL, &func_jogo, &flag_fimJogo);
                    // pthread_create(&tempo, NULL, &contar_seg, &res.tempo);
                    sleep(1);
                } else {
                    printf("\nEsta a decorrer um jogo!\n");
                }
                break;

            case 1://STOP
                flag_fimJogo = 1;
                pthread_join(jogo, NULL);
                //                pthread_join(tempo, NULL);
                break;

            case 2://USER
                primeiro = strtok(NULL, " ");

                f = fopen(clientes.nome_ficheiro, "rt");

                if (!f) {
                    printf("erro ao abrir o ficheiro de utilizadores");
                    break;
                }
                char user[80];
                char pass[80];

                while (fscanf(f, "%s %s", user, pass) == 2) {//TODO: TESTAR (%[^n] no ultimo %s)
                    if (strcmp(user, primeiro) == 0) {

                        printf("\nUtilizador ja existe\n");
                        fclose(f);
                        break;
                    }
                }
                fclose(f);

                f = fopen(clientes.nome_ficheiro, "at");

                if (!f) {
                    printf("erro ao abrir o ficheiro de utilizadores");
                    break;
                }

                segundo = strtok(NULL, " ");
                fprintf(f, "\n%s %s", primeiro, segundo);

                fclose(f);

                break;
            case 3://USERS
                printf("\nClientes Logados: \n");
                for (i = 0; i < clientes.tam; i++) {
                    if (clientes.c[i].logado == 1) {
                        printf("Username: %s  Pid: %d", clientes.c[i].username, clientes.c[i].id);
                    }
                }
                break;
            case 4://RESULT
                printf("\nEquipa a: %d - Equipa b: %d\n", resultados.res_eq1, resultados.res_eq2); //mostrar o resultado ao admin
                break;
            case 5://RED
                segundo = strtok(NULL, " ");
                for (i = 0; i < clientes.tam; i++) {
                    if (strcmp(clientes.c[i].username, segundo) == 0) {

                        if (clientes.c[i].equi != '-') {
                            clientes.c[i].jogador->humano = 0;
                            clientes.c[i].equi = '-';

                        }
                        clientes.c[i].logado = 0;
                        clientes.c[i].jogador = NULL;
                        strcpy(clientes.c[i].username, "");
                        kill(clientes.c[i].id, SIGUSR1);
                    }
                }

            case 6://shutdown
                for (i = 0; i < clientes.tam; i++) {
                    kill(clientes.c[i].id, SIGUSR1);
                }
                sair = 1;
                break;
            default:
                printf("\nComando Invalido!\n");
                break;

        }

    } while (!sair);
    free(ele);
    free(JOG);
    flag_fimJogo = 1;
    pthread_join(jogo, NULL);
    pthread_join(tempo, NULL);

}

