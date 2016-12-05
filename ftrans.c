#include "ftrans.h"

unsigned int addwindow(windows *ws, char *buff) {
    static unsigned int index = 0;
    struct timespec spec;
    window *iter = (*ws).requests;
    window *tmp = calloc(sizeof(window), 1);

    clock_gettime(CLOCK_REALTIME, &spec);
    (*tmp).ms = floor((spec.tv_nsec / 1.0e6) + 0.5);
    (*tmp).id = index++;
    (*tmp).buff = buff;

    if ((*ws).requests == NULL) {
        (*ws).requests = tmp;
    } else {
        while ((*iter).next != NULL) {
            iter = (*iter).next;
        }
        (*iter).next = tmp;
    }

    return index - 1;
}

int removewindow(windows *ws, unsigned int index) {
    window *tmp = (*ws).requests;

    while (tmp != NULL) {
        if ((*tmp).id == index) {
            free(tmp);
            return 1;
        } else {
            tmp = (*tmp).next;
        }
    }
    return -1;
}

window *getwindow(windows *ws, unsigned int index) {
    window *tmp = (*ws).requests;

    while (tmp != NULL) {
        if ((*tmp).id == index) {
            return tmp;
        } else {
            tmp = (*tmp).next;
        }
    }
    return NULL;
}

unsigned int remainingwindows(windows *ws) {
    unsigned int rem = 0;
    window *tmp = (*ws).requests;

    while (tmp != NULL) {
        tmp = (*tmp).next;
        rem++;
    }
    return rem;
}

window *chktimewindows(windows *ws, long ttl) {
    window *stale = NULL;
    window *iter = (*ws).requests;
    window *prev = NULL;
    long ms;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    ms = floor((spec.tv_nsec / 1.0e6) + 0.5);

    while (iter != NULL) {
        if ((*iter).ms + ttl > ms) {
            // we need to resend this message because the response has timed out
            // so we remove it and add it to the linked list of windows returned
            if (prev != NULL) {
                (*prev).next = (*iter).next;
                (*iter).next = stale;
                stale = iter;
            } else {
                (*ws).requests = NULL;
            }
        } else {
            prev = iter;
        }
        iter = (*iter).next;
    }
    return stale;
}
