#ifndef MMGR_H
#define MMGR_H

void *memory_alloc(unsigned size);
char *get_block_string(int *);
void  update_current_block();
void  memory_destroy();
int	  memory_init ();

#endif
