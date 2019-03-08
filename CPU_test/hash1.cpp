/*	һά����ɢ�У�������������룩 2/26
	��һ������ά����ɢ��
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
	Bulk **dirPrimary; //ÿ��Ͱ��ĵ�ַ
	Page **overflowAddr;
	int *splitLevel;  //��ά ���Ϊ����
	int *splitIndex;
	int *overflowIndex;  //���Ͱ��������һ��������Ŀ�Ͱ
	int primaryLen;	//��ǰĿ¼����ĳ���
	int *overfloweLen; //�ѷ������Ͱ����
	int *recordNumber;  //ͳ��ÿһ����ϣͰ���еļ�¼����
	int *preinsertCount; //ÿ��Ͱ����Ҫ�����Ͱ��
	int *sumOverflowBuck; //�ܵ����Ͱ��
	int recoverBucket;
	int nextSplit;//��һ��Ҫ���ѵ�Ͱ��

	MTH_table() {
		dirPrimary = (Bulk **)malloc(N * sizeof(Bulk *));
		dirPrimary[0] = (Bulk *)malloc(sizeof(Bulk));
		//�������䣿��
		Bulk *new_bulk = (Bulk *)malloc(sizeof(Bulk));//�·����Ͱ��
		
		overflowAddr = (Page **)malloc(N * sizeof(Page *));
		for (int i = 0; i < N; i++) {
			overflowAddr[i] = (Page *)malloc(N * sizeof(Page)); //����N������ΪN�����Ͱ
		}
		
	}
	int code(int *k, int i);
	int mapping(int *h);
	bool trigger(int k);
	void expand() {
		//��ʱÿ��ֻ����һ��Ͱ��  GPUʱ��Ҫ�Ľ�
		//int split_num = (new_rec + cur_rec) / (facA*capacity) - primaryLen;
		Bulk *new_bulk = (Bulk *)malloc(sizeof(Bulk));
		primaryLen++;
		dirPrimary[primaryLen] = new_bulk;
		//recordNumber,preinsertCount����ռ�
	}
	void split() {
		//����nextSplit;
	}
	void insert(int val, int *key) {
		int *hash_code;
		for (int i = 0;i < D; i++) {
			hash_code[i] = code(key, i);
		}
		int map_code = mapping(hash_code);//����ϣ������ֵӳ�䵽һά
		if (trigger(map_code)) {  //������ͻ
			expand();
			split();
		}
		if (map_code < nextSplit) {

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
};
struct Hash_table {
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

};

int main()
{
	Hash_table h;
	for (int i = 0; i < 80; i++)
	{
		int key = rand() % 100;
		printf("��new��key = %d\n", key);
		h.insert(i, key);
	}
	h.display();
	return 0;
}