#include "main.hpp"
#include "main_thread.hpp"

void mainLoop() {
    // std::vector<std::tuple<int, int>> dock_tab; // tablica na kolejkę dostępu do doków
    // std::vector<std::tuple<int, int, int>> mech_tab; // tablica na kolejkę dostępu do mechaników
    srandom(rank);
    std::tuple<int, int> myDockReq;
    std::tuple<int, int, int> myMechReq;

    while (state != InFinish) {
        int perc = random()%100; 

        if (perc < STATE_CHANGE_PROB) {
            packet_t *pkt = new packet_t;
            if (state == BeforeDockWait) {
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
                        sendPacket( pkt, i, REQ1 );
                }
                
            	debug("Skończyłem wysyłać REQ1");
            } else if (state == InDockWait) {
                pthread_mutex_lock( &vecDockMut );
                auto it = std::find(dock_tab.begin(), dock_tab.end(), myDockReq);
                int index = it - dock_tab.begin();
                pthread_mutex_unlock( &vecDockMut );

                if (index < DOCKS_COUNT) {
                    println("Wchodzę do doku");
                    for (auto i: dock_tab)
                        std::cout << std::get<0>(i) << " " << std::get<1>(i) << ", ";
                    changeState( BeforeMechWait );
                }
                sleep( SEC_IN_STATE );
            } else if (state == BeforeMechWait) {
                println("Czekam na mechaników!");
		        changeState( InSend );
            	int necessary_mechs = random()%(int)(MECHANICS_COUNT / 2) + 1;
                pkt->mech_count = necessary_mechs;
                
                sleep( SEC_IN_STATE );
                changeTime(lamportTime + 1);
                myMechReq = std::make_tuple(lamportTime, rank, necessary_mechs);

                pthread_mutex_lock( &vecMechMut );
                mech_tab.push_back(myMechReq);
                std::sort(mech_tab.begin(), mech_tab.end());
                pthread_mutex_unlock( &vecMechMut );

                for (int i = 0; i < size; i++) {
                    if (i != rank)
                        sendPacket( pkt, i, REQ2 );
                }                

            	debug("Skończyłem wysyłać REQ2");
            } else if (state == InMechWait) {
                // Znajdź siebie
                int used_mechs = 0;
                pthread_mutex_lock( &vecMechMut );
                auto it = std::find(mech_tab.begin(), mech_tab.end(), myMechReq);
                int index = it - mech_tab.begin();
                pthread_mutex_unlock( &vecMechMut );

                for (int i = 0; i <= index; i++) {
                    used_mechs += std::get<2>(mech_tab.at(i));
                    if (used_mechs > MECHANICS_COUNT) 
                        break;
                }

                // sprawdzenie kolejki na mechanikow
                if (used_mechs <= MECHANICS_COUNT) {
                    for (auto i: mech_tab)
                        std::cout << std::get<0>(i) << " " << std::get<1>(i) << " " << std::get<2>(i) << ", ";
                    changeState( InRepair );
                }

                sleep( SEC_IN_STATE );

            } else if (state == InRepair) {
                println("Jestem naprawiany");
                sleep( random() % 4 + 2 ); // sleep 2 - 5
                changeTime( lamportTime + 1 );

                // Usun sie z kolejki
                pthread_mutex_lock( &vecDockMut );
                for (int i = 0; i < dock_tab.size(); i++) {
                    if (std::get<1>(dock_tab(i) == pkt->src)) {
                        dock_tab.erase(dock_tab.begin() + i)
                    }
                }
                pthread_mutex_unlock( &vecDockMut );

                pthread_mutex_lock( &vecMechMut );
                for (int i = 0; i < mech_tab.size(); i++) {
                    if (std::get<1>(mech_tab(i) == pkt->src)) {
                        mech_tab.erase(mech_tab.begin() + i)
                    }
                }
                pthread_mutex_unlock( &vecMechMut );

                for (int i = 0; i < size; i++) {
                    if (i != rank)
                        sendPacket( pkt, i, RELEASE );
                }
                println("Kończę naprawę! Do boju!");
                sleep( SEC_IN_STATE );
                changeState( BeforeDockWait );
            } else { }

        }
        println("Idę spać\n");
        sleep(SEC_IN_STATE);
    }
}
