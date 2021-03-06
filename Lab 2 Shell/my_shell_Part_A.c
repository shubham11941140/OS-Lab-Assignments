#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens */

char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i =0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}


int main(int argc, char* argv[])
{
	char  line[MAX_INPUT_SIZE];
	char  **tokens;
	int i;

	FILE* fp;
	if(argc == 2)
	{
		fp = fopen(argv[1],"r");
		if(fp == NULL)
		{
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1)
	{
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2)
		{
			// batch mode
			if(fgets(line, sizeof(line), fp) == NULL)
			{
				// file reading finished
				break;
			}
			line[strlen(line) - 1] = '\0';
		}
		else
		{
			// interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}

		// printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		if(tokens[0]==NULL)
		{
			// empty command, pass on
			continue;
		}

		if(strcmp(tokens[0],"cd")==0)
		{
			// cd command
			int rt = chdir(tokens[1]);
			if(rt!=0)
			{
				printf("Shell: Incorrect command\n");
			}
			continue;
		}

       	//do whatever you want with the commands, here we just print them
		int rc=fork();
		if(rc<0)
		{
			// fork fail, exit
			printf("fork failed\n");
			exit(1);
		}
		else if(rc==0)
		{
			// printf("hello, I am child (pid:%d)\n", (int) getpid());
			execvp(tokens[0],tokens);
			printf("exec failed for command: %s\n", tokens[0]);
			_exit(1);
		}
		else
		{
			int rc_wait = wait(NULL);
			// printf("hello, I am parent of %d (rc_wait:%d) (pid:%d)\n",rc, rc_wait, (int) getpid());
		}


		// Freeing the allocated memory
		for(i=0;tokens[i]!=NULL;i++)
		{
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
