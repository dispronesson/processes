#include "functions.h"

extern char** environ; //Переменная, содержащая массив окружения

int main(int argc, char* argv[], char* envp[]) {
    //Вывод информации о дочернем процессе на экран
    printf("\nchild name: %s\tpid: %d\tppid: %d\n", argv[0], getpid(), getppid());

    printf("env info:\n");
    //В зависимости от нажатой клавиши, выполняется соответствующая команда
    if (argv[1][0] == '+') {
        read_env_child(argv[2]);
    }
    else if (argv[1][0] == '*') {
        for (int i = 0; envp[i] != NULL; i++) {
            puts(envp[i]);
        }
    }
    else {
        for (int i = 0; environ[i] != NULL; i++) {
            puts(environ[i]);
        }
    }

    return 0;
}