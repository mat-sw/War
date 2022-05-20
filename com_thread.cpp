#include "main.hpp"
#include "com_thread.hpp"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startComThread(void *ptr) {
    
    int ack1_count = 0, ack2_count = 0;
    MPI_Status status;
    packet_t pkt;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( state != InFinish ) {
        MPI_Recv( &pkt, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
        changeTime(max(lamportTime, pkt.ts) + 1);
	    println("Dostałem wiadomość od %d o tagu %d", pkt.src, status.MPI_TAG);
        switch ( status.MPI_TAG ) {
            case FINISH: // Ktoś chce zakończyć wojnę
                println("Kończymy wojnę!\n")
                changeState(InFinish);
                break;
            case REQ1: // Odbieram żądanie doków innego statku
                pthread_mutex_lock( &vecDockMut );

                dock_tab.push_back(std::make_tuple(pkt.ts, pkt.src));
                std::sort(dock_tab.begin(), dock_tab.end());

                pthread_mutex_unlock( &vecDockMut );

                changeTime( lamportTime + 1 );
                sendPacket(&pkt, pkt.src, ACK1);
                break;
            case REQ2: // Odbieram żądanie mechaników innego statku
                pthread_mutex_lock( &vecMechMut );

                mech_tab.push_back(std::make_tuple(pkt.ts, pkt.src, pkt.mech_count));
                std::sort(mech_tab.begin(), mech_tab.end());

                pthread_mutex_unlock( &vecMechMut );

                changeTime( lamportTime + 1 );
                sendPacket(&pkt, pkt.src, ACK2);
                break;
            case ACK1: // Odbieram potwierdzenie dostępu do doku od innego statku
                ack1_count++;
                if (ack1_count == size - 1) {
                    changeState( InDockWait );
                    ack1_count = 0;
                }
                break;
            case ACK2: // Odbieram potwierdzenie dostępu do mechaników od innego statku
                ack2_count++;
                if (ack2_count == size - 1) {
                    changeState( InMechWait );
                    ack2_count = 0;
                }
                break;
            case RELEASE: // Inny statek opuścił dok
                // println("Statek %d opuszcza port i wraca do bitwy", pkt.src);
                pthread_mutex_lock( &vecDockMut );
                for (int i = 0; i < dock_tab.size(); i++) {
                    if (std::get<1>(dock_tab.at(i)) == pkt.src) {
                        dock_tab.erase(dock_tab.begin() + i);
                    }
                }
                pthread_mutex_unlock( &vecDockMut );

                pthread_mutex_lock( &vecMechMut );
                for (int i = 0; i < mech_tab.size(); i++) {
                    if (std::get<1>(mech_tab.at(i)) == pkt.src) {
                        mech_tab.erase(mech_tab.begin() + i);
                    }
                }
                pthread_mutex_unlock( &vecMechMut );
                break;
            default:

                break;
        }
    }
}
