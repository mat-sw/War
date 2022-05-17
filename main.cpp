#include "main.hpp"
#include "main_thread.hpp"
#include "com_thread.hpp"

pthread_mutex_t lamportMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vecDockMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vecMechMut = PTHREAD_MUTEX_INITIALIZER;

std::vector<std::tuple<int, int>> dock_tab; // tablica na kolejkę dostępu do doków
std::vector<std::tuple<int, int, int>> mech_tab; // tablica na kolejkę dostępu do mechaników

int lamportTime;
int size, rank, mechanics, docks;
state_t state = BeforeDockWait;
MPI_Datatype MPI_PAKIET_T;
pthread_t threadCom;

void check_thread_support(int provided) {
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE:
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
            fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
            MPI_Finalize();
            exit(-1);
            break;
        case MPI_THREAD_FUNNELED:
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	        break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	        break;
        case MPI_THREAD_MULTIPLE: 
            printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	        break;
        default: printf("Nikt nic nie wie\n");
    }
}

void init(int *argc, char ***argv) {
    int provided;
    MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);

    const int    nitems = 3; /* bo packet_t ma trzy pola */
    int          blocklengths[3] = {1,1,1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[3];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, mech_count);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);
    pthread_create( &threadCom, NULL, startComThread, 0 );
}

void finalize() {
    pthread_mutex_destroy( &stateMut );
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n");
    pthread_join(threadCom, NULL);
    MPI_Type_free( &MPI_PAKIET_T );
    MPI_Finalize();
}

void sendPacket(packet_t *pkt, int destination, int tag) {
    bool freepkt = false;
    if (pkt == 0) {
        pkt = new packet_t;
        freepkt = true;
    }
    pkt->src = rank;
    // changeTime(lamportTime + 1);
    pkt->ts = lamportTime;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD );
    if (freepkt) delete pkt;
}

void changeState( state_t newState ) {
    pthread_mutex_lock( &stateMut );
    if (state == InFinish) { 
	    pthread_mutex_unlock( &stateMut );
        return;
    }
    state = newState;
    pthread_mutex_unlock( &stateMut );
}

void changeTime(int new_time) {
    pthread_mutex_lock( &lamportMut );
    
    lamportTime = new_time;

    pthread_mutex_unlock( &lamportMut );
}

int max(int a, int b) {
    if (a >= b) return a;
    return b;
}

int main(int argc, char ** argv) {
    init(&argc, &argv);
    mechanics = MECHANICS_COUNT;
    docks = DOCKS_COUNT;
    std::cout << "Hello world: "<< rank <<" of "<< size <<std::endl;
    mainLoop();
    finalize();
    return 0;
}