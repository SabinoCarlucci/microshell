#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

//ft_strlen
int ft_strlen(char *str)
{
	int i = 0;
	while (str[i])
		i++;
	return (i);
}

//error
void error(char *str, char *str2)
{
	write(2, str, ft_strlen(str));
	if (str2)
		write(2, str2, ft_strlen(str2));
	write(2, "\n", 1);
}

//ft_execute: mette NULL,
//duplica tmp su 0 e chiude tmp
// esegue execve (comando, argomenti, variabili amb), 
//messaggio errore e return
int ft_execute(char **argv, int i, int tmp, char **envp)
{
	argv[i] = NULL;
	dup2(tmp, 0);//copia tmp su 0, quindi quando leggiamo da 0 in realtà stiamo leggendo da tmp
	close(tmp);//0 e tmp sono uguali, ma sono entrambi aperti, quindi uno lo devo chiudere
	execve(argv[0], argv, envp);
	error("error: cannot execute ", argv[0]);
	exit(1);//mi raccomando exit e non return
}

int main(int argc, char **argv, char **envp)
{
	(void)argc;
	int tmp;
	int i;
	int pipe_fd[2];

	i = 0;
	tmp = dup(0);

	while (argv[i] && argv[i + 1])
	{
		//fai avanzare puntatore
		argv = &argv[i + 1];
		//resetta i
		i = 0;
		//scorri argomenti fino a fine rigo o pipe o semicolon
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|"))
			i++;
		if (strcmp(argv[0], "cd") == 0)
		{
			if (i != 2)
				error("error: cd: bad arguments", NULL);
			else if (chdir(argv[1]) != 0)
				error("error: cd: cannot change directory to ", argv[1]);
		}
		else if (i != 0 && (argv[i] == NULL || strcmp(argv[i], ";") == 0))
		{
			//se figlio
			if (fork() == 0)
			{
				ft_execute(argv, i, tmp, envp);
			}
			else
			{
				close(tmp);
				while (waitpid(-1, NULL, 2) != -1)//forse 2 è in realtà 0
					;
				tmp = dup(0);
			}
		}
		else if (i != 0 && strcmp(argv[i], "|") == 0)
		{
			if (pipe(pipe_fd) != 0)
			{
				error("error: fatal", NULL);
				exit(1);//non sono sicuro
			}
			//fork: figlio reindirizza 1, chiude pipe0 e 1, lancia execute
			if (fork() == 0)
			{
				dup2(pipe_fd[1], 1);
				close(pipe_fd[0]);
				close(pipe_fd[1]);
				ft_execute(argv, i, tmp, envp);
			}
			else
			{
				//padre chiude pipefd1, chiude tmp e mette pipe0 in tmp senza dup
				close(pipe_fd[1]);
				close(tmp);
				tmp = pipe_fd[0];
			}				
		}
	}	
	close(tmp);
	return (0);
}