/*****************************************************************************
 *
 *    seg_mem.c
 *
 *    By:   Sitara Rao (srao03) and Arnav Kothari (akotha02)
 *    Date: 11/20/2019
 *      
 *    Seg_mem module implementation for use in the um program.
 *    Defines all functions that access and modify um memory, including
 *    initialization of program, 
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "assert.h"
#include "seq.h"
#include "seg_mem.h"
#include "bitpack.h"


/* Constants for memory mmapping */
static const uint32_t MAX_SEGMENTS = ~0;
static const uint32_t SEGS = 500;

/**************************************************************************
*                Allocate/initialize/free segmented memory                *
***************************************************************************/
/*
 * seg_mem_new()
 * Parameters: none
 * Returns: a pointer to a newly allocated seg_mem_obj
 */
seg_mem_obj* seg_mem_new()
{
    seg_mem_obj *new_seg_mem = malloc(sizeof(seg_mem_obj));
    assert(new_seg_mem != NULL);
    new_seg_mem->mapped = Seq_new(SEGS);
    new_seg_mem->unmapped = Seq_new(SEGS);
    return new_seg_mem;
}

/*
 * seg_mem_free()
 * Parameters: a seg_mem_obj pointer
 * Frees memory associated with passed in memory object
 * Returns nothing
 */
void seg_mem_free(seg_mem_obj* mem)
{
    assert(mem != NULL);
    int unmapped_len = Seq_length(mem->unmapped);
    int mapped_len = Seq_length(mem->mapped);

    /* Free mapped segments */
    for (int i = 0; i < mapped_len; i++) {
        uint32_t *curr = Seq_remlo(mem->mapped);
        free(curr);
        curr = NULL;
    }

    /* Free unmapped segment ids */
    for (int i = 0; i < unmapped_len; i++) {
        uint32_t *curr = Seq_remlo(mem->unmapped);
        free(curr);
        curr = NULL;
    }

    /* Free the sequences themselves, then the pointer to the object */
    Seq_free(&(mem->mapped));
    Seq_free(&(mem->unmapped));
    free(mem);
}

/*
 * init_prog()
 * Parameters: a seg_mem_obj pointer, FILE * to file containing um program
 * Reads in all instructions in program file and stores in m[0]
 * Returns nothing
 */
void init_prog(seg_mem_obj *mem, FILE *prog)
{
    assert(mem != NULL && prog != NULL);
    /* Read in file instructions to a sequence that can expand to fit them */
    Seq_T instructions = Seq_new(SEGS);
    int curr = 0;
    while (! feof(prog)){
        uint32_t *prog_bits = malloc(sizeof(*prog_bits));
        assert(prog_bits != NULL);
        *prog_bits = 0;
        
         for (int i = 3; i >= 0; i--){
            curr = fgetc(prog);
            if (curr == EOF) {
                free(prog_bits);
                break;
            }
            *prog_bits = Bitpack_newu(*prog_bits, 8, i * 8, curr);
        }
        if (curr == EOF) {
            break;
        }
        Seq_addhi(instructions, prog_bits);
    }

    /* Get number of instructions read in */
    int len = Seq_length(instructions);

    /* Create memory segment (array) to hold program instructions */
    /* Array[0] is the size of the segment, or total number of instructions */
    uint32_t *mem_seg = malloc((len + 1) * sizeof(*mem_seg));
    assert(mem_seg != NULL);
    mem_seg[0] = len;
    for (int i = 1; i <= len; i++) {
        uint32_t *curr = Seq_remlo(instructions);
        mem_seg[i] = *curr;
        free(curr);
    }

    /* Insert instructions array into m[0] segment */
    Seq_addlo(mem->mapped, mem_seg);

    /* Add indices 1 - 499 to sequence storing unmapped segment ids */
    for (unsigned i = 1; i < SEGS; i++) {
        uint32_t *curr_index = malloc(sizeof(*curr_index));
        assert(curr_index != NULL);
        *curr_index = i;
        Seq_addhi(mem->unmapped, curr_index);
    }
    
    /* Free the placeholder instruction sequence */
    Seq_free(&instructions);
}

/**************************************************************************
*                  Update/access memory per UM instructions               *
***************************************************************************/
/*
 * seg_map()
 * Parameters: a seg_mem_obj pointer, uint32_t size (of segment)
 * Reads in all instructions in program file and stores in m[0]
 * Returns segment id of newly mapped segment
 */
uint32_t seg_map(seg_mem_obj *mem, uint32_t size)
{
    assert(mem != NULL);
    assert((uint32_t)Seq_length(mem->mapped) < MAX_SEGMENTS);

    /* Increase number of unmapped ids by SEGS if client has used them all */
    if (Seq_length(mem->unmapped) == 0) {
        unsigned int start = Seq_length(mem->mapped);
        for (unsigned i = start; i < start + SEGS; i++) {
            uint32_t *curr_index = malloc(sizeof(*curr_index));
            assert(curr_index != NULL);
            *curr_index = i;
            Seq_addhi(mem->unmapped, curr_index);
        }
    }

    /* 
     * Create array of size + 1 words to represent segment
     * store size in array[0] and init all other values in array to 0
     */
    uint32_t *map_seg = malloc((size + 1) * sizeof(*map_seg));
    assert(map_seg != NULL);
    map_seg[0] = size;
    for (unsigned i = 1; i <= size; i++) {
        map_seg[i] = 0;
    }

    /* get the id of an unmapped segment */
    uint32_t *new_segment = Seq_remlo(mem->unmapped);
    uint32_t seg_id = *new_segment;

    /*
     * Either add new segment to high end of mapped sequence if its larger
     * than the max index; else put it at a particular index in the sequence
     */
    if (seg_id >= (uint32_t)Seq_length(mem->mapped)) {
        Seq_addhi(mem->mapped, map_seg);
    } else {
        uint32_t *old_seg = Seq_put(mem->mapped, seg_id, map_seg);
        free(old_seg);
    }

    free(new_segment);

    /* Return id that identifies the mapped segment */
    return seg_id;
}

/*
 * seg_unmap()
 * Parameters: a seg_mem_obj pointer, uint32_t seg_id
 * Frees the segment at m[seg_id]
 * Returns nothing
 */
void seg_unmap(seg_mem_obj *mem, uint32_t seg_id)
{
    assert(mem != NULL);
    uint32_t *index = malloc(sizeof(*index));
    assert(index != NULL);
    *index = seg_id;
    Seq_addlo(mem->unmapped, index);
}

/*
 * seg_load()
 * Parameters: a seg_mem_obj pointer, uint32_t b, and uint32_t c
 * Returns the word stored at m[b][c]
 */
uint32_t seg_load(seg_mem_obj *mem, uint32_t b, uint32_t c)
{
    assert(mem != NULL);

    /* Add one to c because m[b][0] holds size of the segment */
    return ((uint32_t *)Seq_get(mem->mapped, b))[c + 1];
}

/*
 * seg_store()
 * Parameters: a seg_mem_obj pointer, uint32_ts a, b, and c
 * Stores c in memory at m[a][b]
 * Returns nothing
 */
void seg_store(seg_mem_obj *mem, uint32_t a, uint32_t b, uint32_t c)
{
    assert(mem != NULL);
    uint32_t *segment = Seq_get(mem->mapped, a);

    /* Again, add one to segment offset because first index is the size */
    segment[b + 1] = c;
}

/*
 * seg_load_prog()
 * Parameters: a seg_mem_obj pointer, uint32_t b
 * Duplicates memory segment in m[b]
 * Discards current m[0] and replaces it with copy of m[b]
 * Returns nothing
 */
void seg_load_prog(seg_mem_obj *mem, uint32_t b)
{
    assert(mem != NULL);
    /* If segment to duplicate is m[0] we have nothing to do */
    if (b == 0) {
        return;
    }

    /* Allocate size of segment m[b] to store duplicated segment */
    uint32_t seglen = ((uint32_t *)Seq_get(mem->mapped, b))[0];
    uint32_t *duplicate = malloc((seglen + 1) * sizeof(*duplicate));
    assert(duplicate != NULL);

    /* Copy over values into duplicate segment */
    duplicate[0] = seglen;
    for (unsigned i = 1; i <= seglen; i++) {
        duplicate[i] = ((uint32_t *)Seq_get(mem->mapped, b))[i];
    }

    /* Free current 0 segment */
    uint32_t *currprog = Seq_remlo(mem->mapped);
    free(currprog);
    currprog = NULL;

    /* Put duplicated memory segment in m[0] */
    Seq_addlo(mem->mapped, duplicate);
}

/***************************************************************************
*                       Iterate through m[0] segment                       *
****************************************************************************/
/*
 * program_size()
 * Parameters: pointer to a seg_mem_obj
 * Returns size of program stored in m[0]
 */
uint32_t program_size(seg_mem_obj* mem)
{
    assert(mem != NULL);
    return ((uint32_t *)Seq_get(mem->mapped, 0))[0];
}

/*
 * get_prog_instruction()
 * Parameters: pointer to a seg_mem_obj; uint32_t prog_ctr
 * Returns the word at m[0][prog_ctr]
 */
uint32_t get_prog_instruction(seg_mem_obj* mem, uint32_t prog_ctr)
{
    assert(mem != NULL);
    return seg_load(mem, 0, prog_ctr);
}
