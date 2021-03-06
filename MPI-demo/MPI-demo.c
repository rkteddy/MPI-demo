#include "mpi.h"
#include "string.h"
#include "sys/stat.h"
#include "memory.h"
#include "io.h"
#include "stdio.h"
#include "stdlib.h"
#include "process.h"
#include "direct.h"

#define uchar unsigned char
#define uint unsigned int

//消息类型标签
#define DICT_SIZE_MSG 0    // 字典大小标签
#define FILE_NAME_MSG 1    // 文档名字标签
#define VECTOR_MSG 2    // 特征向量标签
#define EMPTY_MSG 3    // 空消息标签

//参数类型标签
#define DIR_ARG 1    // 路径参数
#define DICT_ARG 2    // 字典参数
#define RES_ARG 3    // 结果参数

#define HASH_SIZE 100

// 链地址法解决哈希码冲突

int matrix[5][5] = { 0 };

typedef struct _node
{
	char *data;    // 哈希节点所存字符串
	int num;	// 表示在字典中的序号
	struct _node *next;    // 链表的下个节点
}node;

typedef struct _hash_table
{
	node* value;	// 构成哈希表
}hash_table;

// 哈希函数
int hash_func(char *string)
{
	int len = strlen(string);
	int sum = 0;
	int h = 0;
	int a = 0;
	char *ptr = string;
	while (ptr - string < len)
	{
		a = *(ptr++)*(ptr - string);
		sum += sum^a;
		h += a;
	}
	return (((sum) << 16) | h) % HASH_SIZE;
}

// 插入一个字符串到哈希表中
void hash_insert(char *string, hash_table *hash_tbl, int num)
{
	int key_value = hash_func(string);
	node *nde = &(hash_tbl->value[key_value]);

	// 搜索链表直到节点为空
	while (nde->data != NULL)
	{
		nde = nde->next;
		printf("Position was used, turn to next.\n");
	}

	// 插入字符串
	nde->data = (char*)malloc(strlen(string) * sizeof(char));
	strcpy_s(nde->data, strlen(string) + 1, string);
	nde->num = num;
	nde->next = (node*)malloc(sizeof(node));

	// 置下一个节点为空
	nde->next->data = NULL;
	nde->next->next = NULL;
	printf("\"%s\" Insert success! Key value: %d\n", string, key_value);
}

// 根据缓冲区数据建立哈希表
void build_hash_table(char *buffer, hash_table *hash_tbl, int *words_cnt)
{
	char *string = (char*)malloc(50 * sizeof(char));
	char *ptr;
	int i = 0;
	*words_cnt = 0;
	ptr = buffer;

	// 以空格为分界 以换行符为结尾读取字符串并插入哈希表中
	while (*ptr != '\0')
	{
		i = 0;

		string = (char*)malloc(50 * sizeof(char));
		memset(string, 0, 50);
		while (*ptr != ' ')
		{
			string[i] = *ptr;
			i++;
			ptr++;
		}
		*words_cnt += 1;
		hash_insert(string, hash_tbl, *words_cnt);
		ptr++;
		if (string != NULL)
		{
			free(string);
			string = NULL;
		}
	}

	// 返回单词总数
	printf("End building hash table.\n");
}

// 生成并初始化哈希表 返回指向哈希表的指针
hash_table *init_hash_table()
{
	int i;
	hash_table *hash_tbl = (hash_table*)malloc(sizeof(hash_table*));
	hash_tbl->value = (node*)malloc(HASH_SIZE * sizeof(node));

	for (i = 0; i < HASH_SIZE; i++)
	{
		hash_tbl->value[i].data = NULL;
		hash_tbl->value[i].next = NULL;
	}

	return hash_tbl;
}

// 查找某个字符串是否存在哈希表中
char hash_exist(hash_table hash_tbl, char *string)
{
	int key_value = hash_func(string);
	node *nde = &(hash_tbl.value[key_value]);

	// 如果关键字处的链表第一个节点便为空 则不存在
	if (nde->data == NULL)
		return 0;

	// 与链表中的一个个节点进行比较
	else while (nde->next != NULL)
	{
		if (strcmp(nde->data, string) == 0)
			return nde->num;
		nde = nde->next;
	}

	// 找不到返回0
	return 0;
}

// 得到某路径下的所有txt文件名
void get_names(char *path, char ***file_name, int *cnt)
{
	int i;
	int count = 0;
	char **name = *file_name;
	long file; 
	struct _finddata_t find;

	_chdir(path);
	file = _findfirst("*.txt", &find);    // 得到第一个txt文件句柄
	if (file == -1)    // 不存在txt文件报错
	{
		printf("Nothing in this directory!\n");
		exit(0);
	}
	else    // 读取第一个txt文件的名字并写入file_name
	{
		name[count] = (char*)malloc(strlen(find.name) + strlen(path) + 1);
		snprintf(name[count], (strlen(find.name) + strlen(path) + 1), "%s%s", path, find.name);
		count++;
	}
	while (_findnext(file, &find) == 0)    // 继续搜索
	{
		name[count] = (char*)malloc(strlen(find.name) + strlen(path) + 1);
		snprintf(name[count], (strlen(find.name) + strlen(path)) + 1, "%s%s", path, find.name);
		count++;
	}

	_findclose(file);

	*cnt = count;
}

// 读取字典txt中的所有单词存在content中并返回字典总大小
void read_dictionary(char *path, char **content, long *len)
{
	char chars;
	int i = 0;
	*len = 0;
	FILE *f;

	int err = fopen_s(&f, path, "r");
	if (err != 0)
	{
		printf("No such file in this directory!\n");
		return;
	}

	// 先搜索一遍得到字典总大小
	printf("Scanning dictionary...\n");
	while ((chars = fgetc(f)) != EOF)
		*len += 1;
	*content = (char*)malloc(*len * sizeof(char));
	fclose(f);

	// 将单词全部存在content中
	err = fopen_s(&f, path, "r");
	printf("Reading dictionary...\n");
	fgets(*content, *len, f);
	printf("Complete!\n");
	fclose(f);
}

// 将文档中的单词与字典单词进行比较得到特征向量
void make_profile(char *path, hash_table hash_tbl, uchar **profile)
{
	char string[50];
	memset(string, 0, 50);
	char chars;
	int num;
	int i = 0;
	FILE *f;
	i = 0;

	int err = fopen_s(&f, path, "r");
	if (err != 0)
	{
		printf("No such file in this directory!\n");
		return;
	}

	while ((chars = fgetc(f)) != EOF)
	{
		if (chars == ',' || chars == '.' || chars == '!' || chars == '\?' || chars == ':' || chars == '\"' || chars == '\'')	// 如果为标点符号则忽略
			continue;
		if (chars == ' ')    //以空格为分界
		{
			printf("%s: %d\n", string, hash_exist(hash_tbl, string));

			if (num = hash_exist(hash_tbl, string))    //如果单词存在于哈希表中则写入特征向量
				(*profile)[num - 1] += 1;

			i = 0;
			memset(string, 0, 50);
		}
		else    // 遇到空格前的字符存在字符串缓冲区里
		{
			string[i] = chars;
			i++;
		}
	}

	fclose(f);
}

// 创建空矩阵并存在vector中
void build_2d_array(int file_cnt, int words_cnt, uchar ***vector)
{
	int i, j;
	*vector = (uchar**)malloc(file_cnt * sizeof(uchar *));

	for (i = 0; i < file_cnt; i++)
	{
		(*vector)[i] = (uchar*)malloc(words_cnt * sizeof(uchar));
		for (j = 0; j < words_cnt; j++)
			(*vector)[i][j] = 0;
	}
}

// 将结果写入文件
void write_profiles(char *path, int file_cnt, int words_cnt, char** file_name, uchar **vector)
{
	int i, j;
	_chdir(path);
	FILE *f;
	int err = fopen_s(&f, "result.txt", "w");    // 创建并打开结果文件

	if (err != 0)    // 无法打开路径
	{
		printf("No such file in this directory!\n");
		return;
	}

	for (i = 0; i < file_cnt; i++)    // 写入矩阵
	{
		fprintf(f, "Document%d :", i);
		for (j = 0; j < words_cnt; j++)
		{
			fprintf(f, "%d ", vector[i][j]);
		}
		fprintf(f, "\n");
	}

	fclose(f);
}

void manager(int argc, char* argv[], int p)
{
	int i, j;
	int words_cnt;
	int assign_cnt;    // 已分配的文档
	int *assigned;    // 分配的文档指针
	uchar *buffer;    // 特征向量
	int dict_size;    // 字典大小
	int file_cnt;    // 文档总数
	char **file_name = (char**)malloc(sizeof(char**));    // 文档名
	MPI_Request pending;    // MPI请求的句柄
	int src;    // 消息的来源
	MPI_Status status;    // 消息状态
	int tag;    // 消息标签
	int terminated;    // 已经完成的工人进程
	uchar **vector;    // 存放特征向量的文档

	printf("\n\nManager:\n");

	MPI_Irecv(&dict_size, 1, MPI_INT, MPI_ANY_SOURCE, DICT_SIZE_MSG, MPI_COMM_WORLD, &pending);    // 即将接收字典大小
	MPI_Irecv(&words_cnt, 1, MPI_INT, MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, &pending);    // 即将接收字典个数
	get_names(argv[DIR_ARG], &file_name, &file_cnt);    // 搜索目录， 并得到所有文档名
	printf("File_cnt: %d\n", file_cnt);
	for (i = 0; i < file_cnt; i++)
		printf("File_name: %s\n", file_name[i]);
	MPI_Wait(&pending, &status);    // 接收消息
	printf("Dict_size: %d\n", dict_size);
	printf("Words_cnt: %d\n", words_cnt);
	buffer = (uchar*)malloc(words_cnt * sizeof(MPI_UNSIGNED_CHAR));    //申请内存空间准备接收特征向量
	build_2d_array(file_cnt, words_cnt, &vector);    // 建立存放特征向量的二维数组
	terminated = 0;
	assign_cnt = 0;
	assigned = (int*)malloc(p * sizeof(int));
	while (terminated < (p - 1))
	{
		MPI_Recv(buffer, dict_size, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);    // 从工人进程接收消息
		src = status.MPI_SOURCE;
		tag = status.MPI_TAG;
		if (tag == VECTOR_MSG)    // 如果是包含特征向量的消息
		{
			for (i = 0; i < words_cnt; i++)
			{
				vector[assigned[src]][i] = buffer[i];    // 写入特征向量 
			}
			printf("\n");
		}

		if (assign_cnt < file_cnt)    // 如果已分配的文档小于文档总数
		{
			MPI_Send(file_name[assign_cnt], strlen(file_name[assign_cnt]) + 1, MPI_CHAR, src, FILE_NAME_MSG, MPI_COMM_WORLD);    // 发送下一个文档名
			printf("File%d: %s\n", assign_cnt, file_name[assign_cnt]);
			assigned[src] = assign_cnt;    // 标记文档
			assign_cnt++;
		}
		else    //如果文档已分配完
		{
			MPI_Send(NULL, 0, MPI_CHAR, src, FILE_NAME_MSG, MPI_COMM_WORLD);    // 发送结束消息
			terminated++;
		}
	}
	write_profiles(argv[RES_ARG], file_cnt, words_cnt, file_name, vector);    // 将特征向量写入文件
	for (i = 0; i < file_cnt; i++)
		free(file_name[i]);

	// 释放内存
	free(file_name);
	free(buffer);
	free(assigned);
}

void worker(int argc, char *argv[], MPI_Comm worker_comm)
{
	int i;
	int words_cnt;
	char *buffer = NULL;     // 字典缓冲区
	hash_table hash_tbl = *init_hash_table();   // 单词的哈希表
	long dict_size;    // 字典大小
	char *name;    // 文档内容
	int name_len;    // 文档大小
	MPI_Request pending;    // 发送的句柄
	uchar *profile;   // 文档的特征向量
	MPI_Status status;    // 通信状态
	int worker_id;    // 进程号

	MPI_Comm_rank(worker_comm, &worker_id);    // 获取当前进程在通信域中的编号
	printf("\n\nWorker %d:\n", worker_id);

	if (!worker_id)
	{
		read_dictionary(argv[DICT_ARG], &buffer, &dict_size);    // 如果为工人0则读取字典
		MPI_Isend(&dict_size, 1, MPI_INT, 0, DICT_SIZE_MSG, MPI_COMM_WORLD, &pending);    // 发送字典大小
		MPI_Wait(&pending, &status);
	}
	MPI_Bcast(&dict_size, 1, MPI_LONG, 0, worker_comm);    // 广播字典大小
	if (worker_id)
		buffer = (char*)malloc(dict_size);    // 如果非工人0则为缓冲区申请内存空间
	MPI_Bcast(buffer, dict_size, MPI_CHAR, 0, worker_comm);    // 得到字典字符串存入缓冲区
	printf("Buffer: %s\n", buffer);
	build_hash_table(buffer, &hash_tbl, &words_cnt);    // 建立哈希表
	if (!worker_id)
	{
		MPI_Isend(&words_cnt, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, &pending);    // 发送字典大小
		MPI_Wait(&pending, &status);
	}
	profile = (uchar*)malloc(words_cnt * sizeof(uchar));
	for (i = 0; i < words_cnt; i++)
		profile[i] = 0;
	MPI_Send(NULL, 0, MPI_UNSIGNED_CHAR, 0, EMPTY_MSG, MPI_COMM_WORLD);    // 发送空消息表示准备就绪

	for (;;)
	{
		MPI_Probe(0, FILE_NAME_MSG, MPI_COMM_WORLD, &status);    // 得到接收的文档消息状态
		MPI_Get_count(&status, MPI_CHAR, &name_len);    // 则从消息状态中获取文档内容长度
		if (!name_len) break;    // 如果没有更多工作则退出循环

		name = (char*)malloc(name_len);    // 为文档内容申请内存
		MPI_Recv(name, name_len, MPI_CHAR, 0, FILE_NAME_MSG, MPI_COMM_WORLD, &status);
		make_profile(name, hash_tbl, &profile);    //得到特征向量
		free(name);    //释放文档内存
		MPI_Send(profile, dict_size, MPI_UNSIGNED_CHAR, 0, VECTOR_MSG, MPI_COMM_WORLD);
		printf("Profile: ");
		for (i = 0; i < words_cnt; i++)
			printf("%d ", profile[i]);
		printf("\n");
	}

	// 释放内存
	free(buffer);
	free(profile);
}

int main(int argc, char *argv[])
{
	int id;
	int p;

	MPI_Comm worker_comm = MPI_COMM_WORLD;
	MPI_Comm manager_comm = MPI_COMM_WORLD;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);    //返回当前进程在通信域中的进程号
	MPI_Comm_size(MPI_COMM_WORLD, &p);    //返回通信域的进程数

	if (argc != 4)    //传入参数错误处理
	{
		if (!id)
		{
			printf("Program needs three arguments:\n");
			printf("%s <dir> <dict> <results> \n", argv[0]);
		}
	}
	else if (p < 2)    //少于两个进程错误处理
	{
		printf("Program needs at least two process\n");
	}
	else    //分离通信域
	{
		if (!id)
		{
			MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, id, &manager_comm);
			manager(argc, argv, p);
		}
		else
		{
			MPI_Comm_split(MPI_COMM_WORLD, 0, id, &worker_comm);
			worker(argc, argv, worker_comm);
		}
	}

	MPI_Finalize();

	return 0;
}
