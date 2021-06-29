#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

// Splits the string by space and returns the array of tokens

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

int parallel_check(char  **tokens)
{
	for(int i=0; tokens[i]!=NULL && i<MAX_NUM_TOKENS; i++)
	{
		if(strcmp(tokens[i],"&&&")==0)
		{
			return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	char  line[MAX_INPUT_SIZE];
	char  **tokens;
	int i;
	int background_pids[MAX_NUM_TOKENS];

	for(i=0; i<MAX_NUM_TOKENS; i++)
	{
		// initialize
		background_pids[i]=-1;
	}

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

	signal(SIGINT, SIG_IGN);

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

		// reap background processes after taking user input
		for(i=0; i<MAX_NUM_TOKENS; i++)
		{
			if(background_pids[i]!=-1)
			{
				int rc = waitpid(background_pids[i],NULL,WNOHANG);
				if(rc==background_pids[i])
				{
					printf("Shell: Background process finished\n");
					background_pids[i]=-1;
				}
			}
		}

		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		if(tokens[0]==NULL)
		{
			// empty command, pass on
		}
		else if(strcmp(tokens[0],"exit")==0)
		{
			// exit command
			for(i=0;tokens[i]!=NULL;i++)
			{
				// free up dynamically allocated memory
				free(tokens[i]);
			}

			free(tokens);
			for(i=0; i<MAX_NUM_TOKENS; i++)
			{
				if(background_pids[i]!=-1)
				{
					int id = kill(background_pids[i],SIGINT);
					if(id<0)
					{
						exit(1);
					}

					int rc = waitpid(background_pids[i],NULL,WUNTRACED);
					if(rc != background_pids[i])
					{
						printf("Error in killing process\n");
					}
					else
					{
						printf("Shell: Background process finished\n");
					}
				}
			}
			break;
		}
		else if(strcmp(tokens[0],"cd")==0)
		{
			// cd command
			int rt = chdir(tokens[1]);
			if(rt!=0)
			{
				printf("Shell: Incorrect command\n");
			}
		}
		else if(parallel_check(tokens)==0)
		{
			// series foreground and background case
			int cur_token_ind=0;
			char *cur_tokens[MAX_NUM_TOKENS];

			for(i=0;i<MAX_NUM_TOKENS;i++)
			{
				if(tokens[i]==NULL || strcmp(tokens[i],"&&")==0)
				{
					// series foreground
					cur_tokens[cur_token_ind++]=NULL;
					int rc=fork();
					if(rc<0)
					{
						//fork fail,exit
						printf("fork failed\n");
						exit(1);
					}
					else if(rc==0)
					{
						//child process
						signal(SIGINT,SIG_DFL);
						execvp(cur_tokens[0],cur_tokens);
						printf("Shell: Exec Failed for Command: %s\n", cur_tokens[0]);
						_exit(1);
					}
					else
					{
						int wstat;
						int rc_wait = waitpid(rc,&wstat,0);

						if(WIFSIGNALED(wstat))
							break;
							// if the child was terminated by signal then break for loop and dont execute further commands

						if(rc_wait==-1)
						{
							printf("Error in reaping\n");
							_exit(1);
						}
					}
					cur_token_ind = 0;
					if(tokens[i]==NULL)
						break;
				}
				else if(strcmp(tokens[i],"&")==0)
				{
					// background case
					tokens[i]=NULL;
					int rc=fork();
					if(rc<0)
					{
						//fork fail,exit
						printf("fork failed\n");
						exit(1);
					}
					else if(rc==0)
					{
						//child process
						signal(SIGINT,SIG_DFL);
						setpgid(0,0);
						execvp(tokens[0],tokens);
						printf("Shell: Exec Failed for Command: %s\n", tokens[0]);
						_exit(1);
					}
					else
					{
						// dont wait
						for(int j=0; j<MAX_NUM_TOKENS; j++)
						{
							// store background pid in the array
							if(background_pids[j]==-1)
							{
								background_pids[j]=rc;
								break;
							}
						}
						break; // break from for loop
					}
				}
				else
				{
					cur_tokens[cur_token_ind++]=tokens[i];
				}
			}
		}
		else
		{
			// parallel case &&&
			int cur_token_ind=0;
			char *cur_tokens[MAX_NUM_TOKENS];
			int parallel_pid[MAX_NUM_TOKENS], par_pid_ind=0;
			for(i=0;i<MAX_NUM_TOKENS;i++)
			{
				if(tokens[i]==NULL || strcmp(tokens[i],"&&&")==0)
				{
					cur_tokens[cur_token_ind]=NULL;
					int rc=fork();
					if(rc<0)
					{
						//fork fail,exit
						printf("fork failed\n");
						exit(1);
					}
					else if(rc==0)
					{
						//child process
						signal(SIGINT,SIG_DFL);
						execvp(cur_tokens[0],cur_tokens);
						printf("Shell: Exec Failed for Command: %s\n", cur_tokens[0]);
						_exit(1);
					}
					else
					{
						parallel_pid[par_pid_ind++]=rc;
					}
					cur_token_ind = 0;
					if(tokens[i]==NULL)
						break;
				}
				else
				{
					cur_tokens[cur_token_ind++]=tokens[i];
				}
			}
			for(i=0; i<par_pid_ind; i++)
			{
				// reap all these processes
				waitpid(parallel_pid[i],NULL,0);
			}
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
