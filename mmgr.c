#include "mmgr.h"
#include "list.h"
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>

#define BLOCK_SIZE  (1<<12) /* 块大小 */
#define BLOCK_COUNT   5 /* 初始块数 */

struct block_list {
	void		*block;
	struct list  lst;
};

/* 全局唯一的内存分配实例 */
static struct {
    struct block_list free_block;   /* 空闲内存块链表 */
	struct block_list full_block;   /* 已使用内存块链表 */
	struct block_list*current_block;/* 当前使用块 */
    unsigned     block_size;		/* 每个内存块的大小 */
    unsigned     block_count;		/* 已申请内存块的总数 */
    unsigned     used_size;			/* 当前块已使用大小 */
    void        *begin;			/* 当前缓存块的当前地址 */
} memory;

/* 先用互斥锁，以后修改为更合适的锁 */
static pthread_mutex_t alloc_mutex;

/*
 * @function:get_free_block
 * @description:获取一个空闲的块
 * @calls:list_empty	malloc
 *				list_insert	list_remove
 * @called by:memory_init update_current_block
 *
 */
static struct block_list *
get_free_block()
{
	int i;
    struct block_list *free;
	/* 如果已经没有了空闲的块，那么重新分配 */
	if (list_empty (&(memory.free_block.lst)))
	{
		/* 再分配BLOCK_COUNT个内存块 */
		memory.block_count += BLOCK_COUNT;
		for (i = 0; i < BLOCK_COUNT; ++i)
		{
			void *block = malloc(memory.block_size);
			struct block_list *block_node_ptr =
				(struct block_list *)malloc(sizeof(struct block_list));
			block_node_ptr->block = block;
			list_insert(&(memory.free_block.lst), &(block_node_ptr->lst));
		}
	}

	free = list_head(&(memory.free_block.lst), struct block_list, lst);
	list_remove (&(free->lst));

    return free;
}

/*
 * @function:get_full_block
 * @description:获取一个写满的块
 * @calls:list_empty
 *
 */
char *
get_block_string()
{
	char *ret = NULL;
	struct block_list *full;
	if (!list_empty (&(memory.full_block.lst)))
	{
		full = list_head(&(memory.full_block.lst), struct block_list, lst);
		ret = (char *)full->block;
	}

    return ret;
}

/*
 * @function:memory_init
 * @description:初始化自分配内存，锁的初始化
 * @calls:list_init				list_insert
 *				get_free_block	pthread_mutex_init
 *
 */
int
memory_init()
{
	int i;
	memory.block_count = BLOCK_COUNT;
	memory.block_size = BLOCK_SIZE;
	memory.used_size = 0;
	memory.full_block.block = NULL;
	list_init(&(memory.full_block.lst));
	memory.free_block.block = NULL;
	list_init(&(memory.free_block.lst));
	for (i = 0; i < memory.block_count; ++i)
	{
		void *block = malloc(memory.block_size);
		struct block_list *block_node_ptr =
			(struct block_list *)malloc(sizeof(struct block_list));
		block_node_ptr->block = block;
		list_insert(&(memory.free_block.lst), &(block_node_ptr->lst));
	}

	memory.current_block = get_free_block ();
	memory.begin = memory.current_block->block;

	pthread_mutex_init(&alloc_mutex, NULL);

	return 0;
}

/*
 * @function:memory_destory
 * @description:销毁内存，释放锁
 * @calls:free	list_remove pthread_mutex_destroy
 *
 */
void
memory_destory()
{
	struct block_list *release = NULL;
	/*
	 * 程序执行完毕，已使用块都应当为空，所以只销毁空闲块和当前块
	 *
	 */
	struct block_list *blck_node = &(memory.free_block);
	list_for_each_entry(blck_node, &(memory.free_block.lst), lst)
	{
		free(blck_node->block);
		blck_node->block = NULL;
		release = blck_node;
		list_remove(&(blck_node->lst));
		blck_node = list_pre(blck_node, lst);
		free(release);
		release = NULL;
	}

	free(memory.current_block->block);
	free(memory.current_block);
	memory.current_block = NULL;

	pthread_mutex_destroy(&alloc_mutex);
}

/*
 * @function:release_full_block
 * @description:释放一块已写入磁盘的内存
 * @calls:list_remove	list_insert
 *
 */
int
release_full_block()
{
	struct block_list *wait_release;
	wait_release = list_head(&(memory.full_block.lst), struct block_list, lst);
	list_remove (&(wait_release->lst));
	list_insert (&(memory.free_block.lst), &(wait_release->lst));

	return 0;
}

/*
 * @function:update_current_block
 * @description:将当前块添加到full列表
 *				从free中取出一个作为current
 * @calls:list_insert	get_free_block
 * @called by:is_overflow_if_append
 *
 */
void
update_current_block()
{
	memory.used_size = 0;
	*((char *)memory.begin) = '\0';
	list_insert(&(memory.full_block.lst), &(memory.current_block->lst));
	memory.current_block = get_free_block ();
}

/*
 * @function:is_overflow_if_append
 * @description:判断如果追加，块是否溢出，若溢出则处理
 * @param:  size->请求的内存大小
 * @return:	0->不溢出
 *					1->溢出
 * @calls:update_current_block
 * @called by:memory_alloc
 *
 */
static int
is_overflow_if_append(unsigned size)
{
	int ret = 0;

	memory.used_size += size;
	if(memory.used_size + 1 > memory.block_size)
	{
		ret = 1;
		update_current_block ();
		memory.begin = memory.current_block->block;
	}

	return ret;
}

/*
 * @function:memory_alloc
 * @description:成功返回分配好的地址
 *				失败返回NULL
 * @calls:pthread_mutex_lock	is_overflow_if_append
 *				pthread_mutex_unlock
 *
 */
void *
memory_alloc(unsigned size)
{
	void *ret;

	pthread_mutex_lock(&alloc_mutex);

	ret = memory.begin;
	if(!is_overflow_if_append (size))
	{
		memory.begin = (void *)((char *)memory.begin + size);
	}
	else
	{
		ret = NULL;
	}

	pthread_mutex_unlock(&alloc_mutex);

    return ret;
}
