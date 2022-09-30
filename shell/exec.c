#include "exec.h"

dup2_checking(int fd, int dest)
{
	if (dup2(fd, dest) < 0) {
		perror("Error");
	}
}

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; eargv[i]; i++) {
		if (block_contains(eargv[i], '=')) {
			char *value = split_line(eargv[i], '=');
			if (setenv(eargv[i], value, 0) < 0)
				perror("Error");
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd = -1;
	fd = open(file, flags, S_IWUSR | S_IRUSR);
	if (fd < 0)
		perror("Error");
	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);
		if (execvp(e->argv[0], e->argv) < 0) {
			perror("Error");
		}
		break;

	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;

		exec_cmd(b->c);
	}

	case REDIR: {
		// changes the input/output/stderr flow

		r = (struct execcmd *) cmd;
		int in_fd = -1, out_fd = -1, err_fd = -1;
		if (strlen(r->in_file)) {
			in_fd = open_redir_fd(r->in_file, O_CLOEXEC);
			dup2_checking(in_fd, 0);
		}
		if (strlen(r->out_file)) {
			out_fd = open_redir_fd(r->out_file, O_CREAT | O_WRONLY);
			dup2_checking(out_fd, 1);
		}
		if (strlen(r->err_file)) {
			err_fd = open_redir_fd(r->err_file, O_CREAT | O_WRONLY);
			if (block_contains(r->err_file, '&') == 0) {
				err_fd = out_fd;
			}
			dup2_checking(err_fd, 2);
		}
		r->type = EXEC;
		exec_cmd((struct cmd *) r);
		break;
	}

	case PIPE: {
		// pipes two commands
		//
		p = (struct pipecmd *) cmd;
		int fds[2], i, pipe_res;

		if ((pipe_res = pipe(fds)) < 0 || (i = fork()) < 0) {
			if (pipe_res == 0) {
				close(fds[READ]);
				close(fds[WRITE]);
			}
			perror("Error");
			return;
		}

		if (i == 0) {
			close(fds[WRITE]);
			dup2_checking(fds[READ], READ);
			close(fds[READ]);
			cmd = parse_line(p->rightcmd->scmd);
			exec_cmd(cmd);
		} else {
			close(fds[READ]);
			dup2_checking(fds[WRITE], WRITE);
			close(fds[WRITE]);
			exec_cmd(p->leftcmd);
		}

		waitpid(i, &status, 0);

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);

		break;
	}
	}
}
