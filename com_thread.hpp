#ifndef COM_THREAD_HPP
#define COM_THREAD_HPP
#include <algorithm>

/* wątek komunikacyjny: odbieranie wiadomości i reagowanie na nie poprzez zmiany stanu */
void *startComThread(void *ptr);

#endif
