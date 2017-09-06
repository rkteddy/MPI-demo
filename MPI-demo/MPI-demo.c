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

int main(int argc, char *argv[])
{
}
