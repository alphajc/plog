# plog
[TOC]
## 库说明
在我们的系统中，可能很多时候需要打印日志每次写一个日志模块，比较麻烦，也犯不着。写个来放在这儿。我这款日志系统可以实现异步打印。主要分为两个模块：<br/>
>1.内存管理<br/>
>2.日志管理<br/>

日志管理中使用了内存管理的接口，依赖于内存管理。我在内存管理的模块中维护了几块内存，每块内存作为一个页面。写日志时将其缓存在页面中，当有页面写满的时候进行通知
## 库使用
#### 接口
```c
int libplog_init ();
int libplog_destrory ();
int libplog_write (const char *file_name, const char *func_name,
		const int line, log_level_t level, char *msg, ...);

/*
 * @description:调用libplog_write(),传入调试级别码
 */
#define PLOG_DEBUG(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, DEBUG,  msg, ##args)

/*
 * @description:调用libplog_write(),传入提示级别码
 */
#define PLOG_INFO(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, INFO,  msg, ##args)

/*
 * @description:调用libplog_write(),传入警告级别码
 */
#define PLOG_WARN(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, WARN,  msg, ##args)

/*
 * @description:调用libplog_write(),传入错误级别码
 */
#define PLOG_ERROR(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, ERROR,  msg, ##args)

/*
 * @description:调用libplog_write(),传入崩溃级别码
 */
#define PLOG_FATAL(msg, args...) \
		libplog_write(__FILE__, __FUNCTION__, __LINE__, FATAL,  msg, ##args)

```
使用起来也特别简单，程序启动后写日志前，必须进行初始化即`libplog_init`操作，然后中间根据需要调用各个宏，也就是间接地调用`libplog_write`函数，最后，使用完后记得销毁，不然会有内存泄漏。
#### 编译
将`libplog.a`放在某个路径下，当前目录的加`-L.`，我这里是放在当前目录下的`lib`中，所以加`-L./lib`。然后在后面添加库名`-lplog`。最后不要忘了加上`-lpthread`，因为我在库中用了多线程。参考指令如下：
```bash
$ gcc main.c -I./include -L./lib -lplog -lpthread
```
