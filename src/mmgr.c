#include "mmgr.h"
#include "list.h"
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>

#define BLOCK_SIZE  (1<<12) /* 块大小 */
#define BLOCK_COUNT   5 /* 初始块数 */

struct block_node {
	void		*block;
	struct list  lst;
};

/* 全局唯一的内存分配实例 */
static struct {
    struct list 		free_list;		/* 空闲内存块链表 */
	struct list 		full_list;		/* 已使用内存块链表 */
	/**********************add 2016.11.24**************************/
    struct list			writing_list;	/* 正在写盘的链表 */
	/**********************add 2016.11.24**************************/
	struct block_node*	current_block;	/* 当前使用块 */
    unsigned     block_size;			/* 每个内存块的大小 */
    unsigned     block_count;			/* 已申请内存块的总数 */
    unsigned     used_size;				/* 当前块已使用大小 */
    void        *begin;					/* 当前缓存块的当前地址 */
} memory;

/* 先用互斥锁，以后修改为更合适的锁 */
static pthread_mutex_t alloc_mutex;
/**************add 2016.11.24**************/
/* 锁full_list */
static pthread_mutex_t g_full_mutex;
/* 锁free_list */
static pthread_mutex_t g_free_mutex;
/* 条件变量（通告写盘） */
static  pthread_cond_t g_write_cond;
/**************add 2016.11.24**************/

static int release_writing_list();

/*
 * @function:get_free_block
 * @description:获取一个空闲的块
 * @calls:list_empty	malloc
 *				list_insert	list_remove
 * @called by:memory_init update_current_block
 * @author:Gavin
 *
 */
static struct block_node *
get_free_block()
{
	int i;
    struct block_node *free;
	/* 如果已经没有了空闲的块，那么重新分配 */
	if (list_empty (&(memory.free_list)))
	{
		/* 再分配BLOCK_COUNT个内存块 */
		memory.block_count += BLOCK_COUNT;
		for (i = 0; i < BLOCK_COUNT; ++i)
		{
			void *block = malloc(memory.block_size);
			struct block_node *block_node_ptr =
				(struct block_node *)malloc(sizeof(struct block_node));
			block_node_ptr->block = block;

			pthread_mutex_lock(&g_free_mutex);
			list_insert(&(memory.free_list), &(block_node_ptr->lst));
			pthread_mutex_unlock(&g_free_mutex);
		}
	}

	pthread_mutex_lock(&g_free_mutex);
	free = list_head(&(memory.free_list), struct block_node, lst);
	list_remove (&(free->lst));
	pthread_mutex_unlock(&g_free_mutex);

    return free;
}

/*
 * @function:get_block_string
 * @description:获取一个写满的块
 * @calls:list_empty
 * @author:Gavin
 * @update: 2016.11.24 Gavin
 *
 */
char *
get_block_string(int * finished)
{
	char *ret = NULL;
	static struct list *current_node = NULL;
	if (NULL == current_node || current_node == &memory.writing_list)
	{
        release_writing_list();
		/* 等待通知从full取list，操作full */
		pthread_mutex_lock(&g_full_mutex);
		while (list_empty(&memory.full_list))
		{
 			pthread_cond_wait(&g_write_cond, &g_full_mutex);
		}
		/* 将full迁移到writing */
        if (list_empty(&memory.writing_list))
		{
			list_trans(&memory.writing_list, &memory.full_list);
		}

		pthread_mutex_unlock(&g_full_mutex);

		current_node = memory.writing_list.next;
        *finished = 0;
	}
	ret = (char *)(list_entry(current_node, struct block_node, lst)->block);
	current_node = current_node->next;
	if (current_node == &memory.writing_list)
	{
		*finished = 1;
	}

    return ret;
}

/*
 * @function:memory_init
 * @description:初始化自分配内存，锁的初始化
 * @calls:list_init				list_insert
 *				get_free_block pthread_mutex_init
 * @author:Gavin
 * @update: 2016.11.24 Gavin
 *
 */
int
memory_init()
{
	int i;
	memory.block_count = BLOCK_COUNT;
	memory.block_size = BLOCK_SIZE;
	memory.used_size = 0;
	list_init(&(memory.full_list));
	list_init(&(memory.free_list));
	/*******************add 2016.11.24******************/
	list_init(&(memory.writing_list));
	/********************add 2016.11.24*****************/
	for (i = 0; i < memory.block_count; ++i)
	{
		void *block = malloc(memory.block_size);
		struct block_node *block_node_ptr =
			(struct block_node *)malloc(sizeof(struct block_node));
		block_node_ptr->block = block;
		list_insert(&(memory.free_list), &(block_node_ptr->lst));
	}

	memory.current_block = get_free_block ();
	memory.begin = memory.current_block->block;

	pthread_mutex_init(&alloc_mutex, NULL);
	/***************add 2016.11.24*******************/
	pthread_mutex_init(&g_full_mutex, NULL);
	pthread_mutex_init(&g_free_mutex, NULL);
	pthread_cond_init(&g_write_cond, NULL);
	/***************add 2016.11.24*******************/

	return 0;
}

/*
 * @function:memory_destroy
 * @description:销毁内存，释放锁
 * @calls:free	list_remove pthread_mutex_destroy
 * @author:Gavin
 *
 */
void
memory_destroy()
{
	struct block_node *release = NULL;
	/*
	 * 程序执行完毕，已使用块都应当为空，所以只销毁空闲块和当前块
	 *
	 */
	struct block_node *blck_node = NULL;
	list_for_each_entry(blck_node, &(memory.free_list), lst)
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
 * @function: release_writing_list
 * @description:释放正在写的内存
 * @author:Gavin
 *
 */
static int
release_writing_list()
{
	//判断writing_list是否为空
	if (list_empty(&memory.writing_list))
		return 0;
	pthread_mutex_lock(&g_free_mutex);
	list_connect(&memory.free_list, &memory.writing_list);
	pthread_mutex_unlock(&g_free_mutex);

	return 0;
}

/*
 * @function:update_current_block
 * @description:将当前块添加到full列表
 *				从free中取出一个作为current
 * @calls:list_insert	get_free_block
 * @called by:is_overflow_if_append
 * @author:Gavin
 * @update: 2016.11.24 Gavin
 *
 */
void
update_current_block()
{
	memory.used_size = 0;
	*((char *)memory.begin) = '\0';

	pthread_mutex_lock(&g_full_mutex);
	list_insert(&(memory.full_list), &(memory.current_block->lst));
	/* 通知writing_list取full_list */
	pthread_cond_signal(&g_write_cond);
	pthread_mutex_unlock(&g_full_mutex);

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
 * @author:Gavin
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
		memory.used_size += size;
	}

	return ret;
}

/*
 * @function:memory_alloc
 * @description:成功返回分配好的地址
 *				失败返回NULL
 * @calls:pthread_mutex_lock	is_overflow_if_append
 *				pthread_mutex_unlock
 * @author:Gavin
 * @update: 2016.11.24 Gavin
 *
 */
void *
memory_alloc(unsigned size)
{
	void *ret;

	pthread_mutex_lock(&alloc_mutex);

	ret = memory.begin;
	if(is_overflow_if_append (size))
	{
        ret = memory.begin;
	}
	memory.begin = (void *)((char *)memory.begin + size);

	pthread_mutex_unlock(&alloc_mutex);

    return ret;
}
