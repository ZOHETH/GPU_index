/*	多维维线性散列（仅含构造与插入） 3/12
	下一步：完善并移植到cuda
*/


#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#define PAGE_SIZE 3
#define N 4
#define D 2
#define LENGTH 2
using namespace std;

struct Record {
	int *key;
	int value;
	Record *next;
	Record(int *k, int v) {
		key = (int *)malloc(D * sizeof(int));
		memcpy(key, k, D * sizeof(int));
		value = v;
		next = NULL;
	}
	Record()
	{
		//key = -1;
		value = -1;
		next = NULL;
	}
};
struct Page {
	Record *record;
	Record *tail;
	Record *head;
};
struct Bulk {
	Page pages[N];
};

struct MTH_table {
	Bulk **dirPrimary; //每个桶块的地址
	Page **overflowAddr;
	int splitLevel;  // 总分裂层数
	int splitIndex; //要分裂的桶号
	int *overflowIndex;  //溢出桶数组中下一个待分配的空桶
	int primaryLen;	//当前目录数组的长度
	int *overfloweLen; //已分配溢出桶总数
	int *recordNumber;  //统计每一个哈希桶链中的记录总数
	int *preinsertCount; //每个桶链需要的溢出桶数
	int *sumOverflowBuck; //总的溢出桶数
	int recoverBucket;

	MTH_table() {
		dirPrimary = (Bulk **)malloc(sizeof(Bulk *));
		dirPrimary[0] = (Bulk *)malloc(sizeof(Bulk));
		//继续分配？？
		for (int i = 0; i < N; i++) {
			dirPrimary[0]->pages[i].head= 
			dirPrimary[0]->pages[i].tail=
			dirPrimary[0]->pages[i].record = new Record;		
		}
		
		/*
		overflowAddr = (Page **)malloc(N * sizeof(Page *));
		for (int i = 0; i < N; i++) {
			overflowAddr[i] = (Page *)malloc(sizeof(Page)); //分配N个溢出桶
		}*/
		recordNumber = (int *)malloc(N * sizeof(int));
		memset(recordNumber, 0, N * sizeof(int));
		splitLevel = 0;
		splitIndex = 0;
		primaryLen = 0;
	}
	int mapping(int *h) {
		int result = 0;
		for (int i = 0; i < D; i++) {
			int copy = h[i];
			for (int j = 0; copy > 0; j++) {
				int temp = copy & 1;
				temp <<= (i + j * D);
				result += temp;
				copy >>= 1;
			}
		}
		return result;
	}
	int code(int *k) {
		int L= splitLevel;
		int hash_code[D];
		int result;
		//int l = splitLevel / D;
		for (int i = 0; i < D; i++) {
			int level=i<L%D?floor(L/D)+1:floor( L / D); //可以做一个变量来储存
			hash_code[i] = k[i] % (LENGTH << level);
		}
		result=mapping(hash_code);
		if (result < splitIndex) {
			result + (N << splitLevel);
		}
		return result;
	}
	bool trigger(int k) {
		return recordNumber[k] > 3;
	}
	void* assign(void *src) {   //分配新空间 
		void *dst = malloc(sizeof(int)*(N << (splitLevel+1)));
		memset(dst, 0, (N << (splitLevel + 1)) * sizeof(int));
		memcpy(dst, src, sizeof(int)*(N << splitLevel ));
		//free(src);
		return dst;
	}
	void expand() {		//index对应问题？？？？？？？？？？？
		//暂时每次只增加一个桶块  GPU时需要改进
		//int split_num = (new_rec + cur_rec) / (facA*capacity) - primaryLen;
		Bulk **new_bulk = (Bulk **)malloc(sizeof(Bulk*)*(1 << splitLevel+1));
		memcpy(new_bulk, dirPrimary, sizeof(Bulk*)*(1 << splitLevel)); //有问题
		dirPrimary = new_bulk;
		for (int i = 1<<splitLevel; i < 1<<splitLevel+1; i++) {
			dirPrimary[i] = (Bulk *)malloc(sizeof(Bulk));
			for (int j = 0; j < N; j++) {
				dirPrimary[i]->pages[j].head =
				dirPrimary[i]->pages[j].tail =
				dirPrimary[i]->pages[j].record = new Record;
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				printf("dir[%d]->pages[%d]  = %d\n", i, j, dirPrimary[i]->pages[j].record->value);
			}
		}

		//recordNumber,preinsertCount分配空间
		//recordNumber=(int*) assign(recordNumber);
		int *new_r = (int *)malloc(sizeof(int)*(N << (splitLevel + 1)));
		int *temp = recordNumber;
		memset(new_r, 0, (N << (splitLevel + 1)) * sizeof(int));
		memcpy(new_r, recordNumber, sizeof(int)*(N << splitLevel));
		recordNumber = new_r;
		free(temp);
		//preinsertCount=(int*) assign(preinsertCount);
	}
	void split() {     //index对应问题？？？？？？？？？？？
		//分裂splitIndex;
		int index = splitIndex;
		splitIndex++;
		int j = 0; //
		int dir_index = index / N;
		int page_index = index % N ;
		int new_index = index + (N << splitLevel);
		printf("i=%d\tj=%d\n", dir_index, page_index);
		Record *the_rec = dirPrimary[dir_index]->pages[page_index].head->next;
		Record *new_rec = dirPrimary[dir_index + (1<<splitLevel)]->pages[page_index].head;
		Record *pre_rec = dirPrimary[dir_index]->pages[page_index].head;
		Record *temp;
		for (int i = 0; i < recordNumber[index]; i++)//分裂
		{
			if (the_rec == NULL)
				break;
			int the_v = the_rec->value;
			//int *the_k = the_rec->key;
			int *the_k = (int *)malloc(D * sizeof(int));
			memcpy(the_k, the_rec->key, D * sizeof(int));		//获取键值对
			
			int map_code = code(the_k);
			if (map_code % (N << (splitLevel + 1)) > index) {	//添加到新增的槽 空位用-1记
				Record *temp = new Record(the_k, the_v);
				new_rec->next = temp;
				new_rec = new_rec->next;
				recordNumber[new_index]++;
				//补齐空位   !!!!未解决“头”问题
				temp=the_rec;
				pre_rec->next = the_rec->next;
				the_rec = the_rec->next;
				free(temp);
				recordNumber[index]--;
			}
			else {
				pre_rec = the_rec;
				the_rec = the_rec->next;
			}
			
		}
	}

	
	void insert(int val, int *key) {
		int hash_code[D];
		if (splitIndex==(N<<splitLevel)) { //一轮分裂已完成
			splitIndex = 0;
			expand();
			splitLevel++;
		}
		/*for (int i = 0; i < D; i++) {
			hash_code[i] = code(key, i,false);
		}
		int map_code = mapping(hash_code);//讲哈希函数的值映射到一维
		if (map_code < splitIndex) {
			for (int i = 0; i < D; i++) {
				hash_code[i] = code(key, i, true);
			}
			int map_code = mapping(hash_code);
		}*/
		int map_code = code(key);
		printf("#h1=%d \t #h2=%d\n", hash_code[0], hash_code[1]);
		printf("*map=%d\t *L=%d\n", map_code , splitLevel);
		printf("+++++Index=%d\n", splitIndex);
		printf("i = %d\tj = %d\n", map_code / N , map_code % N );
		Page the_page = dirPrimary[map_code / N]->pages[map_code% N];
		
		Record *temp = the_page.head->next;
		the_page.head->next = new Record(key, val);
		the_page.head->next->next = temp;
		recordNumber[map_code]++;
		printf("%d\n", map_code);
		if (trigger(map_code)) {  //触发冲突
			if (splitIndex == 0) {
				expand();
			}
			split();

		}
	}
	void display() {
		for (int i = 0; i < 100; i++) {
			printf("[%d]: key= ", i);
			Record *the_record = dirPrimary[i / N ]->pages[i%N].head->next;
			if (the_record == NULL) {
				printf("\n");
				continue;
			}
			else {for (int j = 0; j < recordNumber[i]; j++) {				
				printf("[%d,%d], ", the_record->key[0],the_record->key[1]);
				the_record = the_record->next;
			}

			}
				
			
				
			printf("\n");
		}
	}
};
/*struct Hash_table {
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

};*/

int main()
{
	MTH_table h;
	for (int i = 0; i < 300; i++)
	{
		int key[2];
		key[0] = rand() % 1000;
		key[1] = rand() % 1000;
		printf("【new】key1 = %d key2 = %d\n", key[0],key[1]);
		h.insert(i, key);
	}
	h.display();
	return 0;
}