#include "main.hpp"
#include "main_thread.hpp"

void mainLoop() {
    srandom(rank);
    std::tuple<int, int> myDockReq;
    std::tuple<int, int, int> myMechReq;
    std::stringstream tab_msg;

    while (state != InFinish) {
        int perc = random()%100; 

        if (perc < STATE_CHANGE_PROB) {
            packet_t *pkt = new packet_t;
            if (state == BeforeDockWait) {
		        changeState( InSend );
            	pkt->mech_count = 1; // nieużywane ale trzeba wpisać
                
                sleep( SEC_IN_STATE ); // to nam zasymuluje, że wiadomość trochę leci w kanale
                changeTime(lamportTime + 1);
		        println("Ubiegam się o dok");
                myDockReq = std::make_tuple(lamportTime, rank);

                pthread_mutex_lock( &vecDockMut );
                dock_tab.push_back(myDockReq);
                std::sort(dock_tab.begin(), dock_tab.end());
                pthread_mutex_unlock( &vecDockMut );

            	for (int i = 0; i < size; i++) {
                    if (i != rank)
                        sendPacket( pkt, i, REQ1 );
                }
            } 
            else if (state == InDockWait) {
                pthread_mutex_lock( &vecDockMut );
                auto it = std::find(dock_tab.begin(), dock_tab.end(), myDockReq);
                int index = it - dock_tab.begin();
                pthread_mutex_unlock( &vecDockMut );

                if (index < DOCKS_COUNT) {
                    pthread_mutex_lock( &vecDockMut );
                    for (auto i: dock_tab) 
                        tab_msg << "ts: " << std::get<0>(i) << " r: " << std::get<1>(i) << ", ";
                    pthread_mutex_unlock( &vecDockMut );
                    
                    println("Wchodzę do doku przy stanie %s", tab_msg.str().c_str());
                    tab_msg.str("");
                    changeState( BeforeMechWait );
                }
                sleep( SEC_IN_STATE );
            } 
            else if (state == BeforeMechWait) {
		        changeState( InSend );
            	int necessary_mechs = random()%4 + 2;;//random()%(int)(MECHANICS_COUNT / 2) + 1; // Domyślnie 1 - 5
                pkt->mech_count = necessary_mechs;
                
                sleep( SEC_IN_STATE );
                changeTime(lamportTime + 1);
                println("Ubiegam się o mechaników!");
                myMechReq = std::make_tuple(lamportTime, rank, necessary_mechs);

                pthread_mutex_lock( &vecMechMut );
                mech_tab.push_back(myMechReq);
                std::sort(mech_tab.begin(), mech_tab.end());
                pthread_mutex_unlock( &vecMechMut );

                for (int i = 0; i < size; i++) {
                    if (i != rank) {
                        sendPacket( pkt, i, REQ2 );
                    }
                }
            } 
            else if (state == InMechWait) {
                int used_mechs = 0;

                // Znajdź siebie
                pthread_mutex_lock( &vecMechMut );
                auto it = std::find(mech_tab.begin(), mech_tab.end(), myMechReq);
                int index = it - mech_tab.begin();
                pthread_mutex_unlock( &vecMechMut );

                // sprawdzenie kolejki na mechanikow
                pthread_mutex_lock( &vecMechMut );
                for (int i = 0; i <= index; i++) {
                    used_mechs += std::get<2>(mech_tab.at(i));
                    if (used_mechs > MECHANICS_COUNT) 
                        break;
                }
                pthread_mutex_unlock( &vecMechMut );

                if (used_mechs <= MECHANICS_COUNT) {
                    pthread_mutex_lock( &vecMechMut );
                    for (auto i: mech_tab)
                        tab_msg << "ts: " << std::get<0>(i) << " r: " << std::get<1>(i) << " m: " << std::get<2>(i) << ", ";
                    pthread_mutex_unlock( &vecMechMut );
                    
                    println("Zaczynam naprawę przy stanie %s", tab_msg.str().c_str());
                    tab_msg.str("");
                    changeState( InRepair );
                }

                sleep( SEC_IN_STATE );
            } 
            else if (state == InRepair) {
                sleep( random() % 4 + 2 ); // sleep 2 - 5
                changeTime( lamportTime + 1 );

                // Usun sie z kolejki
                pthread_mutex_lock( &vecDockMut );
                for (int i = 0; i < dock_tab.size(); i++) {
                    if (std::get<1>(dock_tab.at(i)) == rank) {
                        dock_tab.erase(dock_tab.begin() + i);
                    }
                }
                pthread_mutex_unlock( &vecDockMut );

                pthread_mutex_lock( &vecMechMut );
                for (int i = 0; i < mech_tab.size(); i++) {
                    if (std::get<1>(mech_tab.at(i)) == rank) {
                        mech_tab.erase(mech_tab.begin() + i);
                    }
                }
                pthread_mutex_unlock( &vecMechMut );
                
                changeTime( lamportTime + 1 );
                println("Kończę naprawę! Do boju!");

                for (int i = 0; i < size; i++) {
                    if (i != rank)
                        sendPacket( pkt, i, RELEASE );
                }
                sleep( SEC_IN_STATE * 3 );
                changeState( BeforeDockWait );
            } 
            else { }
        }
        sleep(SEC_IN_STATE);
    }
}
