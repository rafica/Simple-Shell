#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h>

char* get_input();
bool process_string(char* str[], int i);

extern int errno;
char *path="";

int main(){
	
	char *command;

//	path = "";

	int max_commands = 10;
	int y =0;
	char *token;
	char *arg[100];
	int i=0;
	bool carryOn;

	while(y<max_commands){
		y++;
		//display $
		printf("$");


		command = get_input();
		
		token = strtok(command, " \t");
		while(token != NULL){
			arg[i++] = token;
			token = strtok(NULL, " \t");
		
		}

		carryOn = process_string(arg,i);	
		i=0;	
	        //after using the command
		if(command)
			free(command);
		command = NULL;

		if(!carryOn)
			break;
		
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


bool process_string(char* str[], int len){
	
	char *directory;
	
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
			//do this
				
			}
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
		//fprintf(stderr, "Error allocating space :%s", strerror(errno));
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
					//flushing the input
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
