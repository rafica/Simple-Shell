#include<stdio.h>
#include<stdlib.h>

int main(){
	
	unsigned int len_max = 128;
	unsigned int current_size = 0;
	
	char *ptr = malloc(len_max);
	current_size = len_max;

	char next;

	int max_commands = 1;
	int y =0;
	while(y<max_commands){
		y++;
		//display $
		printf("$");


		if(ptr!= NULL){
			unsigned int i=0;
			// get input
			while((next=getchar())!=EOF && next!='\n'){
				ptr[i++] = next;

				if(i == current_size){
					current_size = i+len_max;
					ptr = realloc(ptr, current_size);
				}
			}
			ptr[i]='\0';

			printf("\n Long string value :%s \n\n", ptr);

			free(ptr);
			ptr = NULL;


		}	

	}

	return 0;
}
