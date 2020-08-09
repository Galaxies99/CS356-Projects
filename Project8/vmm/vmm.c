# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define PAGE_NUM 256
# define PAGE_SIZE 256
# define FRAME_NUM 128
# define FRAME_SIZE 256
# define TLB_SIZE 16

// ================= Empty Frame List ================== //
struct empty_frame_list_node {
	int frame_num;
	struct empty_frame_list_node *nxt;
};

struct empty_frame_list_node *head = NULL;
struct empty_frame_list_node *tail = NULL;

// Add the empty frame to the empty frame list.
void add_empty_frame(int frame_num) {
	if (head == NULL && tail == NULL) {
		tail = (struct empty_frame_list_node *) malloc (sizeof(struct empty_frame_list_node));
		tail -> frame_num = frame_num;
		tail -> nxt = NULL;
		head = tail;
	} else {
		tail -> nxt = (struct empty_frame_list_node *) malloc (sizeof(struct empty_frame_list_node));
		tail -> nxt -> frame_num = frame_num;
		tail -> nxt -> nxt = NULL;
		tail = tail -> nxt;
	}
}

// Get an empty frame from the empty frame list.
// If success, return frame_num; otherwise, return -1.
int get_empty_frame() {
	if (head == NULL && tail == NULL) return -1;

	int frame_num;
	if (head == tail) {
		frame_num = head -> frame_num;
		free(head);
		head = tail = NULL;
		return frame_num;
	}

	struct empty_frame_list_node *tmp;	
	frame_num = head -> frame_num;
	tmp = head;
	head = head -> nxt;
	free(tmp);
	return frame_num;
}

// Initialize the empty frame list.
void initialize_empty_frame_list() {
	for (int i = 0; i < FRAME_NUM; ++ i)
		add_empty_frame(i);
}

// Clean the empty frame list.
void clean_empty_frame_list() {
	if (head == NULL && tail == NULL) return;
	struct empty_frame_list_node *tmp;
	while (head != tail) {
		tmp = head;
		head = head -> nxt;
		free(tmp);
	}
	free(head);
	head = tail = NULL;
}
// ============== End of Empty Frame List ============== //


// ====================== Memory ======================= //
char memory[FRAME_NUM * FRAME_SIZE];
int frame_LRU[FRAME_NUM];
char buf[FRAME_SIZE];
FILE *fp_backing_store;

void initialize_memory() {
	fp_backing_store = fopen("BACKING_STORE.bin", "rb");
	if (fp_backing_store == NULL) {
		fprintf(stderr, "[Err] Open backing store file error!\n");
		exit(1);
	}
	initialize_empty_frame_list();
	for (int i = 0; i < FRAME_NUM; ++ i)
		frame_LRU[i] = 0;
}

void delete_page_table_item(int frame_num);
int add_page_into_memory(int page_num) {
	fseek(fp_backing_store, page_num * FRAME_SIZE, SEEK_SET);
  fread(buf, sizeof(char), FRAME_SIZE, fp_backing_store);
	
	int frame_num = get_empty_frame();
	if (frame_num == -1) {
		// LRU replacement.
		for (int i = 0; i < FRAME_NUM; ++ i)
			if (frame_LRU[i] == FRAME_NUM) {
				frame_num = i;
				break;
			}
		delete_page_table_item(frame_num);
	}

	for (int i = 0; i < FRAME_SIZE; ++ i)
		memory[frame_num * FRAME_SIZE + i] = buf[i];
	for (int i = 0; i < FRAME_NUM; ++ i)
		if (frame_LRU[i] > 0) ++ frame_LRU[i];
	frame_LRU[frame_num] = 1;
	return frame_num;
}

char access_memory(int frame_num, int offset) {
	char res = memory[frame_num * FRAME_SIZE + offset];
	for (int i = 0; i < FRAME_NUM; ++ i)
		if (frame_LRU[i] > 0 && frame_LRU[i] < frame_LRU[frame_num])
			++ frame_LRU[i];
	frame_LRU[frame_num] = 1;
	return res;
}

void clean_memory() {
	clean_empty_frame_list();
	fclose(fp_backing_store);
}
// =================== End of Memory =================== //


// ======================== TLB ======================== //
int TLB_page[TLB_SIZE], TLB_frame[TLB_SIZE];
int TLB_LRU[TLB_SIZE];
int TLB_hit_count;

void initialize_TLB() {
	TLB_hit_count = 0;
	for (int i = 0; i < TLB_SIZE; ++ i) {
		TLB_page[i] = 0;
		TLB_frame[i] = 0;
		TLB_LRU[i] = 0;
	}
}

// Get the corresponding frame number from TLB.
//   Return non-negative number for the corresponding frame number;
//   Return -1 for TLB miss.
//   Note: it's needless to check the validation of page_num again.
int get_TLB_frame_num(int page_num) {
	int pos = -1;
	for (int i = 0; i < TLB_SIZE; ++ i)
		if (TLB_LRU[i] > 0 && TLB_page[i] == page_num) {
			pos = i;
			break;
		}

	if (pos == -1) return -1;
	
	// TLB hit.
	++ TLB_hit_count;
	for (int i = 0; i < TLB_SIZE; ++ i)
		if (TLB_LRU[i] > 0 && TLB_LRU[i] < TLB_LRU[pos])
			++ TLB_LRU[i];
	TLB_LRU[pos] = 1;
	return TLB_frame[pos];
}

// Update TLB entry
void update_TLB(int page_num, int frame_num) {
	int pos = -1;
	for (int i = 0; i < TLB_SIZE; ++ i)
		if(TLB_LRU[i] == 0) {
			pos = i;
			break;
		}
	
	if (pos == -1) {
		// LRU replacement.
		for (int i = 0; i < TLB_SIZE; ++ i)
			if(TLB_LRU[i] == TLB_SIZE) {
				pos = i;
				break;
			}
	}
	
	TLB_page[pos] = page_num;
	TLB_frame[pos] = frame_num;
	for (int i = 0; i < TLB_SIZE; ++ i)
		if (TLB_LRU[i] > 0) ++ TLB_LRU[i];
	TLB_LRU[pos] = 1;
}

// Delete TLB item.
void delete_TLB_item(int page_num, int frame_num) {
	int pos = -1;
	for (int i = 0; i < TLB_SIZE; ++ i)
		if(TLB_LRU[i] && TLB_page[i] == page_num && TLB_frame[i] == frame_num) {
			pos = i;
			break;
		}
	
	if (pos == -1) return;
	
	for (int i = 0; i < TLB_SIZE; ++ i)
		if (TLB_LRU[i] > TLB_LRU[pos]) -- TLB_LRU[i];
	TLB_LRU[pos] = 0;
}
// ===================== End of TLB ==================== //


// ==================== Page Table ===================== //
int page_table[PAGE_NUM];
int vi_page_table[PAGE_NUM];  // vi: valid-invalid
int page_fault_count;

void initialize_page_table() {
	page_fault_count = 0;
	for (int i = 0; i < PAGE_NUM; ++ i) {
		page_table[i] = 0;
		vi_page_table[i] = 0;
	}
}

// Get the corresponding frame number.
//   Return non-negative number for the corresponding frame number;
//   Return -1 for invalid page number.
int get_frame_num(int page_num) {
	if (page_num < 0 || page_num >= PAGE_NUM) return -1;
	
	int TLB_res = get_TLB_frame_num(page_num);
	if (TLB_res != -1) return TLB_res;

	if (vi_page_table[page_num] == 1) {
		update_TLB(page_num, page_table[page_num]);
		return page_table[page_num];
	} else {
		// Page fault.
		++ page_fault_count;
		page_table[page_num] = add_page_into_memory(page_num);
		vi_page_table[page_num] = 1;
		update_TLB(page_num, page_table[page_num]);
		return page_table[page_num];
	}
}

// Delete page table item
void delete_page_table_item(int frame_num) {
	int page_num = -1;
	for (int i = 0; i < PAGE_NUM; ++ i)
		if(vi_page_table[i] && page_table[i] == frame_num) {
			page_num = i;
			break;
		}
	if (page_num == -1) {
		fprintf(stderr, "[Err] Unexpected Error!\n");
		exit(1);
	}
	vi_page_table[page_num] = 0;
	delete_TLB_item(page_num, frame_num);
}
// ================= End of Page Table ================= //


void initialize() {
	initialize_page_table();
	initialize_TLB();
	initialize_memory();
}

void clean() {
	clean_memory();
}


int main(int argc, char *argv[]) {	
	if (argc != 2) {
		fprintf(stderr, "[Err] Invalid input!\n");
		return 1;
	}

	FILE *fp_in = fopen(argv[1], "r");
	if(fp_in == NULL) {
		fprintf(stderr, "[Err] File Error!\n");
		return 1;
	}
	
	FILE *fp_out = fopen("output.txt", "w");
	if (fp_out == NULL) {
		fprintf(stderr, "[Err] File Error!\n");
		return 1;
	}

	initialize();
		
	int addr, page_num, offset, frame_num, res, cnt = 0;
	while(~fscanf(fp_in, "%d", &addr)) {
		++ cnt;
		addr = addr & 0x0000ffff;
		offset = addr & 0x000000ff;
		page_num = (addr >> 8) & 0x000000ff;
		frame_num = get_frame_num(page_num);
		res = (int) access_memory(frame_num, offset);
		fprintf(fp_out, "Virtual address: %d Physical address: %d Value: %d\n", addr, (frame_num << 8) + offset, res);
	}
	
	fprintf(stdout, "[Statistics]\n  TLB hit rate: %.4f %%\n  Page fault rate: %.4f %%\n", 100.0 * TLB_hit_count / cnt, 100.0 * page_fault_count / cnt);
	
	clean();
	fclose(fp_in);
	fclose(fp_out);
	return 0;
}
