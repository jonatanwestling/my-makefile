
// Headerfile created for this c code
#include "mexec.h"

/* This C program
 *  
 * Author: Jonatan Westling(c22jwg@cs.umu.se)
 * 
 * Version information:
 *   2023-09-13: v1.0, first version.
 */ 

/**
 * main()- Main function that run the whole program by calling
 * diffrent functions.
 *
 * @argc: The amount of command-line arguments
 * @argc: An array of strings that hold the arguments passed to
 * the program.
 */
int main(int argc, char *argv[])
{
	char **args = NULL;
	int argCount = 0;

	if (argc == 1)
	{
		// no arguments passed reas from stdin
		args = readStream(stdin, &argCount);
	}
	else if (argc == 2)
	{

		FILE *file = fopen(argv[1], "r");
		if (file == NULL)
		{
			fprintf(stderr, "Failed to open the file.\n");
			exit(EXIT_FAILURE);
		}
		args = readStream(file, &argCount);
		if (fclose(file) != 0)
		{
			perror("Failed to close file\n");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		fprintf(stderr, "Invalid numbers of arguments. Program takes one or zero arguments.\n");
		exit(EXIT_FAILURE);
	}

	// execute all the commands
	executeCommands(args, argCount);

	// Free allocated memory for args
	for (int i = 0; i < argCount; i++)
	{
		free(args[i]);
	}
	free(args);

	// printf("Program exit success\n");
	exit(EXIT_SUCCESS);
}
/**
 * readStream()- Function that reads the input
 * and stringize the input to an dynamiclly allocated
 * array of strings.
 *
 * @stream: A pointer to the input stream to read from
 * @argCount: A pointer to an integer to store the count of strings
 * RETURN: Ruturns the dynamiccaly allocated array of strings where each
 * string is a command. The main functuion get the responsibility of freeing
 * the allocated memmory.
 */
char **readStream(void *stream, int *argCount)
{
	char command[MAX_LINE_LENGHT];
	char **args = NULL;
	int argCounter = 0;

	// read each line until end of file is reached
	while (fgets(command, sizeof(command), stream) != NULL)
	{
		size_t size = strlen(command);

		// replace newline terminator  with \0
		if (size > 0 && command[size - 1] == '\n')
		{
			command[size - 1] = '\0';
		}
		// allocate more memory for the dynamic array by one slot
		args = (char **)realloc(args, (argCounter + 1) * sizeof(char *));
		if (args == NULL)
		{
			perror("Memory allocation failed.\n");
			exit(EXIT_FAILURE);
		}
		//allocate memory for the string itself including the nullterminator 
		args[argCounter] = (char *)malloc(strlen(command) + 1);
		if (args[argCounter] == NULL)
		{
			perror("Memory allocation failed.\n");
			exit(EXIT_FAILURE);
		}
		//copy the string to the dynamic array
		strcpy(args[argCounter], command);
		argCounter++;
	}
	*argCount = argCounter;
	return args;
} 
/**
 * executeCommands()- Creates all nessecary pipes
 * then runs all the commands that is stored in the array of strings.
 *
 * @args: An array of strings (pointer to a pointer to a char)
 * @argCount: An integer that tells how many commans there are.
 * RETURNS: Nothing
 */
int executeCommands(char **args, int argCount)
{
	pid_t pid;
	int pipefd[argCount - 1][2];
	pid_t childPids[argCount];
	// create all the pipes
	for (int i = 0; i < argCount - 1; i++)
	{
		if (pipe(pipefd[i]) != 0)
		{
			perror("Pipe");
			exit(EXIT_FAILURE);
		}
	}
	// run all the commands
	int i = 0;
	while (i < argCount)
	{
		// create a fork
		if ((pid = fork()) < 0)
		{
			perror("Fork");
			exit(EXIT_FAILURE); 
		}
		if (pid != 0)
		{
			childPids[i] = pid;
		}
		// enter for child protextttcess
		if (pid == 0)
		{
			// redirect output to next child
			if (i < argCount - 1)
			{
				if (dup2(pipefd[i][WRITE_END], STDOUT_FILENO) == -1)
				{
					perror("dup2");
					exit(EXIT_FAILURE);
				}
				close(pipefd[i][WRITE_END]);
			}

			// redirect input from previous pipe
			if (i > 0)
			{
				if (dup2(pipefd[i - 1][READ_END], STDIN_FILENO) == -1)
				{
					perror("dup2");
					exit(EXIT_FAILURE);
				}
				close(pipefd[i - 1][READ_END]);
			}

			// Close all pipes in the child process
			for (int j = 0; j < argCount - 1; j++)
			{
				close(pipefd[j][0]);
				close(pipefd[j][1]);
			}

			execl("/bin/sh", "shell", "-c", args[i], NULL);
			// if process did not start
			perror("Execlp");
			exit(EXIT_FAILURE);
		}
		i++;
	}
	// close all the pipes in the parent
	for (int j = 0; j < argCount - 1; j++)
	{
		close(pipefd[j][READ_END]);
		close(pipefd[j][WRITE_END]);
	}
	waitForChildren(childPids, argCount, args);
	
	return 0;
}
/**
 * waitForChildren()- Function wait for all the child processes
 * and check if the child processes exited and with what type of 
 * status.
 *
 * @argCount: An integer that tells how many commans there are.
 * @childPids[]: An array of the active child process IDs.
 * @args: The allocated array with strings if we need to free the 
 * memory allocation before exit faliure.
 * RETURNS: Nothing
 */
void waitForChildren(pid_t childPids[], int argCount, char ** args)
{
	pid_t pidStatus;
	int status;
	// wait for the child process
	for (int i = 0; i < argCount; i++)
	{
		// get status
		pidStatus = waitpid(childPids[i], &status, 0);

		if (pidStatus == -1)
		{
			perror("Waitpid");
			exit(EXIT_FAILURE);
		}
		// check the child exited at all and the status of the exit
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			//free allocated memory before exit faliure
			for(int i = 0; i < argCount; i++){
				free(args[i]);
			}
			free(args);
			exit(EXIT_FAILURE);
		}
	}
}