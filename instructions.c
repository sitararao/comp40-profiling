/*****************************************************************************
 *
 *    instructions.c
 *
 *    By:   Sitara Rao (srao03) and Arnav Kothari (akotha02)
 *    Date: 11/20/2019
 *      
 *    Instructions module implementation for use in the um program.
 *    Defines all functions that handle um instructions (not including "halt")
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include "instructions.h"
#include "seg_mem.h"
#include "assert.h"


/*
 * cond_mov()
 * Parameters: array of UM registers; indices of registers a, b, and c
 * If r[c] does not equal 0, sets r[a] equal to r[b]
 * Returns nothing
 */
void cond_mov(uint32_t regs[], reg a, reg b, reg c)
{
    assert(a < 8 && b < 8 && c < 8);

    if (regs[c] == 0) {
        return;
    }

    regs[a] = regs[b];
}

/*
 * segment_load()
 * Parameters: array of UM registers; pointer to um memory object;
 *             indices of registers a, b, and c
 * Calls seg_mem function seg_load() to get value in m[r[b]][r[c]] 
 *             and stores this value in r[a]
 * Returns nothing
 */
void segment_load(uint32_t regs[], seg_mem_obj *mem, reg a, reg b, reg c)
{
    assert(a < 8 && b < 8 && c < 8);
    regs[a] = seg_load(mem, regs[b], regs[c]);
}

/*
 * segment_store()
 * Parameters: array of UM registers; pointer to um memory object;
 *             indices of registers a, b, and c
 * Calls seg_mem function seg_store() to store value in r[c] in m[r[a]][r[b]]
 * Returns nothing
 */
void segment_store(uint32_t regs[], seg_mem_obj *mem, reg a, reg b, reg c)
{
    assert(a < 8 && b < 8 && c < 8);
    seg_store(mem, regs[a], regs[b], regs[c]);
}

/*
 * addition()
 * Parameters: array of UM registers; indices of registers a, b, and c
 * Sets r[a] equal to r[b] + r[c]
 * Returns nothing
 */
void addition(uint32_t regs[], reg a, reg b, reg c)
{
    assert(a < 8 && b < 8 && c < 8);
    regs[a] = regs[b] + regs[c];
}

/*
 * multiplication()
 * Parameters: array of UM registers; indices of registers a, b, and c
 * Sets r[a] equal to r[b] * r[c]
 * Returns nothing
 */
void multiplication(uint32_t regs[], reg a, reg b, reg c)
{
    assert(a < 8 && b < 8 && c < 8);
    regs[a] = (regs[b] * regs[c]);
}

/*
 * division()
 * Parameters: array of UM registers; indices of registers a, b, and c
 * Sets r[a] equal to r[b] / r[c]
 * Returns nothing
 */
void division(uint32_t regs[], reg a, reg b, reg c)
{
    assert(a < 8 && b < 8 && c < 8);
    regs[a] = regs[b] / regs[c];
}

/*
 * bitwise_nand()
 * Parameters: array of UM registers; indices of registers a, b, and c
 * Sets r[a] equal to ~(r[b] & r[c])
 * Returns nothing
 */
void bitwise_nand(uint32_t regs[], reg a, reg b, reg c)
{
    assert(a < 8 && b < 8 && c < 8);
    regs[a] = ~(regs[b] & regs[c]);
}

/*
 * map_segment()
 * Parameters: array of UM registers; pointer to um memory object;
 *             indices of registers b and c
 * Calls seg_map() from seg_mem module to map a chunk of memory of size r[c]
 * R[b] is set to the mapped segment's id in memory, accessible by m[r[b]].
 * Returns nothing
 */
void map_segment(uint32_t regs[], seg_mem_obj *mem, reg b, reg c)
{
    assert(b < 8 && c < 8);
    regs[b] = seg_map(mem, regs[c]);
}

/*
 * unmap_segment()
 * Parameters: array of UM registers; pointer to um memory object;
 *             index of register c
 * Calls seg_unmap() from seg_mem module to unmap segment m[r[c]]
 * Returns nothing
 */
void unmap_segment(uint32_t regs[], seg_mem_obj *mem, reg c)
{
    assert(c < 8);
    seg_unmap(mem, regs[c]);
}

/*
 * output()
 * Parameters: array of UM registers; index of register c
 * Prints contents of r[c] to stdout
 * Unchecked runtime error for r[c] to be < 0 or > 255
 * Returns nothing
 */
void output(uint32_t regs[], reg c)
{
    assert(c < 8);
    fputc(regs[c], stdout);
}

/*
 * input()
 * Parameters: array of UM registers; index of register c
 * Stores input in r[c]; if end of input signaled, stores all 1s in r[c]
 * CRE for input value to be < 0 or > 255
 * Returns nothing
 */
void input(uint32_t regs[], reg c)
{
    assert(c < 8);

    int input = getchar();
    assert((input <= 255 && input >= 0) || input == EOF);
    if (input != EOF) {
        regs[c] = input;
    } else {
        regs[c] = ~0;
    }
}

/*
 * load_program()
 * Parameters: array of UM registers; pointer to um memory object;
 *             indices of registers b and c
 * Calls seg_load_prog() to duplicate m[r[b]] and overwrite current contents
 *             of m[0]
 * Returns value to update um's program counter to
 */
uint32_t load_program(uint32_t regs[], seg_mem_obj *mem, reg b, reg c)
{
    assert(b < 8 && c < 8);
    seg_load_prog(mem, regs[b]);

    /*
     * Want next instruction to set prog_counter to r[c], but upon exiting
     * program counter will increment by 1 so we return r[c] - 1
     */
    return regs[c] - 1;
}

/*
 * load_value()
 * Parameters: array of UM registers; index of register a; a uint32_t value
 * Sets r[a] equal to value passed in
 * Returns nothing
 */
void load_value(uint32_t regs[], reg a, uint32_t value)
{
    assert(a < 8);
    regs[a] = value;
}