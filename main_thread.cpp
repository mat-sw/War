#include "main.hpp"
#include "main_thread.hpp"

void mainLoop() {
    // std::vector<std::tuple<int, int>> dock_tab; // tablica na kolejkę dostępu do doków
    // std::vector<std::tuple<int, int, int>> mech_tab; // tablica na kolejkę dostępu do mechaników
    srandom(rank);
    std::tuple<int, int> myDockReq;
    std::tuple<int, int> myMechReq;
    while (stan != InFinish) {
        int perc = random()%100; 

        if (perc < STATE_CHANGE_PROB) {
            packet_t *pkt = new packet_t;
            if (stan == BeforeDockWait) {
		        debug("Ubiegam się o dok!\n");
		        changeState( InSend );
            	pkt->mech_count = 1; // nieużywane ale trzeba wpisać
                
                sleep( SEC_IN_STATE ); // to nam zasymuluje, że wiadomość trochę leci w kanale
                changeTime(lamportTime + 1);
                myDockReq = std::make_tuple(lamportTime, rank);

                pthread_mutex_lock( &vecDockMut );
                dock_tab.push_back(myDockReq);
                std::sort(dock_tab.begin(), dock_tab.end());
                pthread_mutex_unlock( &vecDockMut );

            	for (int i = 0; i < size; i++) {
                    if (i != rank)
                        sendPacket( pkt, i, REQ1); // main.cpp - sendpacket() zmienia za każdym razem zegar lamporta. OK?
                }
                
            	// changeState( InMechWait );
            	debug("Skończyłem wysyłać");
            } else if (stan == InDockWait) {
                auto it = std::find(dock_tab.begin(), dock_tab.end(), myDockReq);
                int index = it - dock_tab.begin();
                if (index < DOCKS_COUNT) {
                    println("Wchodzę do doku");
                    changeState( BeforeMechWait );
                }
            } else if (stan == BeforeMechWait) {
                println("Czekam na mechaników!");
		        changeState( InSend );
            	int necessary_mechs = random()%(int)(MECHANICS_COUNT / 2) + 1;
                pkt->mech_count = necessary_mechs;
                
                sleep( SEC_IN_STATE ); // to nam zasymuluje, że wiadomość trochę leci w kanale
                changeTime(lamportTime + 1);
                for (int i = 0; i < size; i++) {
                    sendPacket( pkt, i, REQ2);
                }                

            	// changeState( InMechWait );
            	debug("Skończyłem wysyłać");
            } else if (stan == InMechWait) {
                
            } else { }

        }
        println("Idę spać\n");
        sleep(SEC_IN_STATE);
    }
}
