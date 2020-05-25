# include <stdio.h>
# include <fcntl.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>

# define MAX_LINE 80
# define READ_END 0
# define WRITE_END 1

// Clear the string.
void clear_str(char *str);

// Check whether the instruction is concurrent.
int check_concurrent(char *inst);

// Standardlize the instruction.
void standardlize_inst(char *inst);

// Parse the instruction
int parse(char *inst, char **args);

// Debug program for parsing.
void debug_parse(char *args[], int argn);

int main(void) {
	// arguments, instruction, last instruction
	char *args[MAX_LINE / 2 + 1], *inst, *last_inst;
	// whether have the last instruction, cocurrent status
	int have_last_inst = 0, concurrent = 0;
	// input redirect filename, output redirect filename
	char *in_file, *out_file;

	inst = (char*) malloc(MAX_LINE * sizeof(char));
	last_inst = (char*) malloc(MAX_LINE * sizeof(char));
	in_file = (char*) malloc(MAX_LINE * sizeof(char));
	out_file = (char*) malloc(MAX_LINE * sizeof(char));
  
	clear_str(last_inst);
  
	int should_run = 1;
  
	pid_t pid;

	while(should_run) {
		printf("osh> ");
		fflush(stdout);
		if (concurrent) wait(NULL);

		concurrent = 0;
		clear_str(inst);
    
		fgets(inst, MAX_LINE, stdin);
    
		standardlize_inst(inst);
		concurrent = check_concurrent(inst);

		// exit shell	  
		if (strcmp(inst, "exit") == 0) {
			should_run = 0;
			continue;
		}

		// execute the last instruction
		if (strcmp(inst, "!!") == 0) {
			if (have_last_inst == 0) {
				fprintf(stderr, "Error: No commands in history.\n");
				continue;
			} else {
				printf("%s\n", last_inst);
				strcpy(inst, last_inst);
			}
		}
    
		pid = fork();
		if (pid < 0) fprintf(stderr, "Error: Fork failed!\n");
		else {
			if (pid == 0) {
				// child process 
				// whether an error has occured
				int error_occur = 0;

				// allocate space for commands & arguments
				for (int i = 0; i <= MAX_LINE / 2; ++ i) 
				args[i] = (char*) malloc(MAX_LINE * sizeof(char));
				
				// parse the instruction
				int argn = parse(inst, args);

				// free the space of extra commands & extra arguments
				for (int i = argn; i <= MAX_LINE / 2; ++ i) {
					free(args[i]);
					args[i] = NULL;
				}
				if (concurrent == 1) {
					-- argn;
					free(args[argn]);
					args[argn] = NULL;
				}
		
				// find pipe
				int pipe_pos = -1;
				for (int i = 0; i < argn; ++ i)
					if (strcmp(args[i], "|") == 0) {
						pipe_pos = i;
						break;
					}

				if(pipe_pos >= 0) {										
					// pipe found
					if (pipe_pos == 0 || pipe_pos == argn - 1) {
						fprintf(stderr, "Error: Unexpected syntax '|'.\n");
						error_occur = 1;
					}
													
					// pipe fd
					int pipe_fd[2];			

					if (pipe(pipe_fd) == -1) {
						fprintf(stderr, "Error: Pipe Failed!\n");
						error_occur = 1;
					}
					
					if(error_occur == 0) {
						// fork a grandson process					
						pid = fork();
						if (pid < 0) {
							fprintf(stderr, "Error: Fork failed!\n");
							error_occur = 1;
						} else {
							if (pid == 0) {
								// grandchild process
								for (int i = pipe_pos; i < argn; ++ i) {
									free(args[i]);
									args[i] = NULL;
								}
								argn = pipe_pos;

								close(pipe_fd[READ_END]);
								if (error_occur == 0 && dup2(pipe_fd[WRITE_END], STDOUT_FILENO) < 0) {
									fprintf(stderr, "Error: dup2 failed!\n");
									error_occur = 1;
								}
							
								if(error_occur == 0 && argn > 0) execvp(args[0], args);
								
								close(pipe_fd[WRITE_END]);
							
								// free the spaces
								for (int i = 0; i < argn; ++ i) free(args[i]);
								free(inst); 
								free(last_inst); 
								free(in_file);
								free(out_file); 
								
								exit(error_occur);
							} else {
								// child process
								wait(NULL);
								for (int i = 0; i <= pipe_pos; ++ i) free(args[i]);
								for (int i = pipe_pos + 1; i < argn; ++ i) args[i - pipe_pos - 1] = args[i];
								for (int i = argn - pipe_pos - 1; i < argn; ++ i) args[i] = NULL;
								argn = argn - pipe_pos - 1;

								close(pipe_fd[WRITE_END]);
								if (error_occur == 0 && dup2(pipe_fd[READ_END], STDIN_FILENO) < 0) {
									fprintf(stderr, "Error: dup2 failed!\n");
									error_occur = 1;
								}
		
								if(error_occur == 0 && argn > 0) execvp(args[0], args);
								
								close(pipe_fd[READ_END]);
							}
						}
					}
				} else {
					// find in_redirect or out_redirect
					int in_redirect = 0, out_redirect = 0, in_fd = -1, out_fd = -1;
					while (argn >= 2 && (strcmp(args[argn - 2], "<") == 0 || strcmp(args[argn - 2], ">") == 0)) {
						argn -= 2;
						if (strcmp(args[argn], "<") == 0) {
							in_redirect = 1;
							strcpy(in_file, args[argn + 1]);
						} else {
							out_redirect = 1;
							strcpy(out_file, args[argn + 1]);
						}
						free(args[argn]); args[argn] = NULL;
						free(args[argn + 1]); args[argn + 1] = NULL;
					}
					
					// redirect input
					if (error_occur == 0 && in_redirect == 1) {
						in_fd = open(in_file, O_RDONLY, 0644);
						if (error_occur == 0 && in_fd < 0) {
							fprintf(stderr, "Error: No such files.\n");
							error_occur = 1;
						}
						if (error_occur == 0 && dup2(in_fd, STDIN_FILENO) < 0) {
							fprintf(stderr, "Error: dup2 failed!\n");
							error_occur = 1;
						}
					}
					
					// redirect output
					if (error_occur == 0 && out_redirect == 1) {
						out_fd = open(out_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
						if (error_occur == 0 && out_fd < 0) {
							fprintf(stderr, "Error: No such files.\n");
							error_occur = 1;
						}
						if (error_occur == 0 && dup2(out_fd, STDOUT_FILENO) < 0) {
							fprintf(stderr, "Error: dup2 failed!\n");
							error_occur = 1;
						}
					}
				
					// not an empty instruction & no error occur, then execute the instruction
					if (error_occur == 0 && argn != 0)	
						execvp(args[0], args);
					
					// close the files
					if (in_redirect == 1 && in_fd > 0) close(in_fd);
					if (out_redirect == 1 && out_fd > 0) close(out_fd);
				}

				// free the spaces
				for (int i = 0; i < argn; ++ i) free(args[i]);
				free(inst); 
				free(last_inst); 
				free(in_file);
				free(out_file); 
	
				// child process exit
				exit(error_occur);
			} else {
				// parent process
				if(concurrent == 0) wait(NULL);
			}
		}

		if(have_last_inst == 0) have_last_inst = 1;
		strcpy(last_inst, inst);
	}
	
	// free the spaces
	free(inst); 
	free(last_inst); 
	free(in_file); 
	free(out_file); 
	return 0;
}


// Clear the string.
void clear_str(char *str) {
	memset(str, 0, sizeof(str));
}

// Check whether the instruction is concurrent.
int check_concurrent(char *inst) {
	int len = strlen(inst);
	if(len && inst[len - 1] == '&') return 1;
	return 0;
}

// Standardlize the instruction.
// Specific function: clear the extra space & tab & enter in the instruction.
void standardlize_inst(char *inst) {
	int len = strlen(inst);

	char *temp = (char*) malloc(len * sizeof(char));
	for (int i = 0; i < len; ++ i) temp[i] = inst[i];
	clear_str(inst);
  
	int new_len = 0, last_blank = 1;
	for (int i = 0; i < len; ++ i) {
		if (temp[i] == ' ' || temp[i] == '\n' || temp[i] == '\t') {
			if (last_blank == 0) {
				inst[new_len ++] = ' ';
				last_blank = 1;
			}
		} else {
			inst[new_len ++] = temp[i];
			last_blank = 0;
		}
	}
	if(inst[new_len - 1] == ' ') inst[new_len - 1] = 0;
  
	free(temp);
}

// Parse the instruction.
// Specific function: parse the instruction and find out the command & arguments.
int parse(char *inst, char **args) {
	int len = strlen(inst);

	// find out the arguments
	int argn = 0;
	for (int i = 0; i < len; ++ i) {
		clear_str(args[argn]);

		int j = i;
		while(j < len && inst[j] != ' ') {
			args[argn][j - i] = inst[j];
			++ j;
		}
		if ((args[argn][0] == '<' || args[argn][0] == '>' || args[argn][0] == '|') && j > i+1) {
			strcpy(args[argn + 1], args[argn] + 1);
			for (int k = 1; k < j - i; ++ k) args[argn][k] = 0;
			++ argn;		
		}
		
		i = j;
		++ argn;
	}

	return argn;
}

// Debug program for parsing.
void debug_parse(char *args[], int argn) {
	fprintf(stderr, "Comm: %s, total %d arguments\n", args[0], argn);
	for (int i = 0; i < argn; ++ i)
		fprintf(stderr, "args[%d] = %s\n", i, args[i]);
}

