#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h>
#include<dirent.h>
#include<sys/stat.h>


char* get_input();
int process_string(char* str[], int i);

extern int errno;
char *path="";

int main(){
	
	char *command;
	char *token;
	char *arg[100];
	int i=0;
	bool carryOn;

	while(true){
		
		printf("$");


		command = get_input();
		
		token = strtok(command, " \t");
		while(token != NULL){
			arg[i++] = token;
			token = strtok(NULL, " \t");
		
		}
		arg[i] = NULL;
		carryOn = process_string(arg,i);	
		
		i=0;	
	       
		if(command)
			free(command);
		command = NULL;

		if(!carryOn)
			break;
		printf("waiting for next \n");		
	}
	return 0;
}

char* concat(char *s1, char *s2){
	char *result;
	char *tmp = malloc(strlen(s1) + strlen(s2) + 2);

	if(tmp==NULL){
		return NULL;
	}
	else{
		result = tmp;
		strcpy(result, s1);		
		if(strcmp(s1, "")!=0){
			free(s1);
			strcat(result, ":");
		}
		strcat(result, s2);

		return result;
	}

}

char* general_concat(char *s1, char *s2){
	char *result;
	char *tmp = malloc(strlen(s1) + strlen(s2) + 1);

	if(tmp==NULL){
		return NULL;
	}
	else{
		result = tmp;
		strcpy(result, s1);		
		if(strcmp(s1, "")!=0){
			free(s1);
		}
		strcat(result, s2);

		return result;
	}

}


char* find_path(char *cmd){
	char *token;
	char *found=NULL;
	char *tmp1 = malloc(strlen(path) + 1);
	char *tmp;
	char *result;
	
	if(tmp1==NULL){
		return NULL;
	}
	else{
		tmp = tmp1;
		strcpy(tmp, path);		
		
	}

	if(strcmp(tmp, "")==0)
		return NULL;
	
	token = strtok(tmp, ":");
	while(token != NULL){
		
		if(isPresent(cmd, token)){
			result = malloc(strlen(token) +1);
			strcpy(result, token);
			found = result;
			break;
		}
		
		
		token = strtok(NULL, ":");
	
	}
	free(tmp);
	tmp = NULL;

	return result;

}


int isPresent(char* cmd, char* dir){
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	char cwd[1024];
	int found = 0;
	
	if((dp = opendir(dir))==NULL){
		return 0;
	}
	if(getcwd(cwd, sizeof(cwd)) == NULL){
		printf("cwd error\n");
		return 0;
	}
	

	chdir(dir);

	while((entry = readdir(dp))!= NULL){
		lstat(entry->d_name, &statbuf);
		if(S_ISDIR(statbuf.st_mode)==0){
			if(strcmp(entry->d_name, cmd)==0){
				found = 1;
				break;
			}
		}
	
	}
	chdir(cwd);

	closedir(dp);

	return found;
}

int process_string(char* str[], int len){
	
	char *directory;
	char *token;
	char *newPath="";
	pid_t child_pid;
	
	char *foundPath;

	int pid;
	if(len>0 && strcmp(str[0], "exit")==0){
		return 0;
	}

	else if(len>1 && strcmp(str[0], "cd")==0){
		directory = str[1];
		printf("%s ", directory);
		if(chdir(directory)<0){
			printf("error: %s\n", strerror(errno));
		}
		
	}
	else if(len>0 && strcmp(str[0], "path")==0){
		if(len==1){		
			printf("%s\n", path);
		}
		else if(len==3){
			if(strcmp(str[1], "+")==0){
				path = concat(path, str[2]);
				if(path==NULL){
					printf("Error:%s\n ",strerror(errno));
					return 0;
				}
			}
			else if(strcmp(str[1], "-")==0){
				if(strcmp(path, "")!=0){
						
					token = strtok(path, ":");
					while(token != NULL){
						
						if(strcmp(token, str[2])!=0){
							newPath = concat(newPath, token);
						}
						
						token = strtok(NULL, ":");
					
					}
				
					free(path);
					path = NULL;	
					
					path = newPath;
		
				
				}	
			
				
			}
		}
		
	}	

	else if(len>0){
		foundPath = find_path(str[0]);
			
		if((child_pid=fork())<0){
			printf("error: Fork failure\n");
			
		}

		else if(child_pid==0){
			foundPath = general_concat(foundPath, "/");
			foundPath = general_concat(foundPath, str[0]);	
			execv(foundPath , str);
			_exit(1);
		
		}
		else{
			wait();	
		}



	}
	return 1;


}

char* get_input(){


	unsigned int len_max = 128;
	unsigned int current_size = 0;
	

	char *ptr;
	char *tmp = malloc(len_max);
	if(tmp){
		ptr = tmp;
		tmp = NULL;
	}
	else{
		printf("Error : %s", strerror(errno));
		return NULL;	
	}


	current_size = len_max;

	char next;
		
	unsigned int i=0;


	while((next=getchar())!=EOF && next!='\n'){
			ptr[i++] = next;

			if(i == current_size){
				current_size = i+len_max;
				
				tmp = realloc(ptr, current_size);
				if(tmp){
					ptr = tmp;
					tmp = NULL;
				}
				else{
					printf("Error : %s\n", strerror(errno));
				
					while((next=getchar())!='\n' && next!= EOF);

					free(ptr);
					ptr = NULL;
					return NULL;	
				}
			}
	}
	ptr[i]='\0';


	return ptr;


}
