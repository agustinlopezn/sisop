#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0)
		return 1;

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	char buf[FNAMESIZE];
	char *directory, *cwd;
	memset(buf, 0, FNAMESIZE);
	strcpy(buf, cmd);

	directory = split_line(buf, ' ');

	if (strcmp(buf, "cd") != 0)
		return 0;
	if (strlen(directory) == 0)
		sprintf(directory, "%s", "/home");
	if (chdir(directory) < 0 || (cwd = getcwd(buf, PRMTLEN)) < 0) {
		status = 1;
	}

	snprintf(promt, sizeof promt, "(%s)", cwd);

	return 1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") == 0) {
		char buf[FNAMESIZE];
		printf_debug(getcwd(buf, FNAMESIZE));
		printf_debug("\n");
		return 1;
	}
	return 0;
}
