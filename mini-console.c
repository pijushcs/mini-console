#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/wait.h>

#define DEFAULT_NUM_PRINT_LINES 1
#define DEFAULT_BUFFER_SIZE 256

int my_cp(int arg_count, char *args[]) {
	char buf[256];

	if(arg_count < 2) {
		printf("Usage: mycp <file_name>");
		return -1;
	}

	FILE *fp = fopen(args[0] , "r");
	FILE *copy_fp = fopen(args[1], "w");

	if(fp != NULL && copy_fp != NULL) {
		while(fgets(buf, 256, fp) != NULL) 
		fputs(buf, copy_fp);

		fclose(fp);
		fclose(copy_fp);

		return 0;
	}

	return -1;
}

char* my_cat(int is_print_data, int arg_count, char *args[]) {
	char buf[256], *write_buf = malloc(256*sizeof(char));

	if(arg_count < 1) {
		printf("Usage: mycat <file_name>");
		strcpy(write_buf, "\n");
		return write_buf;
	}

	FILE *fp = fopen(args[0] , "r");
	if(fp != NULL) {
		while(fgets(buf, 256, fp) != NULL) {
			if(is_print_data) printf("%s",buf);
			strcat(write_buf, buf);
		}
		fclose(fp);
	} else {
		strcpy(write_buf, "\n");
		printf("Error processing file %s\n", args[0]);
	}

	return write_buf;
}

char* my_head(int is_print_data, int arg_count, char *args[], char *pipe_data) {
	int num_print_lines = DEFAULT_NUM_PRINT_LINES, str_len = 0;
	char buf[256], *write_buf = malloc(256*sizeof(char));

	if(arg_count < 1 && !pipe_data) {
		printf("Usage: myhead <file_name>");
		strcpy(write_buf, "\n");
		return write_buf;
	}

	if(arg_count) {
		FILE *fp = fopen(args[0] , "r");
		if(fp != NULL) {
			while(num_print_lines-- && fgets(buf, 256, fp) != NULL) {
				if(is_print_data) printf("%s",buf);
				strcat(write_buf, buf);
			}
			fclose(fp);
		} else {
			strcpy(write_buf, "\n");
			printf("Error processing file %s\n", args[0]);
		}
	} else {
		while(num_print_lines && pipe_data!='\0') {
			write_buf[str_len++] = *pipe_data;	
			if(is_print_data) printf("%c",*pipe_data++);
			if(*pipe_data == '\n') num_print_lines--;	
		}

		printf("\n");
		write_buf[str_len++] = '\n';
		write_buf[str_len] = '\0';	
	}

	return write_buf;
}

char* my_tail(int is_print_data, int arg_count, char *args[], char *pipe_data) {
	int num_print_lines = DEFAULT_NUM_PRINT_LINES, str_len = 0, total_lines = 0, skip_lines = 0;
	char buf[256], *write_buf = malloc(256*sizeof(char)), *cur_data_ptr;

	if(arg_count < 1 && !pipe_data) {
		printf("Usage: mytail <file_name>");
		strcpy(write_buf, "\n");
		return write_buf;
	}

	if(arg_count) {
		FILE *fp = fopen(args[0] , "r");
		if(fp != NULL) {
			while(fgets(buf, 256, fp) != NULL) total_lines++;

			fseek(fp, 0, SEEK_SET);
			skip_lines = total_lines - num_print_lines;

			while(fgets(buf, 256, fp) != NULL) {
				if(is_print_data && skip_lines <= 0) printf("%s",buf);
				if(skip_lines <= 0) strcat(write_buf, buf);

				skip_lines--;
			}

			fclose(fp);
		} else {
			strcpy(write_buf, "\n");
			printf("Error processing file %s\n", args[0]);
		}
	} else {
		cur_data_ptr = pipe_data;
		while(*cur_data_ptr != '\0') {
			if(*cur_data_ptr++ == '\n') total_lines++;
		}

		skip_lines = total_lines - num_print_lines;

		printf("%d %d %d\n", skip_lines, num_print_lines, total_lines);

		while(pipe_data != '\0') {
			if(skip_lines <= 0 && is_print_data) printf("%c",*pipe_data);
			if(skip_lines <= 0) write_buf[str_len++] = *pipe_data;	
			if(*pipe_data++ == '\n') skip_lines--;
		}

		printf("\n");
		write_buf[str_len++] = '\n';
		write_buf[str_len] = '\0';	
	}

	return write_buf;
}

int my_rm(int arg_count, char *args[]) {
	if(arg_count < 1) {
		printf("Usage: myrm <file_name>");
		return -1;
	}

	remove(args[0]);
	return 0;
}

int my_mv(int arg_count, char *args[]) {
	if(arg_count < 2) {
		printf("Usage: mymv <old_file_name> <new_file_name>");
		return -1;
	}

	rename(args[0], args[1]);
	return 0;
}

char *trim(char *str) {
	int i = 0, trim_index = 0;
	char *trim_str = malloc(strlen(str)+1);

	for(i=0; i<strlen(str); i++) 
		if(str[i]!='\n' && str[i]!=' ') trim_str[trim_index++] = str[i];
	trim_str[trim_index] = '\0';
	return trim_str;
}

int main() {
	char *cmd, buf[256], *write_buf;
	char *cmd_ptr, *args_ptr, *token, *args_token, *args[10];
	int pipe_buf[2], arg_count = 0, pid = 0, wstatus = 0, buf_size=DEFAULT_BUFFER_SIZE;

	cmd = (char*)malloc(256);

	pipe(pipe_buf);
		
	printf("> ");
	getline(&cmd, (size_t*)&buf_size, stdin);

	token = strtok_r(cmd, "|", &cmd_ptr);

	while(strcmp(trim(token), "exit")) {
		while(token) {
			args_token = trim(strtok_r(token, " ", &args_ptr));
			cmd = trim(args_token);

			arg_count = 0;
			while(args_token) {
				args_token = strtok_r(NULL, " ", &args_ptr);
				if(args_token) args[arg_count++] = trim(args_token);
			}

			pid = fork();
			if (pid == 0) {
				if(!strcmp(cmd, "mycp")) 
					my_cp(arg_count, args);
				
				if(!strcmp(cmd, "mymv")) 
					my_mv(arg_count, args);
				
				if(!strcmp(cmd, "myrm")) 
					my_rm(arg_count, args);

				if(!strcmp(cmd, "mycat")) {
					write_buf = my_cat(1, arg_count, args);
					write(pipe_buf[1], write_buf, 256);
				}

				if(!strcmp(cmd, "myhead")) {
					write_buf = my_head(1, arg_count, args, write_buf);
					write(pipe_buf[1], write_buf, strlen(write_buf));
				}

				if(!strcmp(cmd, "mytail")) {
					write_buf = my_tail(1, arg_count, args, write_buf);
					write(pipe_buf[1], write_buf, strlen(write_buf));
				}

				exit(0);
			}

			if(!strcmp(cmd, "mycat") || !strcmp(cmd, "myhead") || !strcmp(cmd, "mytail")) {
				write_buf = malloc(256*sizeof(char));
				read(pipe_buf[0], write_buf, 256);
			}

			waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);

			token = strtok_r(NULL, "|", &cmd_ptr);
		}

		printf("> ");
		getline(&cmd, (size_t*)&buf_size, stdin);

		token = strtok_r(cmd, "|", &cmd_ptr);
	}
	
	return 0;	
}