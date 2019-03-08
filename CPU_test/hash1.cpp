/*	一维线性散列（仅含构造与插入） 2/26
	下一步：多维线性散列
*/


#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#define PAGE_SIZE 3
#define N 7
#define D 3
using namespace std;

struct Page {
	int key;
	int value;
};
struct Bulk {
	Page pages[N];
};

struct MTH_table {
	Bulk **dirPrimary; //每个桶块的地址
	Page **overflowAddr;
	int *splitLevel;  //多维 因此为数组
	int *splitIndex;
	int *overflowIndex;  //溢出桶数组中下一个待分配的空桶
	int primaryLen;	//当前目录数组的长度
	int *overfloweLen; //已分配溢出桶总数
	int *recordNumber;  //统计每一个哈希桶链中的记录总数
	int *preinsertCount; //每个桶链需要的溢出桶数
	int *sumOverflowBuck; //总的溢出桶数
	int recoverBucket;
	int nextSplit;//下一个要分裂的桶号

	MTH_table() {
		dirPrimary = (Bulk **)malloc(N * sizeof(Bulk *));
		dirPrimary[0] = (Bulk *)malloc(sizeof(Bulk));
		//继续分配？？
		Bulk *new_bulk = (Bulk *)malloc(sizeof(Bulk));//新分配的桶块
		
		overflowAddr = (Page **)malloc(N * sizeof(Page *));
		for (int i = 0; i < N; i++) {
			overflowAddr[i] = (Page *)malloc(N * sizeof(Page)); //分配N个长度为N的溢出桶
		}
		
	}
	int code(int *k, int i);
	int mapping(int *h);
	bool trigger(int k);
	void expand() {
		//暂时每次只增加一个桶块  GPU时需要改进
		//int split_num = (new_rec + cur_rec) / (facA*capacity) - primaryLen;
		Bulk *new_bulk = (Bulk *)malloc(sizeof(Bulk));
		primaryLen++;
		dirPrimary[primaryLen] = new_bulk;
		//recordNumber,preinsertCount分配空间
	}
	void split() {
		//分裂nextSplit;
	}
	void insert(int val, int *key) {
		int *hash_code;
		for (int i = 0;i < D; i++) {
			hash_code[i] = code(key, i);
		}
		int map_code = mapping(hash_code);//讲哈希函数的值映射到一维
		if (trigger(map_code)) {  //触发冲突
			expand();
			split();
		}
		if (map_code < nextSplit) {

		}
		arr[the_code][pages_size[the_code]].key = key;
		arr[the_code][pages_size[the_code]++].value = val;


		if (overf(the_code)) {	//触发溢出
			//放入溢出桶
			size++;
			if (cur_position == 0)   //新循环第一次冲突 则capacity翻倍
			{
				int *new_size = (int *)malloc((N << (L + 1)) * sizeof(int));
				memset(new_size, 0, (N << (L + 1)) * sizeof(int));
				Page **new_arr = (Page **)malloc(N << (L + 1) * sizeof(Page*));
				for (int i = 0; i < (N << (L + 1)); i++)
				{
					new_arr[i] = (Page *)malloc((PAGE_SIZE + 10) * sizeof(Page));

				}

				memcpy(new_arr, arr, (N << L) * sizeof(Page*));
				memcpy(new_size, pages_size, (N << L) * sizeof(int));
				arr = new_arr;
				pages_size = new_size;
				this->display();

			}
			int j = 0; //
			for (int i = 0; i < pages_size[cur_position]; i++)//分裂
			{
				int the_v = arr[cur_position][i].value;
				int the_k = arr[cur_position][i].key;		//获取键值对

				int new_code = cur_position + (N << L);
				if (the_k % (N << (L + 1)) > cur_position) {	//添加到新增的槽 空位用-1记
					arr[new_code][pages_size[new_code]].key = the_k;
					arr[new_code][pages_size[new_code]++].value = the_v;
					arr[cur_position][i].key = -1;
				}
				j = i - 1;	//处理原来的槽的空位
				while ((0 <= j) && arr[cur_position][j].key == -1)
					j--;
				if (j >= -1 && j != i - 1) {
					arr[cur_position][j + 1].key = arr[cur_position][i].key;
					arr[cur_position][j + 1].value = arr[cur_position][i].value;
					arr[cur_position][i].key = -1;
				}
			}
			pages_size[cur_position] = j + 1;
			cur_position++;
		}
	}
};
struct Hash_table {
	Page **arr; //桶页数组（存储数据）
	Page **o_arr;//溢出桶
	//int N;
	int L;		//分裂层数
	int size;				//
	int capacity;			//容量（翻倍更新）
	int *pages_size;
	int cur_position;

	Hash_table() {
		//N = 7;
		arr = (Page **)malloc(N * sizeof(Page*));
		for (int i = 0; i < N; i++)
		{
			arr[i] = (Page *)malloc((PAGE_SIZE + 10) * sizeof(Page));//此处即将溢出桶页个数设置为10 会出现问题（多于十个之后）
		}
		pages_size = (int *)malloc(N * sizeof(int));
		memset(pages_size, 0, N * sizeof(int));
		L = 0;
		size = N;
		cur_position = 0;
	}
	int code(int key) {
		return key % (N << L);
	}
	bool overf(int the_code) {  //该桶溢出？
		if (pages_size[the_code] > 3)
			return true;
		return false;
	}

	void insert(int val, int key) {
		int the_code = code(key);
		if (cur_position == (N << L))
		{
			cur_position = 0;
			L++;
		}
		arr[the_code][pages_size[the_code]].key = key;
		arr[the_code][pages_size[the_code]++].value = val;


		if (overf(the_code)) {	//触发溢出
			//放入溢出桶
			size++;
			if (cur_position == 0)   //新循环第一次冲突 则capacity翻倍
			{
				int *new_size = (int *)malloc((N << (L + 1)) * sizeof(int));
				memset(new_size, 0, (N << (L + 1)) * sizeof(int));
				Page **new_arr = (Page **)malloc(N << (L + 1) * sizeof(Page*));
				for (int i = 0; i < (N << (L + 1)); i++)
				{
					new_arr[i] = (Page *)malloc((PAGE_SIZE + 10) * sizeof(Page));

				}

				memcpy(new_arr, arr, (N << L) * sizeof(Page*));
				memcpy(new_size, pages_size, (N << L) * sizeof(int));
				arr = new_arr;
				pages_size = new_size;
				this->display();

			}
			int j = 0; //
			for (int i = 0; i < pages_size[cur_position]; i++)//分裂
			{
				int the_v = arr[cur_position][i].value;
				int the_k = arr[cur_position][i].key;		//获取键值对

				int new_code = cur_position + (N << L);
				if (the_k % (N << (L + 1)) > cur_position) {	//添加到新增的槽 空位用-1记
					arr[new_code][pages_size[new_code]].key = the_k;
					arr[new_code][pages_size[new_code]++].value = the_v;
					arr[cur_position][i].key = -1;
				}
				j = i - 1;	//处理原来的槽的空位
				while ((0 <= j) && arr[cur_position][j].key == -1)
					j--;
				if (j >= -1 && j != i - 1) {
					arr[cur_position][j + 1].key = arr[cur_position][i].key;
					arr[cur_position][j + 1].value = arr[cur_position][i].value;
					arr[cur_position][i].key = -1;
				}
			}
			pages_size[cur_position] = j + 1;
			cur_position++;
		}
	}
	void display() {
		for (int i = 0; i < size; i++) {
			printf("[%d]: key= ", i);
			for (int j = 0; j < pages_size[i]; j++)
				printf("%d, ", arr[i][j].key);
			printf("\n");
		}
	}

};

int main()
{
	Hash_table h;
	for (int i = 0; i < 80; i++)
	{
		int key = rand() % 100;
		printf("【new】key = %d\n", key);
		h.insert(i, key);
	}
	h.display();
	return 0;
}