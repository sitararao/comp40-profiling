/*****************************************************************************
 *
 *    seg_mem.h
 *
 *    By:   Sitara Rao (srao03) and Arnav Kothari (akotha02)
 *    Date: 11/20/2019
 *      
 *    Header file for seg_mem module
 *
 *****************************************************************************/
#ifndef SEG_MEM_H
#define SEG_MEM_H

#include <stdint.h>
#include "seq.h"

typedef struct seg_mem_obj {
	Seq_T mapped;
	Seq_T unmapped;
} seg_mem_obj;

/* Functions to allocate, initialize, and free segmented memory */
seg_mem_obj* seg_mem_new();
void init_prog(seg_mem_obj *obj, FILE *prog);
void seg_mem_free(seg_mem_obj *obj);

/* Functions used by the um to iterate through program instructions */
uint32_t program_size(seg_mem_obj* mem);
uint32_t get_prog_instruction(seg_mem_obj* mem, uint32_t prog_ctr);

/* Functions that update and access memory according to instructions */
uint32_t seg_map  (seg_mem_obj *obj, uint32_t size);
void     seg_unmap(seg_mem_obj *obj, uint32_t seg_id);
uint32_t seg_load(seg_mem_obj *obj, uint32_t b, uint32_t c);
void     seg_store(seg_mem_obj *obj, uint32_t a, uint32_t b, uint32_t c);
void     seg_load_prog(seg_mem_obj *obj, uint32_t b);



#endif