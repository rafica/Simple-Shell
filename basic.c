#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>

char* get_input();

extern int errno;

int main(){
	
	char *command;


	int max_commands = 10;
	int y =0;
	char *token;
	char *arg[100];
	int i=0;

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


		if(strcmp(arg[0], "exit")==0)
			exit(0);

		
		
	        //after using the command
		if(command)
			free(command);
		command = NULL;

		
	}
	return 0;
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
