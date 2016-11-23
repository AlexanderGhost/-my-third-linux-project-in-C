#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<linux/types.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_LINE 80

int main(void) {
	char *args[MAX_LINE / 2 + 1]; // Массив для хранения аргументов коммандной строки
	char buffer[80];					// буфер для входа данных пользователем
	char *token = NULL;	// массив лексем
	char *whiteSpace = " \t\n\f\r\v";		// массив символов пробелов

	int pipefd[2];
	if (pipe(pipefd) == -1) {
		perror("pipe");
		return -1;
	}

	int c, i, fd;// переменная для хранения одного символа ввода, чтобы завершить выполнение в нужный момент
	do {
		fflush(stdout);	// оставляем входной поток открытым
		printf("input> ");					//ввод данных с клавиатуры
		fflush(stdout);
		scanf(" %[^\n]", buffer);	//читаем, что было введено

		int count = 0;// переменная для хранения кол-ва параметров коммандной строки
		char *fileName = NULL;
		token = strtok(buffer, whiteSpace);
		args[count] = strdup(token);// получаем первую строку с помощью strdup(?)

		count += 1;	//читаем введенные данные, циклично повторяя  высше описанную операцию
		while ((token = strtok(NULL, whiteSpace)) != NULL) {
			args[count] = strdup(token);
			count += 1;
		}
		args[count] = NULL;	// завершаем формированную строку, которую должны передать функции execvp

		for (i = 0; i < count; i++) {
			if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL) {
				fileName = (char *) malloc(strlen(args[i + 1]));
				strcpy(fileName, args[i + 1]);
				args[i] = 0;
				args[i + 1] = 0;
				fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
				if (fd == -1) {
					perror("open");
					return EXIT_FAILURE;
				}
				break;
			}
		}
		pid_t pid = fork();					// создаем процесс
		if (pid == 0) {						// мы в дочерном процессе, тут вызываем execvp и проверяем успешность
			close(pipefd[0]);
			if (fileName != NULL) {
				if (dup2(fd, STDOUT_FILENO) == -1) {
					perror("dup2");
					return EXIT_FAILURE;
				}
			}
			if ((execvp(args[0], args)) == -1) {
				fprintf(stderr, "Failed to execvp() '%s' (%d: %s)\n", args[0],
						errno, strerror(errno));
				exit(1);
			}
		} else if (pid > 0) {				// а тут мы в родительском процессе
			if (args[count - 1] == "&")		// ожидаем
				wait();
			else
				waitpid(pid, 0, 0);
		} else {
			exit(1);
		}

	} while ((c = getchar()) != EOF);// завершаем программы если введен символ EOF
	return 0;
}

//execvp(argv[0], argv);
//fprintf(stderr, "Failed to execvp() '%s' (%d: %s)\n", argv[0], errno, strerror(errno));
