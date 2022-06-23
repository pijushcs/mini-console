#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/wait.h>

#define DEFAULT_NUM_PRINT_LINES 2
#define DEFAULT_BUFFER_SIZE 256

void update_history(char *cmd) {
	char buf[20];
	int line_count = 49;

	FILE *tmp_fp = fopen("tmp", "a");
	FILE *fp = fopen("cmd-history" , "r");

	if(tmp_fp != NULL) {
		fputs(cmd, tmp_fp);

		if(fp != NULL) {
			while(line_count-- && fgets(buf, 20, fp) != NULL) 
				fputs(buf, tmp_fp);
			fclose(fp);
		}

		fclose(tmp_fp);
	}

	rename("tmp", "cmd-history");
	remove("tmp");
}

int my_cp(int arg_count, char *args[]) {
	char buf[DEFAULT_BUFFER_SIZE];

	if(arg_count < 2) {
		printf("Usage: mycp <file_name>");
		return -1;
	}

	FILE *fp = fopen(args[0] , "r");
	FILE *copy_fp = fopen(args[1], "w");

	if(fp != NULL && copy_fp != NULL) {
		while(fgets(buf, DEFAULT_BUFFER_SIZE, fp) != NULL) 
		fputs(buf, copy_fp);

		fclose(fp);
		fclose(copy_fp);

		update_history("mycp\n");

		return 0;
	}

	return -1;
}

char* my_cat(int is_print_data, int arg_count, char *args[]) {
	char buf[DEFAULT_BUFFER_SIZE], *write_buf = malloc(DEFAULT_BUFFER_SIZE*sizeof(char));

	if(arg_count < 1) {
		printf("Usage: mycat <file_name>");
		strcpy(write_buf, "\n");
		return write_buf;
	}

	FILE *fp = fopen(args[0] , "r");
	if(fp != NULL) {
		while(fgets(buf, DEFAULT_BUFFER_SIZE, fp) != NULL) {
			if(is_print_data) printf("%s",buf);
			strcat(write_buf, buf);
		}
		fclose(fp);
	} else {
		strcpy(write_buf, "\n");
		printf("Error processing file %s\n", args[0]);
	}

	update_history("mycat\n");

	return write_buf;
}

char* my_head(int is_print_data, int arg_count, char *args[], char *pipe_data) {
	int num_print_lines = DEFAULT_NUM_PRINT_LINES, str_len = 0;
	char buf[DEFAULT_BUFFER_SIZE], *write_buf = malloc(DEFAULT_BUFFER_SIZE*sizeof(char));

	if(arg_count < 1 && !pipe_data) {
		printf("Usage: myhead <file_name>");
		strcpy(write_buf, "\n");
		return write_buf;
	}

	if(arg_count) {
		FILE *fp = fopen(args[0] , "r");
		if(fp != NULL) {
			while(num_print_lines-- && fgets(buf, DEFAULT_BUFFER_SIZE, fp) != NULL) {
				if(is_print_data) printf("%s",buf);
				strcat(write_buf, buf);
			}
			fclose(fp);
		} else {
			strcpy(write_buf, "\n");
			printf("Error processing file %s\n", args[0]);
		}
	} else {
		while(num_print_lines && *pipe_data!='\0') {
			write_buf[str_len++] = *pipe_data;	
			if(is_print_data) printf("%c",*pipe_data);
			if(*pipe_data++ == '\n') num_print_lines--;	
		}

		write_buf[str_len] = '\0';	
	}

	update_history("myhead\n");

	return write_buf;
}

char* my_tail(int is_print_data, int arg_count, char *args[], char *pipe_data) {
	int num_print_lines = DEFAULT_NUM_PRINT_LINES, str_len = 0, total_lines = 0, skip_lines = 0;
	char buf[DEFAULT_BUFFER_SIZE], *write_buf = malloc(DEFAULT_BUFFER_SIZE*sizeof(char)), *cur_data_ptr;

	if(arg_count < 1 && !pipe_data) {
		printf("Usage: mytail <file_name>");
		strcpy(write_buf, "\n");
		return write_buf;
	}

	if(arg_count) {
		FILE *fp = fopen(args[0] , "r");
		if(fp != NULL) {
			while(fgets(buf, DEFAULT_BUFFER_SIZE, fp) != NULL) total_lines++;

			fseek(fp, 0, SEEK_SET);
			skip_lines = total_lines - num_print_lines;

			while(fgets(buf, DEFAULT_BUFFER_SIZE, fp) != NULL) {
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

		while(*pipe_data != '\0') {
			if(skip_lines <= 0 && is_print_data) printf("%c",*pipe_data);
			if(skip_lines <= 0) write_buf[str_len++] = *pipe_data;	
			if(*pipe_data++ == '\n') skip_lines--;
		}

		write_buf[str_len] = '\0';
	}

	update_history("mytail\n");

	return write_buf;
}

int my_rm(int arg_count, char *args[]) {
	if(arg_count < 1) {
		printf("Usage: myrm <file_name>");
		return -1;
	}

	update_history("myrm\n");

	remove(args[0]);
	return 0;
}

int my_mv(int arg_count, char *args[]) {
	if(arg_count < 2) {
		printf("Usage: mymv <old_file_name> <new_file_name>");
		return -1;
	}

	update_history("mymv\n");
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

void get_history() {
	char buf[20];
	FILE *fp = fopen("cmd-history" , "r");

	if(fp != NULL) {
		while(fgets(buf, DEFAULT_BUFFER_SIZE, fp) != NULL)
			printf("%s",buf);
		fclose(fp);
	}
}

void get_help() {
	printf("Usage:\n mycat <file-name>\n myhead <file-name>\n mytail <file-name>\n mycp <old-file-name> <new-file-name>\n");
	printf(" mymv <old-file-name> <new-file-name>\n myrm <filename>\n history\n\n");

	printf("Pipe:\n mycat <file-name> | myhead\n mycat <file-name> | mytail | myhead\n");
}

int main() {
	char *cmd, buf[DEFAULT_BUFFER_SIZE], *write_buf;
	char *cmd_ptr, *args_ptr, *token, *args_token, *args[10], *token_cmd;
	int pipe_buf[2], arg_count = 0, pid = 0, wstatus = 0, buf_size=DEFAULT_BUFFER_SIZE, is_print_data = 1;

	cmd = (char*)malloc(DEFAULT_BUFFER_SIZE);

	pipe(pipe_buf);
		
	printf("> ");

	getline(&cmd, (size_t*)&buf_size, stdin);
	token = strtok_r(cmd, "|", &cmd_ptr);

	while(strcmp(trim(token), "exit")) {
		while(token) {
			args_token = trim(strtok_r(token, " ", &args_ptr));
			cmd = trim(args_token);

			token = strtok_r(NULL, "|", &cmd_ptr);
			if(token != NULL) is_print_data = 0;
			else is_print_data = 1;

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
					write_buf = my_cat(is_print_data, arg_count, args);
					write(pipe_buf[1], write_buf, DEFAULT_BUFFER_SIZE);
				}

				if(!strcmp(cmd, "myhead")) {
					write_buf = my_head(is_print_data, arg_count, args, write_buf);
					write(pipe_buf[1], write_buf, strlen(write_buf));
				}

				if(!strcmp(cmd, "mytail")) {
					write_buf = my_tail(is_print_data, arg_count, args, write_buf);
					write(pipe_buf[1], write_buf, strlen(write_buf));
				}

				if(!strcmp(cmd, "history")) {
					get_history();
				}

				if(!strcmp(cmd, "help")) {
					get_help();
				}

				exit(0);
			}

			if(!strcmp(cmd, "mycat") || !strcmp(cmd, "myhead") || !strcmp(cmd, "mytail")) {
				write_buf = malloc(DEFAULT_BUFFER_SIZE*sizeof(char));
				read(pipe_buf[0], write_buf, DEFAULT_BUFFER_SIZE);
			}

			waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);
		}

		printf("> ");

		getline(&cmd, (size_t*)&buf_size, stdin);
		token = strtok_r(cmd, "|", &cmd_ptr);
	}
	
	return 0;	
}