#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/wait.h>

char *get_input();
int process_string(char *str[], int i);
char *concat(char *s1, char *s2);
char *general_concat(char *s1, char *s2);
char *find_path(char *cmd);
int isPresent(char *cmd, char *dir);
pid_t wait(int *status);
void remove_path(char *filePath);
void process_pipe(char *commands);
void execute_pipe(int n, char *cmd[]);
char *trim_white_space(char *str);
int isspace(int c);

extern int errno;
char *path = "";
int stdin_copy;
int stdout_copy;

int main(void)
{
	char *command;
	char *token;
	char *arg[100];
	int i = 0;
	int carryOn;

	/* making a copy of the default input and output file handlers.
	stdin and stdout file handlers
		are changed later */
	stdin_copy = dup(STDIN_FILENO);
	stdout_copy = dup(STDOUT_FILENO);
	if (stdin_copy < 0 || stdout_copy < 0) {
		printf("error: %s\n", strerror(errno));
		return 0;
	}
	while (1) {
		printf("$");
		int t1 = dup2(stdin_copy, STDIN_FILENO);
		int t2 = dup2(stdout_copy, STDOUT_FILENO);

		if (t1 < 0 && t2 < 0) {
			printf("error: %s\n", strerror(errno));
			break;
		}
		command = get_input();
		/*trim leading and trailing whitespae */
		command = trim_white_space(command);
		/*check if there is pipe. if there is,
		its processed in process_pipe() */
		if (strstr(command, "|") != NULL) {
			process_pipe(command);
			if (command)
				free(command);
			command = NULL;
			continue;
		}
		/*tokenized*/
		token = strtok(command, " \t");
		while (token != NULL) {
			arg[i++] = token;
			token = strtok(NULL, " \t");
		}
		arg[i] = NULL;
		carryOn = process_string(arg, i);
		i = 0;
		if (command)
			free(command);
		command = NULL;
		if (!carryOn)
			break;
	}
	return 0;
}
/*trims leading and trailing white space */
char *trim_white_space(char *str)
{
	size_t len = 0;
	char *frontp = str;
	char *endp = NULL;

	if (str == NULL)
		return NULL;
	if (str[0] == '\0')
		return str;
	len = strlen(str);
	endp = str + len;
	while (isspace(*frontp))
		++frontp;
	if (endp != frontp) {
		while (isspace(*(--endp)) && endp != frontp)
			;
	}
	if (str + len - 1 != endp)
		*(endp + 1) = '\0';
	else if (frontp != str && endp == frontp)
		*str = '\0';
	endp = str;
	if (frontp != str) {
		while (*frontp)
			*endp++ = *frontp++;
		*endp = '\0';
	}
	return str;
}

void process_pipe(char *commands)
{
	int pipe_flag = 0;
	int c  = 0;

	/*check if the pipe command is not syntactically correct */
	/*if command starts with pipe, ends with pipe or two consecutive pipes*/
	for (c = 0; c < strlen(commands); c++) {
		char ch = commands[c];

		if (c == 0 && ch == '|') {
			printf("error: Command not found\n");
			return;
		}
		if (c == (strlen(commands) - 1) && ch == '|') {
			printf("error: command not found\n");
			return;
		}
		if (ch == '|' && pipe_flag == 1) {
			printf("error: Command not found\n");
			return;
		}
		if (ch == '|' && pipe_flag == 0)
			pipe_flag = 1;
		else if (pipe_flag == 1 && ch != ' ' && ch != '\t')
			pipe_flag = 0;
	}
	char *token;
	int cmdNo = 0;
	char *tmp1 = malloc(strlen(commands) + 1);
	char *tmp;
	int i = 0;

	/*tokenizing over pipe */
	if (tmp1 == NULL)
		return;
	tmp = tmp1;
	strcpy(tmp, commands);
	token = strtok(tmp, "|");
	while (token != NULL) {
		cmdNo = cmdNo + 1;
		token = strtok(NULL, "|");
	}
	free(tmp);
	tmp =  NULL;
	tmp1 = malloc(strlen(commands) + 1);
	if (tmp1 == NULL)
		return;
	tmp = tmp1;
	strcpy(tmp, commands);
	char *cmd[cmdNo];

	token = strtok(tmp, "|");
	while (token != NULL) {
		cmd[i] = token;
		token = strtok(NULL, "|");
		i++;
	}
	/*after tokenizing the cmd & validation, this func processes */
	execute_pipe(cmdNo, cmd);

}

int create_process(int in, int out, char *arg[])
{
	pid_t child_pid = fork();

	if (child_pid < 0)
		printf("error: Fork failure\n");
	else if (child_pid > 0) {
		int status;

		if (waitpid(child_pid, &status, 0) == -1) {
			printf("error: %s\n", strerror(errno));
			return 0;
		}
		/* if any one of the pipe fails exit the command */
		if (WEXITSTATUS(status))
			return 0;
	} else if (child_pid == 0) {
		if (in != 0) {
			if (dup2(in, 0) == -1) {
				printf("error: %s\n", strerror(errno));
				return 0;
			}
			if (close(in) == -1) {
				printf("error: %s\n", strerror(errno));
				return 0;
			}
		}
		if (out != 1) {
			if (dup2(out, 1) == -1) {
				printf("error: %s\n", strerror(errno));
				return 0;
			}
			if (close(out) == -1) {
				printf("error: %s\n", strerror(errno));
				return 0;
			}
		}
		/* execute the command as it is */
		execv(arg[0], arg);
		char *foundPath;

		/* if it fails find the path of the command */
		foundPath = find_path(arg[0]);
		if (foundPath == NULL) {
			if (dup2(stdout_copy, STDOUT_FILENO) == -1) {
				printf("error: %s\n", strerror(errno));
				return 0;
			}
			printf("error: command not found\n");
			exit(10);
		}
		foundPath = general_concat(foundPath, "/");
		foundPath = general_concat(foundPath, arg[0]);
		/*execute with the found path */
		execv(foundPath , arg);
		if (dup2(stdout_copy, STDOUT_FILENO) == -1) {
			printf("error: %s\n", strerror(errno));
			return 0;
		}
		printf("error: %s", strerror(errno));
		exit(10);
	}
	return 1;
}

void execute_pipe(int n, char *cmd[])
{
	int i;
	int in, fileDesc[2];
	char *arg[100];

	in = 0;
	char *token;
	int j;

	for (i = 0; i < n-1; i++) {
		j = 0;
		/*get file descriptors*/
		if (pipe(fileDesc) == -1) {
			printf("error: %s\n", strerror(errno));
			return;
		}
		/*tokenize*/
		token = strtok(cmd[i], " \t");
		while (token != NULL) {
			arg[j++] = token;
			token = strtok(NULL, " \t");
		}
		arg[j] = NULL;
		/* change the input and output ends for the next command */
		if (!create_process(in, fileDesc[1], arg))
			return;
		if (close(fileDesc[1]) == -1) {
			printf("error: %s\n", strerror(errno));
			return;
		}
		in = fileDesc[0];
	}
	if (in != 0) {
		if (dup2(in, 0) == -1)
			return;
	}
	j = 0;
	token = strtok(cmd[i], " \t");
	while (token != NULL) {
		arg[j++] = token;
		token = strtok(NULL, " \t");
	}
	arg[j] = NULL;
	int child_pid = fork();
	/*execute for the last command. output should be default stdout*/
	if (child_pid > 0) {
		int status;

		if (wait(&status) == -1) {
			printf("error: %s\n", strerror(errno));
			return;
		}
	} else if (child_pid == 0) {
		execv(arg[0], arg);
		char *foundPath = find_path(arg[0]);

		if (foundPath == NULL) {
			printf("error: Command not found here\n");
			exit(1);
		}
		foundPath = general_concat(foundPath, "/");
		foundPath = general_concat(foundPath, arg[0]);
		execv(foundPath, arg);
		printf("error: %s\n", strerror(errno));
		exit(1);
	} else {
		printf("error: %s\n", strerror(errno));
		return;
	}
}
/*function to concat two strings with : as separator*/
char *concat(char *s1, char *s2)
{
	if (s1 == NULL || s2 == NULL)
		return NULL;
	char *result;
	char *tmp = malloc(strlen(s1) + strlen(s2) + 2);

	if (tmp == NULL)
		return NULL;
	result = tmp;
	strcpy(result, s1);
	if (strcmp(s1, "") != 0) {
		free(s1);
		strcat(result, ":");
	}
	strcat(result, s2);
	return result;
}
/*function to concat two strings */
char *general_concat(char *s1, char *s2)
{
	if (s1 == NULL || s2 == NULL)
		return NULL;
	char *result;
	char *tmp = malloc(strlen(s1) + strlen(s2) + 1);

	if (tmp == NULL)
		return NULL;
	result = tmp;
	strcpy(result, s1);
	if (strcmp(s1, "") != 0)
		free(s1);
	strcat(result, s2);
	return result;
}
/*find the path of the command from the path variable */
char *find_path(char *cmd)
{
	char *token;
	char *tmp1 = malloc(strlen(path) + 1);
	char *tmp;
	char *result = NULL;
	char *tresult;

	if (tmp1 == NULL)
		return NULL;
	tmp = tmp1;
	strcpy(tmp, path);
	if (strcmp(tmp, "") == 0)
		return NULL;
	token = strtok(tmp, ":");
	while (token != NULL) {
		if (isPresent(cmd, token)) {
			tresult = malloc(strlen(token) + 1);
			if (tresult == NULL) {
				result =  NULL;
				break;
			}
			result = tresult;
			strcpy(result, token);
			break;
		}
		token = strtok(NULL, ":");
	}
	free(tmp);
	tmp = NULL;
	return result;
}

/*check if the cmd is present in dir*/
int isPresent(char *cmd, char *dir)
{
	DIR *dp;
	struct dirent *e;
	struct stat statbuf;
	char cwd[1024];
	int found = 0;

	dp = opendir(dir);
	if (dp == NULL)
		return 0;
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		printf("cwd error\n");
		return 0;
	}
	chdir(dir);
	while ((e = readdir(dp)) != NULL) {
		lstat(e->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode) == 0 &&
			strcmp(e->d_name, cmd) == 0) {
			found = 1;
			break;
		}
	}
	chdir(cwd);
	closedir(dp);
	return found;
}
/*remove a path from the path variable*/
void remove_path(char *filePath)
{
	char *token;
	char *newPath = "";

	token = strtok(path, ":");
	while (token != NULL) {
		if (strcmp(token, filePath) != 0)
			newPath = concat(newPath, token);
		token = strtok(NULL, ":");
	}
	if (newPath == NULL)
		return;
	free(path);
	path = NULL;
	path = newPath;
}
/* the main function which identifies what type of command it is*/
int process_string(char *str[], int len)
{
	char *directory;
	pid_t child_pid;
	char *foundPath;

	if (len > 0 && strcmp(str[0], "exit") == 0)
		return 0;
	if (len > 1 && strcmp(str[0], "cd") == 0) {
		directory = str[1];
		if (chdir(directory) < 0)
			printf("error: %s\n", strerror(errno));
	} else if (len == 1 && strcmp(str[0], "path") == 0) {
		printf("%s\n", path);
	} else if (len == 3 && strcmp(str[0], "path") == 0 &&
			strcmp(str[1], "+") == 0) {
		char *tmp;

		tmp = concat(path, str[2]);
		if (tmp == NULL) {
			printf("Error:%s\n ", strerror(errno));
			return 1;
		}
		path = tmp;
	} else if (len == 3 && strcmp(str[0], "path") == 0 &&
			strcmp(str[1], "-") == 0) {
		if (strcmp(path, "") == 0)
			return 1;
		remove_path(str[2]);
	} else if (len > 0) {
		child_pid = fork();
		if (child_pid > 0) {
			int status;

			wait(&status);
		} else if (child_pid == 0) {
			execv(str[0], str);
			if (strcmp(path, "") == 0) {
				printf("error: Command not found\n");
				exit(1);
			}
			foundPath = find_path(str[0]);
			if (foundPath == NULL) {
				printf("error: Command not found\n");
				exit(1);
			}
			foundPath = general_concat(foundPath, "/");
			foundPath = general_concat(foundPath, str[0]);
			execv(foundPath , str);
			printf("error: %s\n", strerror(errno));
			exit(1);
		} else
			printf("error: Fork failure\n");
	}
	return 1;

}
/*get input from the user*/
char *get_input()
{
	unsigned int len_max = 128;
	unsigned int current_size = 0;
	char *ptr;
	char *tmp = malloc(len_max);

	if (tmp != NULL) {
		ptr = tmp;
		tmp = NULL;
	} else {
		printf("Error : %s", strerror(errno));
		return NULL;
	}
	current_size = len_max;
	char next;
	unsigned int i = 0;

	while ((next = getchar()) != EOF && next != '\n') {
		ptr[i++] = next;
		if (i != current_size)
			continue;
		current_size = i + len_max;
		tmp = realloc(ptr, current_size);
		if (tmp) {
			ptr = tmp;
			tmp = NULL;
		} else{
			printf("Error : %s\n", strerror(errno));
			while ((next = getchar()) != '\n' && next != EOF)
				;
			free(ptr);
			ptr = NULL;
			return NULL;
		}
	}
	ptr[i] = '\0';
	return ptr;
}
