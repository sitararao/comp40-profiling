/*****************************************************************************
 *
 *    um.c
 *
 *    By:   Sitara Rao (srao03) and Arnav Kothari (akotha02)
 *    Date: 11/20/2019
 *      
 *    Main driver for the universal machine (um) program. Defines the main()
 *    function as well as um_new(), um_run(), and um_free().
 *    Relies on 3 modules:
 *          - seg_mem for accessing/modifying memory
 *          - instructions for handling 13 of the 14 defined um instructions
 *          - bitpack for unpacking 
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "um.h"
#include "instructions.h"
#include "seg_mem.h"
#include "bitpack.h"
#include "assert.h"

typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

/*******************************************
 * Constants for unpacking um instructions *
 *******************************************/
static const unsigned OP_WIDTH  = 4;
static const unsigned OP_LSB    = 28;

static const unsigned REG_WIDTH = 3;
static const unsigned RA_LSB    = 6;
static const unsigned RB_LSB    = 3;
static const unsigned RC_LSB    = 0;

static const unsigned RA_LV_LSB = 25;
static const unsigned VAL_WIDTH = 25;
static const unsigned VAL_LSB   = 0;


/******************************************************
 *                    UM Functions                    *
 ******************************************************/
/*
 * main()
 * Takes in the name of one um program file on the command line.
 * If filename is not provided or file does not exist it is a CRE.
 * Calls functions to initialize, run, and free a um object.
 * Returns 0. 
 */
int main(int argc, char* argv[])
{
    assert(argc == 2);
    FILE* fp = fopen(argv[1], "r");
    assert(fp != NULL);

    um_obj *um = um_new(fp);
    um_run(um);
    um_free(um);

    fclose(fp);
    return 0;
}

/*
 * um_new()
 * Takes in a FILE pointer to the file containing the um program to be run.
 * Allocates memory for a um object.
 * Sets program_counter and all registers to 0.
 * Calls init_prog() to read instructions from the program file into m[0].
 * Returns a pointer to the newly created um object.
 */
um_obj* um_new(FILE* fp)
{
    um_obj *new_um = malloc(sizeof(um_obj));
    new_um->memory = seg_mem_new();

    new_um->program_counter = 0;
    for (int i = 0; i < 8; i++) {
        new_um->registers[i] = 0;
    }

    /* Load contents of um program into m[0] */
    init_prog(new_um->memory, fp);

    return new_um;
}

/*
 * um_run()
 * Takes in a pointer to an initialized um object.
 * Iterates through instructions in m[0], using program counter.
 * Unpacks values in each instruction and then executes by calling the
 * appropriate function from instructions module.
 * Returns nothing when computation has finished (either reached end of
 * m[0] or encoutered a "halt" instruction).
 */
void um_run(um_obj *um)
{
    while (um->program_counter < program_size(um->memory)) {

        uint32_t curr_instr = get_prog_instruction(um->memory, 
                                                   um->program_counter);
        Um_opcode opcode = Bitpack_getu(curr_instr, OP_WIDTH, OP_LSB);

        /* Used by opcodes 0-12 */
        uint32_t reg_a = Bitpack_getu(curr_instr, REG_WIDTH, RA_LSB);
        uint32_t reg_b = Bitpack_getu(curr_instr, REG_WIDTH, RB_LSB);
        uint32_t reg_c = Bitpack_getu(curr_instr, REG_WIDTH, RC_LSB);

        /* Used only by opcode 13 */
        uint32_t val;

        /*
         * Call appropriate instruction handler function along with (pointers
         * to) necessary um elements (ex. registers, register indices, memory)
         */
        switch(opcode)
        {
            case CMOV: 
                    cond_mov(um->registers, reg_a, reg_b, reg_c);
                    break;
            case SLOAD:
                    segment_load(um->registers, um->memory, reg_a, reg_b,
                                                                   reg_c);
                    break;
            case SSTORE:
                    segment_store(um->registers, um->memory, reg_a, reg_b,
                                                                    reg_c);
                    break;
            case ADD:
                    addition(um->registers, reg_a, reg_b, reg_c);
                    break;
            case MUL:
                    multiplication(um->registers, reg_a, reg_b, reg_c);
                    break;
            case DIV:
                    division(um->registers, reg_a, reg_b, reg_c);
                    break;
            case NAND:
                    bitwise_nand(um->registers, reg_a, reg_b, reg_c);
                    break;
            case HALT:
                    return;
            case ACTIVATE:
                    map_segment(um->registers, um->memory, reg_b, reg_c);
                    break;
            case INACTIVATE:
                    unmap_segment(um->registers, um->memory, reg_c);
                    break;
            case OUT:
                    output(um->registers, reg_c);
                    break;
            case IN: 
                    input(um->registers, reg_c);
                    break;
            case LOADP:
                    um->program_counter = load_program(um->registers,
                                                       um->memory, reg_b, 
                                                                   reg_c);
                    break;
            case LV: 
                    reg_a = Bitpack_getu(curr_instr, REG_WIDTH, RA_LV_LSB);
                    val   = Bitpack_getu(curr_instr, VAL_WIDTH, VAL_LSB);
                    load_value(um->registers, reg_a, val);
                    break;
            default:
                    /* Opcode not recognized */ 
                    break;
        }

        /* Increment program counter to get next instruction */
        um->program_counter++;                  
    }
}

/*
 * um_free()
 * Takes a pointer to a um object
 * Frees memory associated with the um object (including its memory)
 * Returns nothing.
 */
void um_free(um_obj* um)
{    
    seg_mem_free(um->memory);
    free(um);
}