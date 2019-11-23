/*****************************************************************************
 *
 *    um.h
 *
 *    By:   Sitara Rao (srao03) and Arnav Kothari (akotha02)
 *    Date: 11/20/2019
 *      
 *    Header file for um program driver
 *
 *****************************************************************************/
#ifndef UM_H
#define UM_H

#include "seg_mem.h"

/* um struct declaration */
typedef struct um_obj {
    seg_mem_obj *memory;
    uint32_t registers[8];
    uint32_t program_counter;
} um_obj;

/* um functions called by main() */
um_obj* um_new(FILE* ptr);
void um_run(um_obj *um);
void um_free(um_obj* um);

#endif