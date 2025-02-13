//thanks to julsanti

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> // Necessario per waitpid()

#define MAX_ARGS 10  // Numero massimo di argomenti per comando

/**
 * @brief Divide una stringa di comando in un array di argomenti.
 *
 * @param command Stringa del comando con argomenti separati da spazi.
 * @param argv Array di puntatori a carattere che conterrà gli argomenti.
 */
void parse_command(char *command, char *argv[]) {
    int i = 0;
    char *token = strtok(command, " "); // Divide la stringa sugli spazi

    while (token != NULL && i < MAX_ARGS - 1) {
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    argv[i] = NULL; // L'ultimo elemento deve essere NULL per execvp()
}

/**
 * @brief Programma che esegue due comandi in sequenza utilizzando una pipe.
 *
 * Il programma prende due comandi come argomenti da riga di comando
 * e li esegue in una pipeline, dove il primo comando scrive sulla pipe
 * e il secondo legge dalla pipe.
 *
 * Funzionamento:
 * - Il processo padre crea una pipe.
 * - Crea due processi figli con `fork()`.
 * - Il primo figlio reindirizza `stdout` alla pipe e esegue il primo comando.
 * - Il secondo figlio reindirizza `stdin` alla pipe e esegue il secondo comando.
 * - Il padre chiude la pipe e attende la terminazione dei figli.
 *
 * @param argc Numero di argomenti da riga di comando (deve essere almeno 3).
 * @param argv Array di stringhe contenente i comandi (es. `./pipe_exec "ls -l" "grep txt"`).
 * @return int 0 se il programma termina correttamente, 1 in caso di errore.
 */
int main(int argc, char *argv[], char *envp[])
{
    if (argc < 3) {
        fprintf(stderr, "Use: %s \"comando1\" \"comando2\"\n", argv[0]);
        exit(1);
    }

    int fd[2]; // Pipe per la comunicazione tra i processi
    pid_t pid1, pid2;

    // Creazione della pipe
    if (pipe(fd) == -1) {
        perror("Errore nella creazione della pipe");
        exit(1);
    }

    // Array per contenere i comandi suddivisi in argomenti
    char *cmd1[MAX_ARGS], *cmd2[MAX_ARGS];

    // Divide i comandi in array di argomenti
    parse_command(argv[1], cmd1);
    parse_command(argv[2], cmd2);

    // Primo processo (esegue il primo comando)
    pid1 = fork();
    if (pid1 < 0) {
        perror("Errore nella creazione del primo processo");
        exit(1);
    }

    if (pid1 == 0) {
        // Processo figlio 1: esegue il primo comando e scrive nella pipe
        close(fd[0]); // Chiude la lettura della pipe

        // Reindirizza stdout alla pipe (scrittura)
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]); // Chiude il descrittore originale

        // Esegue il primo comando con i suoi argomenti
        execvp(cmd1[0], cmd1);
        perror("Errore nell'esecuzione del primo comando");
        exit(1);
    }

    // Secondo processo (esegue il secondo comando)
    pid2 = fork();
    if (pid2 < 0) {
        perror("Errore nella creazione del secondo processo");
        exit(1);
    }

    if (pid2 == 0) {
        // Processo figlio 2: legge dalla pipe e esegue il secondo comando
        close(fd[1]); // Chiude la scrittura della pipe

        // Reindirizza stdin alla pipe (lettura)
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]); // Chiude il descrittore originale

        // Esegue il secondo comando con i suoi argomenti
        execvp(cmd2[0], cmd2);
        perror("Errore nell'esecuzione del secondo comando");
        exit(1);
    }

    // Processo padre
    close(fd[0]); // Chiude la pipe (non necessaria per il padre)
    close(fd[1]);

    // Attende la terminazione dei processi figli
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}