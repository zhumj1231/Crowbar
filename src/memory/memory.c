#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

static void default_error_handler(MEM_Controller controller, 
								char *filename, int line, char *msg);

static struct MEM_Controller_tag st_default_controller = {
    NULL,/* stderr */
    default_error_handler,
    MEM_FAIL_AND_EXIT
};
MEM_Controller mem_default_controller = &st_default_controller;

typedef union {
    long        l_dummy;
    double      d_dummy;
    void        *p_dummy;
} Align;

#define MARK_SIZE		(4)

typedef struct {
    int         size;
    char        *filename;
    int         line;
    Header      *prev;
    Header      *next;
    unsigned char       mark[MARK_SIZE];
} HeaderStruct;

#define ALIGN_SIZE      (sizeof(Align))
#define revalue_up_align(val)   ((val) ? (((val) - 1) / ALIGN_SIZE + 1) : 0)
#define HEADER_ALIGN_SIZE       (revalue_up_align(sizeof(HeaderStruct)))
#define MARK (0xCD)

union Header_tag {
    HeaderStruct        s;
    Align               u[HEADER_ALIGN_SIZE];
};

static void default_error_handler(MEM_Controller controller, 
							char *filename, int line, char *msg) {
    fprintf(controller->error_fp,
    		"MEM:%s failed in %s at %d\n", msg, filename, line);
}

static void error_handler(MEM_Controller controller, 
						char *filename, int line, char *msg) {
    if (controller->error_fp == NULL) {
        controller->error_fp = stderr;
    }
    controller->error_handler(controller, filename, line, msg);

    if (controller->fail_mode == MEM_FAIL_AND_EXIT) {
        exit(1);
    }
}

MEM_Controller MEM_create_controller(void) {
    MEM_Controller      p;
    p = MEM_malloc_func(&st_default_controller, __FILE__, __LINE__,
                        sizeof(struct MEM_Controller_tag));
    *p = st_default_controller;
    return p;
}

#ifdef DEBUG

static void chain_block(MEM_Controller controller, Header *new_header) {
    if (controller->block_header) {
        controller->block_header->s.prev = new_header;
    }
    new_header->s.prev = NULL;
    new_header->s.next = controller->block_header;
    controller->block_header = new_header;
}

static void rechain_block(MEM_Controller controller, Header *header) {
    if (header->s.prev) {
        header->s.prev->s.next = header;
    } else {
        controller->block_header = header;
    }
    if (header->s.next) {
        header->s.next->s.prev = header;
    }
}

static void unchain_block(MEM_Controller controller, Header *header) {
    if (header->s.prev) {
        header->s.prev->s.next = header->s.next;
    } else {
        controller->block_header = header->s.next;
    }
    if (header->s.next) {
        header->s.next->s.prev = header->s.prev;
    }
}

void set_header(Header *header, int size, char *filename, int line) {
    header->s.size = size;
    header->s.filename = filename;
    header->s.line = line;
    memset(header->s.mark, MARK, (char*)&header[1] - (char*)header->s.mark);
}

void set_tail(void *ptr, int alloc_size) {
    char *tail;
    tail = ((char*)ptr) + alloc_size - MARK_SIZE;
    memset(tail, MARK, MARK_SIZE);
}

void check_mark_sub(unsigned char *mark, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (mark[i] != MARK) {
            fprintf(stderr, "bad mark\n");
            abort();
        }
    }
}

void check_mark(Header *header) {
    unsigned char        *tail;
    check_mark_sub(header->s.mark, (char*)&header[1] - (char*)header->s.mark);
    tail = ((unsigned char*)header) + header->s.size + sizeof(Header);
    check_mark_sub(tail, MARK_SIZE);
}

#endif /* DEBUG */