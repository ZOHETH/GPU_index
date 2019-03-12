/*	多维维线性散列（仅含构造与插入） 3/12
	下一步：完善并移植到cuda*/


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
		key = NULL;
		value = -1;
		next = NULL;
	}
};
struct Page {
	Record *record;
	Record *head;
};
struct Bulk {
	Page pages[N];		//N个桶页一个桶块 方便管理
};

struct MTH_table {
	Bulk **dirPrimary; //每个桶块地址的数组
	Page **overflowAddr; //暂时未用到
	int splitLevel;  // 总分裂层数
	int splitIndex; //要分裂的桶号
	int *overflowIndex;  //溢出桶数组中下一个待分配的空桶（暂时未用到）
	int primaryLen;	//当前目录数组的长度（暂时未用到）
	int *overfloweLen; //已分配溢出桶总数（暂时未用到）
	int *recordNumber;  //统计每一个哈希桶链中的记录总数
	int *preinsertCount; //每个桶链需要的溢出桶数（暂时未用到）
	int *sumOverflowBuck; //总的溢出桶数（暂时未用到）
	int recoverBucket;//（暂时未用到）

	MTH_table() {
		dirPrimary = (Bulk **)malloc(sizeof(Bulk *));//数组分配一个位置
		dirPrimary[0] = (Bulk *)malloc(sizeof(Bulk));//分配一个桶块
		//分配桶链head
		for (int i = 0; i < N; i++) {
			dirPrimary[0]->pages[i].head= 
			dirPrimary[0]->pages[i].record = new Record;		
		}
		
		/*
		overflowAddr = (Page **)malloc(N * sizeof(Page *));
		for (int i = 0; i < N; i++) {
			overflowAddr[i] = (Page *)malloc(sizeof(Page)); //分配N个溢出桶
		}*/
		
		//为记录变量分配空间
		recordNumber = (int *)malloc(N * sizeof(int));
		memset(recordNumber, 0, N * sizeof(int));
		
		splitLevel = 0;
		splitIndex = 0;
		primaryLen = 0;
	}
	int mapping(int *h) {	//映射到一维空间
		int result = 0;
		for (int i = 0; i < D; i++) {
			int copy = h[i];
			for (int j = 0; copy > 0; j++) {
				int temp = copy & 1;	//取二进制位
				temp <<= (i + j * D);
				result += temp;
				copy >>= 1;
			}
		}
		return result;
	}
	int code(int *k) {	//计算哈希函数值并映射到一维
		int L= splitLevel;
		int hash_code[D];
		int result;
		//int l = splitLevel / D;
		for (int i = 0; i < D; i++) {
			int level=i<L%D?floor(L/D)+1:floor( L / D); //可以做一个变量来储存？每一维的分裂层数
			hash_code[i] = k[i] % (LENGTH << level);
		}
		result=mapping(hash_code);
		if (result < splitIndex) {	//启用下一层哈希函数（直接计算起一维映射）
			result + (N << splitLevel);
		}
		return result;
	}
	bool trigger(int k) {	//桶链长度超过3则触发冲突
		return recordNumber[k] > 3;
	}
	/*void* assign(void *src) {   //分配新空间 （没用了 可能之后有用）
		void *dst = malloc(sizeof(int)*(N << (splitLevel+1)));
		memset(dst, 0, (N << (splitLevel + 1)) * sizeof(int));
		memcpy(dst, src, sizeof(int)*(N << splitLevel ));
		//free(src);
		return dst;
	}*/
	void expand() {
		//暂时每次只扩展一倍  GPU时需要改进
		//int split_num = (new_rec + cur_rec) / (facA*capacity) - primaryLen;
		Bulk **temp1 = dirPrimary;
		Bulk **new_bulk = (Bulk **)malloc(sizeof(Bulk*)*(1 << splitLevel+1)); //分配新空间
		memcpy(new_bulk, dirPrimary, sizeof(Bulk*)*(1 << splitLevel));
		free(temp1);	//释放原来空间
		dirPrimary = new_bulk;
		
		for (int i = 1<<splitLevel; i < 1<<splitLevel+1; i++) {
			dirPrimary[i] = (Bulk *)malloc(sizeof(Bulk));
			for (int j = 0; j < N; j++) {
				dirPrimary[i]->pages[j].head =
				dirPrimary[i]->pages[j].record = new Record;
				//调试语句：printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				//printf("dir[%d]->pages[%d]  = %d\n", i, j, dirPrimary[i]->pages[j].record->value);
			}
		}

		//为recordNumber,（preinsertCount）分配空间
		//recordNumber=(int*) assign(recordNumber);
		int *new_r = (int *)malloc(sizeof(int)*(N << (splitLevel + 1)));
		int *temp2 = recordNumber;
		memset(new_r, 0, (N << (splitLevel + 1)) * sizeof(int));
		memcpy(new_r, recordNumber, sizeof(int)*(N << splitLevel));
		recordNumber = new_r;
		free(temp2);
		//preinsertCount=(int*) assign(preinsertCount);

	}
	void split() {
		//分裂splitIndex;
		int index = splitIndex;
		splitIndex++;
		int j = 0; //
		int dir_index = index / N;
		int page_index = index % N ;
		int new_index = index + (N << splitLevel);
		printf("i=%d\tj=%d\n", dir_index, page_index);
		Record *the_rec = dirPrimary[dir_index]->pages[page_index].head->next;	//要分裂的桶链的第一个记录
		Record *new_rec = dirPrimary[dir_index + (1<<splitLevel)]->pages[page_index].head;	//对应的新位置桶链的head
		Record *pre_rec = dirPrimary[dir_index]->pages[page_index].head;	//要重新计算的记录 的前一个记录
		for (int i = 0; i < recordNumber[index]; i++)//分裂
		{
			if (the_rec == NULL)
				break;	//防止出错 应该删掉
			int the_v = the_rec->value;
			//int *the_k = the_rec->key;
			int *the_k = (int *)malloc(D * sizeof(int));
			memcpy(the_k, the_rec->key, D * sizeof(int));		//获取键值对到the_k,the_v
			
			int map_code = code(the_k);//计算新值  注意：此时splitIndex已经自增

			if (map_code % (N << (splitLevel + 1)) > index) {	//添加到新增的槽 去除空位
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
			//expand();
			splitLevel++;
		}

		int map_code = code(key);
		printf("#h1=%d \t #h2=%d\n", hash_code[0], hash_code[1]);
		printf("*map=%d\t *L=%d\n", map_code , splitLevel);
		printf("+++++Index=%d\n", splitIndex);
		printf("i = %d\tj = %d\n", map_code / N , map_code % N );//调试语句

		Page the_page = dirPrimary[map_code / N]->pages[map_code% N];
		
		//插入新记录（放到head后）
		Record *temp = the_page.head->next;
		the_page.head->next = new Record(key, val);
		the_page.head->next->next = temp;
		recordNumber[map_code]++;
		
		printf("%d\n", map_code);
		if (trigger(map_code)) {  //触发冲突
			if (splitIndex == 0) { //新轮回的第一次 即扩展哈希表
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
//原一维相关结构 暂作保留
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