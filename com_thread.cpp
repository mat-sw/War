#include "main.hpp"
#include "com_thread.hpp"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startComThread(void *ptr) {
    
    int ack1_count = 0, ack2_count = 0;
    MPI_Status status;
    packet_t pkt;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan != InFinish ) {
        MPI_Recv( &pkt, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
        changeTime(max(lamportTime, pkt.ts) + 1);
	    println("Dostałem request!\n");
        switch ( status.MPI_TAG ) {
            case FINISH: // Ktoś chce zakończyć wojnę
                println("Kończymy wojnę!\n")
                changeState(InFinish);
                break;
            case REQ1: // Odbieram żądanie doków innego statku
                // println("Wysyłam potwierdzenie ACK1 do %d\n", pkt.src);
                pthread_mutex_lock( &vecDockMut );

                dock_tab.push_back(std::make_tuple(pkt.ts, pkt.src));
                std::sort(dock_tab.begin(), dock_tab.end());
                changeTime( lamportTime + 1 );
                MPI_Send( &pkt, 1, MPI_PAKIET_T, pkt.src, ACK1, MPI_COMM_WORLD );

                pthread_mutex_unlock( &vecDockMut );
                break;
            case REQ2: // Odbieram żądanie mechaników innego statku
                // println("Wysyłam potwierdzenie ACK2 do %d\n", pkt.src);
                pthread_mutex_lock( &vecMechMut );
                mech_tab.push_back(std::make_tuple(pkt.ts, pkt.src, pkt.mech_count));
                std::sort(mech_tab.begin(), mech_tab.end());
                pthread_mutex_unlock( &vecMechMut );
                changeTime( lamportTime + 1 );
                MPI_Send( &pkt, 1, MPI_PAKIET_T, pkt.src, ACK2, MPI_COMM_WORLD );
                break;
            case ACK1: // Odbieram potwierdzenie dostępu do doku od innego statku
                // println("Odbieram potwierdzenie dostępu do doków od %d\n", pkt.src);
                ack1_count++;
                if (ack1_count == size - 1) {
                    changeState( InDockWait );
                }
                break;
            case ACK2: // Odbieram potwierdzenie dostępu do mechaników od innego statku
                // println("Odbieram potwierdzenie dostępu do mechaników od %d\n", pkt.src);
                ack2_count++;
                if (ack2_count == size - 1) {
                    changeState( InMechWait );
                }
                break;
            case RELEASE: // Inny statek opuścił dok

                break;
            default:

                break;
        }
    }
}
