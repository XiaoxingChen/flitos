//
// Created by syf on 2022/10/23.
//
#include <stdlib.h>
#include <stddef.h>
#include "include/timer.h"
#include "include/SList.h"


/**
 * define the method of dealing with interrupt cause by timer
 * @param mcause
 * @return
 */
void increase_timer();

void handle_timer();

SLTNode *node;
extern volatile char *VIDEO_MEMORY;

volatile uint32_t position3 = 400;

int handle_time_interrupt(int mcause) {
    // determining whether the interrupt is caused by timer
    // fixme: whether to use == or seventh bits
    if (mcause != 0x80000007) {
        return 0;
    }
    if (node == NULL) {
        node = initList();
    } else {
    }
    // dispatch customized handlers
    handle_timer();
    return 1;
}

int size_linkedList() {
    SLTNode *curr2 = node->next;
    int result = 0;
    while (curr2 != NULL) {
        curr2 = curr2->next;
        result++;
    }
    return result;
}

void handle_timer() {
    SLTNode *curr = node->next;
    int flagCounterZ = 0;
    while (curr != NULL) {
        (curr->data)(0);
        curr = curr->next;
        flagCounterZ++;
    }
}

int register_handler(uint32_t addressInt) {
    if (node == NULL) {
        node = initList();
    }
    insertNode(node, ((ptr) addressInt));

    return 1;
}


int deregister_handler(uint32_t (*ptr)(uint32_t event_code)) {
    deleteNode(node, ptr);
    return 1;
}

