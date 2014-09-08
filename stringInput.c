#include<stdio.h>

int main(){
	char next;
	while((next=getchar())!=EOF && next!='\n')
		printf("%c\n",next);

}
