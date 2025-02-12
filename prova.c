#include <stdlib.h>
#include <unistd.h>//per pipe
#include <sys/types.h>//per fork
#include <sys/wait.h>
#include <string.h>//per strcmp

int ft_strlen(char *str)
{
	int i = 0;

	while (str[i])
		i++;
	return (i);
}

void error(char *str1, char *str2)
{
	write(2, str1, ft_strlen(str1));
	if (str2)
		write(2, str2, ft_strlen(str2));
	write(2, "\n", 1);
}

int ft_execute(char **argv, char **envp, int i, int tmp)//viene eseguito tutto solo se execve non funziona
{
	argv[i] = NULL;//metto una fine alla lista di argomenti per far funzionare execve
	dup2(tmp, 0);//copia tmp su 0, quindi se leggiamo da 0 in realta' stiammo leggendo da tmp
	close(tmp);
	execve(argv[0], argv, envp);
	//da qui, solo se execve fallisce
	error("error: cannot execute ", argv[0]);
	return (1);//per questo funzione e' di tipo int
}

int main(int argc, char **argv, char **envp)
{
	(void)argc;//non ci serve
	int	i;//contatore per scorrere argomenti
	int pipe_fd[2];//array per fd del pipe
	int tmp;//file descriptor che useremo per tenere 0 durante le operazioni

	tmp = dup(0);//come sopra; sto memorizzando 0 per usarlo dopo
	i = 0;
	while (argv[i] && argv[i + 1])
	{
		//fai avanzare il puntatore
		argv = &argv[i + 1];//la & per indirizzo di memoria
		//resetta i
		i = 0;
		//scorri argomenti fino a ; o | (usa strcmp)
		while(argv[i] && (strcmp(argv[i], ";")) && (strcmp(argv[i], "|")))
			i++;
		//ora ho puntatore su fine argomenti o ";" o "|"
		//se e' cd, eseguilo
		if (strcmp(argv[0], "cd") == 0)
		{
			if (i != 2)
				error("error: cd: bad arguments", NULL);
			else if (chdir(argv[1]) == -1)
				error("error: cd: cannot change directory to ", argv[1]);
		}
		else if (i != 0 && (argv[i] == NULL || (strcmp(argv[i], ";") == 0)))
		{
			//fai fork: figlio esegue execve, padre aspetta figlio e rimette tmp = 0
			if (fork() == 0)
			ft_execute(argv, envp, i, tmp);
			else
			{
				close(tmp);
				while (waitpid(-1, NULL, 2) != -1)
					;
				tmp = dup(0);
			}
		}
		else if (i != 0 && (strcmp(argv[i], "|") == 0))
		{
			//crea pipe, se non funziona errore
			if (pipe(pipe_fd) != 0)
				error("error: fatal", NULL);//metto exit?
			//fai fork: figlio reindirizza 1, chiude pipe0 e 1, esegue execve
			if (fork() == 0)
			{
				dup2(pipe_fd[1], 1);
				close(pipe_fd[0]);
				close(pipe_fd[1]);
				ft_execute(argv, envp, i, tmp);
			}
			//padre chiude fd inutili e mette pipe0 in temp
			else
			{
				close(pipe_fd[1]);
				close(tmp);
				tmp = pipe_fd[0];
			}
		}
	}
	close(tmp);
	return (0);
}