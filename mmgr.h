#ifndef MMGR_H
#define MMGR_H

void *memory_alloc(unsigned size);
char *get_block_string();
int	  release_full_block();
void  update_current_block();
void  memory_destory();
int	  memory_init ();

#endif
