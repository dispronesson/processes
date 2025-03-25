#include "functions.h"

int main(int argc, char* argv[], char* envp[]) {
    if (!getenv("CHILD_PATH")) { //Существует ли переменная CHILD_PATH
        fprintf(stderr, "error: CHILD_PATH is not set\n");
        return 1;
    }

    setlocale(LC_COLLATE, "C"); //Установка локали

    int count = 0;
    while (envp[count] != NULL) { //Подсчет кол-ва переменных окружения
        count++;
    }
    qsort(envp, count, sizeof(char*), compare_env); //Сортировка массива переменных окружения

    //Вывод информации о текущем процессе
    printf("name: parent\tpid: %d\tppid: %d\n", getpid(), getppid());
    printf("env info:\n");
    for (int i = 0; i < count; i++) {
        puts(envp[i]);
    }

    interface(argv[0], envp); //Вход в цикл обработки нажатия клавиш

    return 0;
}