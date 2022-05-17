#ifndef MAIN_HPP
#define MAIN_HPP

#include <mpi.h>
#include <stdio.h>
#include <cstdlib>
#include <stddef.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

// #define DEBUG

/* boolean */
#define TRUE 1
#define FALSE 0

/* używane w wątku głównym, determinuje jak często i na jak długo zmieniają się stany */
#define STATE_CHANGE_PROB 50
#define SEC_IN_STATE 2

// GLOBALS
#define SHIP_COUNT 8 // Okrety
#define DOCKS_COUNT 5 // Doki
#define MECHANICS_COUNT 10 // Mechanicy

// typy wiadomości
#define REQ1 1 // żądanie dostępu do zasobu doków
#define ACK1 2 // potwierdzenie, że wiemy o cudzym żądaniu dostępu do doku
#define REQ2 3 // żądanie dostępu do zasobu mechaników (w polu count_mech informacja o ilu się ubiegamy)
#define ACK2 4 // potwierdzenie, że wiemy o cudzym żądaniu dostępu do mechaników
#define RELEASE 5 // zwolnienie obu zasobów przez proces
#define FINISH 6 // koniec programu

typedef enum {InRun, BeforeDockWait, InDockWait, BeforeMechWait, InMechWait, InSend, InFinish} state_t;
extern state_t stan;
extern int rank;
extern int size;
extern int mechanics; // Ilość dostępnych mechaników
extern int docks; // Ilość dostępnych doków
extern int lamportTime;

extern std::vector<std::tuple<int, int>> dock_tab; // tablica na kolejkę dostępu do doków
extern std::vector<std::tuple<int, int, int>> mech_tab; // tablica na kolejkę dostępu do mechaników

typedef struct {
    int ts;         // timestamp zegaru Lamporta
    int src;        // nadawca
    int mech_count; // Ilość żądanych mechaników (przy dokach pole ustawione na 1)
} packet_t;
extern MPI_Datatype MPI_PAKIET_T;
extern pthread_mutex_t vecDockMut;
extern pthread_mutex_t vecMechMut;


#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define println(FORMAT, ...) printf("%c[%d;%dm [%d] [ts: %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamportTime, ##__VA_ARGS__, 27,0,37);

void check_thread_support(int provided);
void init(int *argc, char ***argv);
void finalize();
void sendPacket(packet_t *pkt, int destination, int tag);
void changeState( state_t );
void changeTime(int new_time);
int max(int a, int b);
 
#endif