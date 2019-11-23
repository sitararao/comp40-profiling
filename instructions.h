/*****************************************************************************
 *
 *    instructions.h
 *
 *    By:   Sitara Rao (srao03) and Arnav Kothari (akotha02)
 *    Date: 11/20/2019
 *      
 *    Header file for instructions module
 *
 *****************************************************************************/
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "assert.h"
#include "seg_mem.h"

typedef uint32_t reg;

/* Functions that update registers */
void cond_mov        (uint32_t regs[], reg a, reg b, reg c);
void addition        (uint32_t regs[], reg a, reg b, reg c);
void multiplication  (uint32_t regs[], reg a, reg b, reg c);
void division        (uint32_t regs[], reg a, reg b, reg c);
void bitwise_nand    (uint32_t regs[], reg a, reg b, reg c);
void load_value      (uint32_t regs[], reg a, uint32_t value);

/* Functions for I/O */
void output          (uint32_t regs[], reg c);
void input           (uint32_t regs[], reg c);

/* Functions that access and/or update memory */
void segment_load    (uint32_t regs[], seg_mem_obj *mem, reg a, reg b, reg c);
void segment_store   (uint32_t regs[], seg_mem_obj *mem, reg a, reg b, reg c);
void map_segment     (uint32_t regs[], seg_mem_obj *mem, reg b, reg c);
void unmap_segment   (uint32_t regs[], seg_mem_obj *mem, reg c);
uint32_t load_program(uint32_t regs[], seg_mem_obj *mem, reg b, reg c);

#endif
