
/* 
 * File:   estrutura.h
 *
 * Created on 4 de Janeiro de 2017, 17:16
 */

#define FIFO "/tmp/fifo"
#define limSupXRedes 18
#define limInfXRedes 4
#define MaxX 21
#define MaxY 51

typedef struct {
    int res_eq1;
    int res_eq2;
    int fim;
    int tempo;
} RESULTADOS;

typedef struct {
    int op;
    int id;
    char user[80];
    char pass[80];
    int flag_con;
    int flag_log;
    int flag_desliga;
    int flag_operacao;
} clie_serv; //CLIENTE PARA SERVER

typedef struct {
    int xnovo;
    int ynovo;
    int xant;
    int yant;
    char jogador;
    char equipa;
    int flag_logado;
    int flag_campo;
    RESULTADOS resultados;
} serv_clie; //SERVER PARA CLIENTE

//typedef struct { char user[80], pass[80]; int pid; } LOGIN;




