/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 15:18:57 by rpaic             #+#    #+#             */
/*   Updated: 2025/02/05 15:32:42 by scarlucc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>  //chdir
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

void manage_fd(int has_pipe, int fd[2], int end)
{
    if (has_pipe && (dup2(fd[end], end) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
        write(2, "error: fatal\n", 13), exit(1);
}
int ft_cd(char **av, int args)
{
    int i = -1;
    if (args != 2)
    {
        write(2, "error: cd: bad arguments\n", 25);
        return(1);
    }
    if (chdir(av[1]) == -1)
    {
        write(2, "error: cd: cannot change directory to ", 38);
        while (av[1][++i])
            write(2, &av[1][i], 1);
        write(2, "\n", 1);
        return (1); 
    }
    //write(2, "CDCD\n", 5);
    return (0);
}
int ft_exec(char **av, int i, char **envp)
{
    int has_pipe, fd[2];
    int pid, status;
    int j = -1;

    //has_pipe
    has_pipe = av[i] && !strcmp(av[i], "|");
    //CMD is CD
    if (!has_pipe && !strcmp(av[0], "cd"))
        return(ft_cd(av, i));
    //create pipe && check
    if (pipe(fd) == -1)
        write(2, "error: fatal\n", 13), exit(1);
    //create fork && check
    if ((pid = fork()) == -1) 
        write(2, "error: fatal\n", 13), exit(1);
    //if CHILD
    if (pid == 0)//se processo figlio
    {
        //set last to NULL so we pass a mtx to EXECVE //cosa?
        av[i] = NULL;
        //manage_fd(fd, STDOUT);
        manage_fd(has_pipe, fd, 1);
        //check if CD
        if (strcmp(av[0], "cd") == 0)
            exit(ft_cd(av, i));
        //execve(*av, av, envp);
        execve(*av, av, envp);
        //write error;
        write(2, "error: cannot execute ", 22);
        while(av[1][++j])
            write(2, &av[1][j], 1);
        write(2, "\n", 1);
        exit(1);
    }
    waitpid(pid, &status, WUNTRACED);//?
    //set_pipe for CLOSING
    manage_fd(has_pipe, fd, 0);
    return (WIFEXITED(status) && WEXITSTATUS(status));//?
}
int main (int ac, char **av, char **envp)
{
    int i, status;
    i = 0;
    status = 0;
    (void)ac;

    while(av[i])
    {
        av += i + 1;
        i = 0;
        while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
            i++;
        if (i)
            status = ft_exec(av, i, envp); 
    }
    return (status);
}
