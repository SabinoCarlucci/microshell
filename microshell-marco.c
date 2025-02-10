#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

void print_error(char *str, char *arg)
{
	write(2, str, strlen(str));
	if (arg)
		write(2, arg, strlen(arg));
	write(2, "\n", 1);
}

int ft_execute(char **argv, int i, int tmp, char **envp)
{
	argv[i] = NULL;
	dup2(tmp, 0);
	close(tmp);
	execve(argv[0], argv, envp);
	print_error("error: cannot execute ", argv[0]);
	exit(1);
}

int main(int argc, char **argv, char **envp)
{
	int i;
	int fd[2];
	int tmp;
	(void)argc;

	i = 0;
	tmp = dup(0);
	while(argv[i] && argv[i + 1])
	{
		argv = &argv[i + 1];
		i = 0;
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|"))
			i++;
		if (strcmp(argv[0], "cd") == 0)
		{
			if (i != 2)
				print_error("error: cd: bad arguments", NULL);
			else if (chdir(argv[1]) != 0)
				print_error("error: cd: cannot change directory to ", argv[1]);
		}
		else if (i != 0 && (argv[i] == NULL || strcmp(argv[i], ";") == 0))
		{
			if (fork() == 0)
				ft_execute(argv, i, tmp, envp);
			else
			{
				close(tmp);
				while (waitpid(-1, NULL, 2) != -1)
					;
				tmp = dup(0);
			}
		}
		else if (i != 0 && strcmp(argv[i], "|") == 0)
		{
			if (pipe(fd) != 0)
				print_error("error: fatal", NULL);
			if (fork() == 0)
			{
				dup2(fd[1], 1);
				close(fd[0]);
				close(fd[1]);
				ft_execute(argv, i, tmp, envp);
			}
			else
			{
				close(fd[1]);
				close(tmp);
				tmp = fd[0];
			}
		}
	}
	close(tmp);
	return(0);
}