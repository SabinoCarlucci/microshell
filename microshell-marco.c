//thanks to mtani

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

void print_error(char *str, char *arg)
{
	write(2, str, ft_strlen(str));
	if (arg)
		write(2, arg, ft_strlen(arg));
	write(2, "\n", 1);
}

int ft_execute(char **argv, int i, int tmp, char **envp)
{
	argv[i] = NULL;
	dup2(tmp, 0);//perche' non sto mettendo dentro tmp, sto mettendo tmp su 0
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
	tmp = dup(0);//salvo stdin perche' poi lo reindirizzero' quando faccio pipe
	while(argv[i] && argv[i + 1])
	{
		//fai avanzare argomenti
		argv = &argv[i + 1];
		i = 0;
		//scorri argomenti fino a ; o |
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
				while (waitpid(-1, NULL, 0) != -1)//aspetta che tutti i figli abbiano finito
					;
				tmp = dup(0);
			}
		}
		else if (i != 0 && strcmp(argv[i], "|") == 0)
		{
			if (pipe(fd) != 0)//se pipe fallisce
			{
				print_error("error: fatal", NULL);
				exit(1);
			}
			if (fork() == 0)//process figlio
			{
				dup2(fd[1], 1);//voglio che scrittura vada su pipe
				close(fd[0]);//non mi serve leggere
				close(fd[1]);//tanto ormai ho 1
				ft_execute(argv, i, tmp, envp);
			}
			else//processo padre
			{
				close(fd[1]);//perche' fork duplica anche fd, quindi va chiuso sia qui che nel figlio
				close(tmp);//chiudi vecchio tmp per sorascriverlo con tmp nuovo, dato che pipe cambia
				tmp = fd[0];//non dup perche' fd[0] e' gia' valido e fare dup creerebbe altro fd da chiudere
			}
		}
	}
	close(tmp);
	return(0);
}