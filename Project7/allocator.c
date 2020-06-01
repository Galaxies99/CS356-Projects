# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define LINE_MAX 256

int memory_limit;

typedef struct memory_node {
	// belong
	char *bel;
	// begin, end
	int beg, end;
	// next
	struct memory_node * nxt;
} mem_node;

mem_node *head = NULL;


// Requested by process [process_name], size of [size], memory allocation type [type].
// If success, return 0; or return 1 with the error message printed in the screen.
int request(char *process_name, int size, char type) {
	if (size <= 0) {
		fprintf(stderr, "[Err] the process size should be positive!\n");
		return 1;
	}

	int name_len = strlen(process_name);
	if (head == NULL) {
		if (size <= memory_limit) {	
			head = (mem_node *) malloc (sizeof(mem_node));
			head -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(head -> bel, process_name);
			head -> beg = 0; 
			head -> end = 0 + size - 1;
			head -> nxt = NULL;
			return 0;
		} else {
			// No enough spaces
			fprintf(stderr, "[Err] No enough spaces!\n");
			return 1;
		}	
	}
	
	// First-fit
	if (type == 'F') {
		mem_node *p = head;
		int hole_len;

		hole_len = p -> beg - 0;
		if (size <= hole_len) {
			mem_node *tmp = head;
			head = (mem_node *) malloc (sizeof(mem_node));
			head -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(head -> bel, process_name);
			head -> beg = 0; 
			head -> end = 0 + size - 1;
			head -> nxt = tmp;
			return 0;
		}
		
		while (p -> nxt != NULL) {
			hole_len = p -> nxt -> beg - p -> end - 1;
			if (size <= hole_len) {
				mem_node *tmp = p -> nxt;
				p -> nxt = (mem_node *) malloc (sizeof(mem_node));
				p -> nxt -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
				strcpy(p -> nxt -> bel, process_name);
				p -> nxt -> beg = p -> end + 1;
				p -> nxt -> end = p -> nxt -> beg + size - 1;
				p -> nxt -> nxt = tmp;
				return 0;
			}
			p = p -> nxt;
		}
		
		hole_len = memory_limit - p -> end - 1;
		if (size <= hole_len) {
			p -> nxt = (mem_node *) malloc (sizeof(mem_node));
			p -> nxt -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p -> nxt -> bel, process_name);
			p -> nxt -> beg = p -> end + 1;
			p -> nxt -> end = p -> nxt -> beg + size - 1;
			p -> nxt -> nxt = NULL;
			return 0;
		}

		// No enough spaces
		fprintf(stderr, "[Err] No enough spaces!\n");
		return 1;
	}

	// Best-fit
	if (type == 'B') {
		mem_node *p = head;
		int hole_len;
		
		// position
		int cur_min = 2e9;		
		int pos_flag = 0;
		mem_node *pos;
		
		hole_len = p -> beg - 0;
		if (size <= hole_len && hole_len < cur_min) {
			cur_min = hole_len;
			pos_flag = 1;
		}

		while (p -> nxt != NULL) {
			hole_len = p -> nxt -> beg - p -> end - 1;
			if (size <= hole_len && hole_len < cur_min) {
				cur_min = hole_len;
				pos_flag = 2;
				pos = p;
			}
			p = p -> nxt;
		}

		hole_len = memory_limit - p -> end - 1;
		if (size <= hole_len && hole_len < cur_min) {
			cur_min = hole_len;
			pos_flag = 3;
		}
		
		// No enough spaces
		if (pos_flag == 0) {
			fprintf(stderr, "[Err] No enough spaces!\n");
			return 1;
		}
		
		if (pos_flag == 1) {
			mem_node *tmp = head;
			head = (mem_node *) malloc (sizeof(mem_node));
			head -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(head -> bel, process_name);
			head -> beg = 0; 
			head -> end = 0 + size - 1;
			head -> nxt = tmp;
			return 0;
		}

		if (pos_flag == 2) {
			p = pos;
			mem_node *tmp = p -> nxt;
			p -> nxt = (mem_node *) malloc (sizeof(mem_node));
			p -> nxt -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p -> nxt -> bel, process_name);
			p -> nxt -> beg = p -> end + 1;
			p -> nxt -> end = p -> nxt -> beg + size - 1;
			p -> nxt -> nxt = tmp;
			return 0;
		}

		if (pos_flag == 3) {
			p -> nxt = (mem_node *) malloc (sizeof(mem_node));
			p -> nxt -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p -> nxt -> bel, process_name);
			p -> nxt -> beg = p -> end + 1;
			p -> nxt -> end = p -> nxt -> beg + size - 1;
			p -> nxt -> nxt = NULL;
			return 0;
		}
	}
	
	// Worst-fit
	if (type == 'W') {
		mem_node *p = head;
		int hole_len;
		
		// position
		int cur_max = -2e9;		
		int pos_flag = 0;
		mem_node *pos;
		
		hole_len = p -> beg - 0;
		if (size <= hole_len && hole_len > cur_max) {
			cur_max = hole_len;
			pos_flag = 1;
		}

		while (p -> nxt != NULL) {
			hole_len = p -> nxt -> beg - p -> end - 1;
			if (size <= hole_len && hole_len > cur_max) {
				cur_max = hole_len;
				pos_flag = 2;
				pos = p;
			}
			p = p -> nxt;
		}

		hole_len = memory_limit - p -> end - 1;
		if (size <= hole_len && hole_len > cur_max) {
			cur_max = hole_len;
			pos_flag = 3;
		}
		
		// No enough spaces
		if (pos_flag == 0) {
			fprintf(stderr, "[Err] No enough spaces!\n");
			return 1;
		}
		
		if (pos_flag == 1) {
			mem_node *tmp = head;
			head = (mem_node *) malloc (sizeof(mem_node));
			head -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(head -> bel, process_name);
			head -> beg = 0; 
			head -> end = 0 + size - 1;
			head -> nxt = tmp;
			return 0;
		}

		if (pos_flag == 2) {
			p = pos;
			mem_node *tmp = p -> nxt;
			p -> nxt = (mem_node *) malloc (sizeof(mem_node));
			p -> nxt -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p -> nxt -> bel, process_name);
			p -> nxt -> beg = p -> end + 1;
			p -> nxt -> end = p -> nxt -> beg + size - 1;
			p -> nxt -> nxt = tmp;
			return 0;
		}

		if (pos_flag == 3) {
			p -> nxt = (mem_node *) malloc (sizeof(mem_node));
			p -> nxt -> bel = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p -> nxt -> bel, process_name);
			p -> nxt -> beg = p -> end + 1;
			p -> nxt -> end = p -> nxt -> beg + size - 1;
			p -> nxt -> nxt = NULL;
			return 0;
		}
	}
	
	fprintf(stderr, "[Err] Argument Error!\n");
	return 1;
}


// Release the resources allocated to process [process_name].
// If success, return 0; or return 1 with the error message printed in the screen.
int release(char *process_name) {
	if (head == NULL) {
		fprintf(stderr, "[Err] No such process!\n");
		return 1;
	}
	if (strcmp(head -> bel, process_name) == 0) {
		mem_node *tmp = head;
		head = head -> nxt;
		free(tmp -> bel);
		free(tmp);
		return 0;
	}	

	mem_node *p = head;
	while (p -> nxt != NULL) {
		if (strcmp(p -> nxt -> bel, process_name) == 0) {
			mem_node *tmp = p -> nxt;
			p -> nxt = p -> nxt -> nxt;
			free(tmp -> bel);
			free(tmp);
			return 0;
		}
		p = p -> nxt;
	}

	fprintf(stderr, "[Err] No such process!\n");
	return 1;
}


// Compact the holes.
void compact() {
	int cur = 0;
	mem_node *p = head;
	while (p != NULL) {
		int size = p -> end - p -> beg + 1;
		p -> beg = cur;
		p -> end = cur + size - 1;
		cur += size;		
		p = p -> nxt;
	}
	return ;
}


// Display the statistics.
void display() {
	if (head == NULL) {
		fprintf(stdout, "Address [0 : %d] Unused\n", memory_limit - 1);
		return ;
	} else if (head -> beg != 0) {
		fprintf(stdout, "Address [0 : %d] Unused\n", head -> beg - 1);
	}
	
	mem_node *p = head;
	int hole_len;

	while (p -> nxt != NULL) {
		fprintf(stdout, "Address [%d : %d] Process %s\n", p -> beg, p -> end, p -> bel);
		hole_len = p -> nxt -> beg - p -> end - 1;
		if (hole_len > 0) 
			fprintf(stdout, "Address [%d : %d] Unused\n", p -> end + 1, p -> nxt -> beg - 1);
		p = p -> nxt;
	}
	
	fprintf(stdout, "Address [%d : %d] Process %s\n", p -> beg, p -> end, p -> bel);
	hole_len = memory_limit - p -> end - 1;
	if (hole_len > 0)
		fprintf(stdout, "Address [%d : %d] Unused\n", p -> end + 1, memory_limit - 1);
}


// Standardlize input.
void standardlize_input(char *op) {
	char tmp[LINE_MAX];
	int no_need = 1, tmpn = 0;
	
	for (int i = 0; op[i]; ++ i) {
		if (op[i] == ' ' || op[i] == '\t' || op[i] == '\n') {
			if (no_need == 0) {
				no_need = 1;
				tmp[tmpn ++] = ' ';
			}
		} else {
			tmp[tmpn ++] = op[i];
			no_need = 0;
		}
	}

	if (tmpn > 0 && tmp[tmpn - 1] == ' ') tmpn --;
	
	for (int i = 0; i < tmpn; ++ i) op[i] = tmp[i];
	op[tmpn] = 0;
}


int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "[Err] Arguments Error!\n");
		exit(1);
	}

	memory_limit = atoi(argv[1]);

	char op[LINE_MAX], oper[LINE_MAX];
	char process_name[LINE_MAX];
	while(1) {
		for (int i = 0; i < LINE_MAX; ++ i) {
			op[i] = 0; oper[i] = 0; 
			process_name[i] = 0;
		}

		fprintf(stdout, "allocator> ");
		fgets(op, LINE_MAX, stdin);
		
		standardlize_input(op);

		if (strcmp(op, "EXIT") == 0) break;	
	
		if (strcmp(op, "C") == 0) {
			compact();
			continue;
		}

		if (strcmp(op, "STAT") == 0) {
			display();		
			continue;
		}

		// parse the inputs.
		int pos = 0;
		for (; op[pos]; ++ pos)
			if(op[pos] == ' ') break;

		for (int i = 0; i < pos; ++ i)
			oper[i] = op[i];
		oper[pos] = 0;
		
		if (strcmp(oper, "RQ") == 0) {
			if (op[pos] == 0) {
				fprintf(stderr, "[Err] Invalid input!\n");
				continue;
			}
			
			int i;
			for (i = pos + 1; op[i]; ++ i) {
				if (op[i] == ' ') break;
				process_name[i - pos - 1] = op[i];
			}
			
			if (op[i] == 0) {
				fprintf(stderr, "[Err] Invalid input!\n");
				continue;
			}
			
			int invalid_input = 0;
			int size = 0;
			pos = i;
			for (i = pos + 1; op[i]; ++ i) {
				if(op[i] == ' ') break;
				if(op[i] < '0' || op[i] > '9') {
					invalid_input = 1;
					break;
				}
				size = size * 10 + op[i] - '0';
			}
			
			if (invalid_input || op[i] == 0) {
				fprintf(stderr, "[Err] Invalid input!\n");
				continue;
			}

			if ((i - pos - 1) >= 10) {
				fprintf(stderr, "[Err] Size too large!\n");
				continue;
			}

			pos = i;
			for (i = pos + 1; op[i]; ++ i) {
				if (op[i] == ' ') {
					invalid_input = 1;
					break;
				}
			}
			
			if (invalid_input || (i - pos - 1) != 1) {
				fprintf(stderr, "[Err] Invalid input!\n");
				continue;
			}

			request(process_name, size, op[pos + 1]);
		} else if (strcmp(oper, "RL") == 0) {
			if (op[pos] == 0) {
				fprintf(stderr, "[Err] Invalid input!\n");
				continue;
			}

			int invalid_input = 0;
			for (int i = pos + 1; op[i]; ++ i) {
				if (op[i] == ' ') {
					invalid_input = 1;
					break;
				}
				process_name[i - pos - 1] = op[i];
			}
			
			if (invalid_input) fprintf(stderr, "[Err] Invalid input!\n");
			else release(process_name);
		} else 
			fprintf(stderr, "[Err] Invalid input!\n");
	}

	return 0;
}

