// include created headerfile
#include "mmake.h"

/* This C program... (add description)
 *
 * @file: mmake.c
 * @author: Jonatan Westling(c22jwg@cs.umu.se)
 * @version information:
 *   2023-09-27: v1.0, first version.
 */
struct flags
{
	bool flag_B;
	bool flag_s;
};

int main(int argc, char *argv[])
{
	//allocate memory for the flags we need later
	flags *my_flags = malloc(sizeof(flags));
	my_flags->flag_B = false;
	my_flags->flag_s = false;
	char *file_name = NULL;
	int opt;
	while ((opt = (getopt(argc, argv, "f:Bs"))) != -1)
	{
		switch (opt)
		{
		case 'f':
			// get the path to the makefile
			file_name = optarg;
			break;
		case 'B':
			// force rebuild requested
			my_flags->flag_B = true;
			break;
		case 's':
			// no outout to the terminal when running command
			my_flags->flag_s = true;
			break;
		default:
			// something went wrong
			fprintf(stderr, "Usage: %s [-f spec makefile] [-B force build]"
					" [-s scilent mode] target1, target2...\n",
				argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}
	char *makefile_path = "mmakefile";
	if (file_name != NULL)
	{
		// if user want to run another makefile
		makefile_path = file_name;
	}
	// open the makefile
	FILE *makefile_stream = NULL;
	makefile_stream = fopen(makefile_path, "r");
	if (makefile_stream == NULL)
	{
		perror(makefile_path);
		exit(EXIT_FAILURE);
	}
	makefile *m = NULL;
	m = parse_makefile(makefile_stream);
	if (m == NULL)
	{
		fprintf(stderr, "%s: Could not parse makefile\n", makefile_path);
		fclose(makefile_stream);
		exit(EXIT_FAILURE);
	}
	fclose(makefile_stream);
	// enter if user passed target/targets as argument
	if (optind < argc)
	{
		// run with costum targets, probably need to pass what the flags are here
		int i = optind;
		while (i < argc)
		{
			check_targets(m, argv[i], my_flags);
			i++;
		}
	}
	else
	{
		// run with default target since no custom targets where passed
		check_targets(m, makefile_default_target(m), my_flags);
	}
	// free allocated memory
	free(my_flags);
	makefile_del(m);
	exit(EXIT_SUCCESS);
}

/**
 * check_targets()- checks what targets that needs to be built
 * 
 * @m: a pointer to the struct that cantains the parsed_makefile
 * @targetname: a string of the targets name
 * RETURNS: nothing.
 */
void check_targets(makefile *m, const char *targetname, flags *f)
{
	// get the rules of the target
	rule *target_rule = makefile_rule(m, targetname);
	if ((target_rule == NULL) && non_existing_file(targetname))
	{
		fprintf(stderr, "mmake: No rules to make target '%s'.\n", targetname);
		exit(EXIT_FAILURE);
	}
	if (target_rule == NULL)
	{
		// target has no rules
		return;
	}
	// get the prereqs from the target rule
	const char **prereq = rule_prereq(target_rule);
	// compare target with its prereqs
	int i = 0;
	while (prereq[i] != NULL)
	{
		// recursive call the function
		check_targets(m, prereq[i], f);
		i++;
	}
	
	if (non_existing_file(targetname) || target_needs_to_rebild(prereq, targetname, f))
	{
		compile_target_rule(target_rule, f);
	}
	return;
}

/**
 * first_file_is_newer()- Checks if the first file is newer than the
 * second one.
 *
 * @file1: first file to compare (prereq)
 * @file2: second file to compare (target)
 * RETURNS: true or false
 */
bool first_file_is_newer(const char *file1, const char *file2)
{
	struct stat file_info1;
	struct stat file_info2;
	if (stat(file1, &file_info1) != 0)
	{
		perror(file1);
		exit(EXIT_FAILURE);
	}
	if (stat(file2, &file_info2) != 0)
	{
		perror(file2);
		exit(EXIT_FAILURE);
	}
	if (file_info1.st_mtime > file_info2.st_mtime)
	{
		return true;
	}
	return false;
}

/**
 * compile_target_rule()- This function compiles the rules of a target and creates an
 * new updated target because its prereqs has been updated or because target does not exist.
 * This function uses child processes to execute the commands
 *
 * @target_rule: The rules of the soecific target
 * @f: the struct of flags
 * RETURNS: true or false
 */
void compile_target_rule(rule *target_rule, flags *f)
{
	pid_t pid;
	int status;
	if ((pid = fork()) < 0)
	{
		perror("Fork:");
		exit(EXIT_FAILURE);
	}
	if (pid == 0)
	{
		// in child process
		char **cmd_for_rule = rule_cmd(target_rule);
		int i = 0;
		// print if not in silence mode
		if (!f->flag_s)
		{
			while (cmd_for_rule[i] != NULL)
			{
				if (i!=0){
					fprintf(stdout, " ");
				}
				fprintf(stdout, "%s", cmd_for_rule[i]);
				i++;
			}
			fprintf(stdout, "\n");
			// flush buffer to make sure commands get printed
			fflush(stdout);
		} 
		// run the acctual command
		execvp(cmd_for_rule[0], cmd_for_rule);
		// if something with execvp went wrong
		perror("execvp");
		exit(EXIT_FAILURE);
	}
	else if (pid != 0)
	{
		// in parent process, collect child pid
		// get status
		pid = waitpid(pid, &status, 0);
		if (pid == -1)
		{
			perror("Waitpid");
			exit(EXIT_FAILURE);
		}
		// check the child exited at all and the status of the exit
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			exit(EXIT_FAILURE);
		}
	}
}

/**
 * non_existing_file()- This function checks if a file exist or not.
 *
 * @targetname: Name of the target to check
 * RETURNS: true or false
 */
bool non_existing_file(const char *targetname)
{
	struct stat buffer;
	return (stat(targetname, &buffer) != 0);
}

/**
 * target_needs_to_rebild()- This function checks if a target needs to be rebuilt or not
 *
 * @preq: The prerequsits of the target
 * @targetname: Name of the target to check
 * @f: the flags to check if force build is needed. 
 * RETURNS: true or false
 */
bool target_needs_to_rebild(const char **prereq, const char *targetname, flags *f){

	// if flag B is passed we will build anyways
	if (f->flag_B)
	{
		return true;
	}
	int i = 0;
	//check all prereqs with the target
	while (prereq[i] != NULL)
	{
		if(first_file_is_newer(prereq[i], targetname))
		{
			return true;
		}
		i++;
	}
	return false;
	
}