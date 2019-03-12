/*	��άά����ɢ�У�������������룩 3/12
	��һ�������Ʋ���ֲ��cuda*/


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
	Page pages[N];		//N��Ͱҳһ��Ͱ�� �������
};

struct MTH_table {
	Bulk **dirPrimary; //ÿ��Ͱ���ַ������
	Page **overflowAddr; //��ʱδ�õ�
	int splitLevel;  // �ܷ��Ѳ���
	int splitIndex; //Ҫ���ѵ�Ͱ��
	int *overflowIndex;  //���Ͱ��������һ��������Ŀ�Ͱ����ʱδ�õ���
	int primaryLen;	//��ǰĿ¼����ĳ��ȣ���ʱδ�õ���
	int *overfloweLen; //�ѷ������Ͱ��������ʱδ�õ���
	int *recordNumber;  //ͳ��ÿһ����ϣͰ���еļ�¼����
	int *preinsertCount; //ÿ��Ͱ����Ҫ�����Ͱ������ʱδ�õ���
	int *sumOverflowBuck; //�ܵ����Ͱ������ʱδ�õ���
	int recoverBucket;//����ʱδ�õ���

	MTH_table() {
		dirPrimary = (Bulk **)malloc(sizeof(Bulk *));//�������һ��λ��
		dirPrimary[0] = (Bulk *)malloc(sizeof(Bulk));//����һ��Ͱ��
		//����Ͱ��head
		for (int i = 0; i < N; i++) {
			dirPrimary[0]->pages[i].head= 
			dirPrimary[0]->pages[i].record = new Record;		
		}
		
		/*
		overflowAddr = (Page **)malloc(N * sizeof(Page *));
		for (int i = 0; i < N; i++) {
			overflowAddr[i] = (Page *)malloc(sizeof(Page)); //����N�����Ͱ
		}*/
		
		//Ϊ��¼��������ռ�
		recordNumber = (int *)malloc(N * sizeof(int));
		memset(recordNumber, 0, N * sizeof(int));
		
		splitLevel = 0;
		splitIndex = 0;
		primaryLen = 0;
	}
	int mapping(int *h) {	//ӳ�䵽һά�ռ�
		int result = 0;
		for (int i = 0; i < D; i++) {
			int copy = h[i];
			for (int j = 0; copy > 0; j++) {
				int temp = copy & 1;	//ȡ������λ
				temp <<= (i + j * D);
				result += temp;
				copy >>= 1;
			}
		}
		return result;
	}
	int code(int *k) {	//�����ϣ����ֵ��ӳ�䵽һά
		int L= splitLevel;
		int hash_code[D];
		int result;
		//int l = splitLevel / D;
		for (int i = 0; i < D; i++) {
			int level=i<L%D?floor(L/D)+1:floor( L / D); //������һ�����������棿ÿһά�ķ��Ѳ���
			hash_code[i] = k[i] % (LENGTH << level);
		}
		result=mapping(hash_code);
		if (result < splitIndex) {	//������һ���ϣ������ֱ�Ӽ�����һάӳ�䣩
			result + (N << splitLevel);
		}
		return result;
	}
	bool trigger(int k) {	//Ͱ�����ȳ���3�򴥷���ͻ
		return recordNumber[k] > 3;
	}
	/*void* assign(void *src) {   //�����¿ռ� ��û���� ����֮�����ã�
		void *dst = malloc(sizeof(int)*(N << (splitLevel+1)));
		memset(dst, 0, (N << (splitLevel + 1)) * sizeof(int));
		memcpy(dst, src, sizeof(int)*(N << splitLevel ));
		//free(src);
		return dst;
	}*/
	void expand() {
		//��ʱÿ��ֻ��չһ��  GPUʱ��Ҫ�Ľ�
		//int split_num = (new_rec + cur_rec) / (facA*capacity) - primaryLen;
		Bulk **temp1 = dirPrimary;
		Bulk **new_bulk = (Bulk **)malloc(sizeof(Bulk*)*(1 << splitLevel+1)); //�����¿ռ�
		memcpy(new_bulk, dirPrimary, sizeof(Bulk*)*(1 << splitLevel));
		free(temp1);	//�ͷ�ԭ���ռ�
		dirPrimary = new_bulk;
		
		for (int i = 1<<splitLevel; i < 1<<splitLevel+1; i++) {
			dirPrimary[i] = (Bulk *)malloc(sizeof(Bulk));
			for (int j = 0; j < N; j++) {
				dirPrimary[i]->pages[j].head =
				dirPrimary[i]->pages[j].record = new Record;
				//������䣺printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				//printf("dir[%d]->pages[%d]  = %d\n", i, j, dirPrimary[i]->pages[j].record->value);
			}
		}

		//ΪrecordNumber,��preinsertCount������ռ�
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
		//����splitIndex;
		int index = splitIndex;
		splitIndex++;
		int j = 0; //
		int dir_index = index / N;
		int page_index = index % N ;
		int new_index = index + (N << splitLevel);
		printf("i=%d\tj=%d\n", dir_index, page_index);
		Record *the_rec = dirPrimary[dir_index]->pages[page_index].head->next;	//Ҫ���ѵ�Ͱ���ĵ�һ����¼
		Record *new_rec = dirPrimary[dir_index + (1<<splitLevel)]->pages[page_index].head;	//��Ӧ����λ��Ͱ����head
		Record *pre_rec = dirPrimary[dir_index]->pages[page_index].head;	//Ҫ���¼���ļ�¼ ��ǰһ����¼
		for (int i = 0; i < recordNumber[index]; i++)//����
		{
			if (the_rec == NULL)
				break;	//��ֹ���� Ӧ��ɾ��
			int the_v = the_rec->value;
			//int *the_k = the_rec->key;
			int *the_k = (int *)malloc(D * sizeof(int));
			memcpy(the_k, the_rec->key, D * sizeof(int));		//��ȡ��ֵ�Ե�the_k,the_v
			
			int map_code = code(the_k);//������ֵ  ע�⣺��ʱsplitIndex�Ѿ�����

			if (map_code % (N << (splitLevel + 1)) > index) {	//��ӵ������Ĳ� ȥ����λ
				Record *temp = new Record(the_k, the_v);
				new_rec->next = temp;
				new_rec = new_rec->next;
				recordNumber[new_index]++;
				//�����λ   !!!!δ�����ͷ������
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
		if (splitIndex==(N<<splitLevel)) { //һ�ַ��������
			splitIndex = 0;
			//expand();
			splitLevel++;
		}

		int map_code = code(key);
		printf("#h1=%d \t #h2=%d\n", hash_code[0], hash_code[1]);
		printf("*map=%d\t *L=%d\n", map_code , splitLevel);
		printf("+++++Index=%d\n", splitIndex);
		printf("i = %d\tj = %d\n", map_code / N , map_code % N );//�������

		Page the_page = dirPrimary[map_code / N]->pages[map_code% N];
		
		//�����¼�¼���ŵ�head��
		Record *temp = the_page.head->next;
		the_page.head->next = new Record(key, val);
		the_page.head->next->next = temp;
		recordNumber[map_code]++;
		
		printf("%d\n", map_code);
		if (trigger(map_code)) {  //������ͻ
			if (splitIndex == 0) { //���ֻصĵ�һ�� ����չ��ϣ��
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
//ԭһά��ؽṹ ��������
/*struct Hash_table {
	Page **arr; //Ͱҳ���飨�洢���ݣ�
	Page **o_arr;//���Ͱ
	//int N;
	int L;		//���Ѳ���
	int size;				//
	int capacity;			//�������������£�
	int *pages_size;
	int cur_position;

	Hash_table() {
		//N = 7;
		arr = (Page **)malloc(N * sizeof(Page*));
		for (int i = 0; i < N; i++)
		{
			arr[i] = (Page *)malloc((PAGE_SIZE + 10) * sizeof(Page));//�˴��������Ͱҳ��������Ϊ10 ��������⣨����ʮ��֮��
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
	bool overf(int the_code) {  //��Ͱ�����
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


		if (overf(the_code)) {	//�������
			//�������Ͱ
			size++;
			if (cur_position == 0)   //��ѭ����һ�γ�ͻ ��capacity����
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
			for (int i = 0; i < pages_size[cur_position]; i++)//����
			{
				int the_v = arr[cur_position][i].value;
				int the_k = arr[cur_position][i].key;		//��ȡ��ֵ��

				int new_code = cur_position + (N << L);
				if (the_k % (N << (L + 1)) > cur_position) {	//��ӵ������Ĳ� ��λ��-1��
					arr[new_code][pages_size[new_code]].key = the_k;
					arr[new_code][pages_size[new_code]++].value = the_v;
					arr[cur_position][i].key = -1;
				}
				j = i - 1;	//����ԭ���Ĳ۵Ŀ�λ
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
		printf("��new��key1 = %d key2 = %d\n", key[0],key[1]);
		h.insert(i, key);
	}
	h.display();
	return 0;
}