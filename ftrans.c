#include "ftrans.h"

unsigned int addwindowindex(windows *ws, unsigned char *buff, unsigned int size,
                            unsigned int index) {
    struct timespec spec;
    window *iter = (*ws).requests;
    window *tmp = calloc(sizeof(window), 1);

    clock_gettime(CLOCK_REALTIME, &spec);
    (*tmp).ms = floor((spec.tv_nsec / 1.0e6) + 0.5);
    (*tmp).id = index;
    (*tmp).buff = buff;
    (*tmp).size = size;

    if ((*ws).requests == NULL) {
        (*ws).requests = tmp;
    } else {
        while ((*iter).next != NULL) {
            iter = (*iter).next;
        }
        (*iter).next = tmp;
    }

    return index;
}

unsigned int addwindow(windows *ws, unsigned char *buff, unsigned int size) {
    static unsigned int index = 0;
    struct timespec spec;
    window *iter = (*ws).requests;
    window *tmp = calloc(sizeof(window), 1);

    clock_gettime(CLOCK_REALTIME, &spec);
    (*tmp).ms = floor((spec.tv_nsec / 1.0e6) + 0.5);
    (*tmp).id = index++;
    (*tmp).buff = buff;
    (*tmp).size = size;

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
    window *prev = NULL;

    while (tmp != NULL) {
        if ((*tmp).id == index) {
            if (prev == NULL) {
                (*ws).requests = (*tmp).next;
            } else {
                (*prev).next = (*tmp).next;
            }
            free((*tmp).buff);
            free(tmp);
            return 1;
        } else {
            prev = tmp;
            tmp = (*tmp).next;
        }
    }
    return -1;
}

window *getwindow(windows *ws, unsigned int index) {
    window *tmp = (*ws).requests;
    window *prev = NULL;

    while (tmp != NULL) {
        if ((*tmp).id == index) {
            if (prev == NULL) {
                (*ws).requests = (*tmp).next;
            } else {
                (*prev).next = (*tmp).next;
            }
            return tmp;
        } else {
            prev = tmp;
            tmp = (*tmp).next;
        }
    }
    return NULL;
}

unsigned int remainingwindows(windows ws) {
    unsigned int rem = 0;
    window *tmp = ws.requests;

    while (tmp != NULL) {
        tmp = (*tmp).next;
        rem++;
    }
    return rem;
}

window *chktimewindows(windows *ws, long ttl) {
    window *iter = (*ws).requests;
    window *stale = NULL;
    window *tmp = NULL;
    
    long ms;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    ms = floor((spec.tv_nsec / 1.0e6) + 0.5);

    while (iter != NULL) {
        if ((*iter).ms + ttl > ms) {
            window *tmp = getwindow(ws,(*iter).id);
            if(tmp != NULL) {
                if(stale == NULL) {
                    stale = tmp;
                } else {
                    (*tmp).next = stale;
                    stale = tmp;
                }
            }
        }
        iter = (*iter).next;
    }

    //appand the stale list to the end of the remaining nodes
    tmp = (*ws).requests;
    while(tmp != NULL && (*tmp).next != NULL) {
        tmp = (*tmp).next;
    }

    if(tmp == NULL) { 
        (*ws).requests = stale;
    } else {
        (*tmp).next = stale;
    }

    return stale;
}

void printwindows(windows ws) {
    window *tmp = ws.requests;
    while (tmp != NULL) {
        printf("Window: id: %d bufflen: %d addr: %04x next: %04x\n", (*tmp).id,
               (*tmp).size, tmp, (*tmp).next);
        tmp = (*tmp).next;
    }
}
