#include "functions.h"

//Компаратор для qsort
int compare_env(const void* a, const void* b) {
    return strcoll(*(const char**)a, *(const char**)b);
}

//Функция получения полного пути откуда запустилась программа
char* get_env_path(const char* path) {
    char prog_path[PATH_MAX];
    char* env_file_path;

    if (!realpath(path, prog_path)) { //Получение полного пути
        perror("realpath");
        exit(EXIT_FAILURE);
    }

    char* dir = dirname(prog_path); //Получение директории
    env_file_path = malloc(strlen(dir) + 12);
    if (!env_file_path) {
        perror("malloc failure");
        abort();
    }

    //Создание пути к файлу .env
    strcpy(env_file_path, dir);
    strcat(env_file_path, "/../../.env");

    return env_file_path;
}

//Функция чтения файла .env и создания массива окружения
char** read_env(const char* path) {
    FILE* file = fopen(path, "r"); //Открытие файла
    if (!file) {
        perror("open file failure");
        exit(EXIT_FAILURE);
    }

    int size = 0;
    char key[64];

    while (fgets(key, sizeof(key), file)) { //Подсчет кол-ва ключей
        size++;
    }
    fseek(file, 0, SEEK_SET); //Перемещение в начало файла

    //Выделение памяти под массив окружения
    char** envp_child = malloc((size + 1) * sizeof(char*));
    if (!envp_child) {
        perror("malloc failure");
        abort();
    }

    int line_count = 0;
    while (fgets(key, sizeof(key), file)) { //Чтение ключа из файла
        if (key[strlen(key) - 1] == '\n') { //Замена \n на \0
            key[strlen(key) - 1] = '\0';
        }

        char* value = getenv(key); //Получение значения из окружения
        //Если отсутствует, то .env файл неправильно настроен 
        //или значениене для ключа отсутствует
        if(!value) {
            fprintf(stderr, "error with .env file or value of key\n");
            exit(EXIT_FAILURE);
        }

        //Выделение памяти под строку ключ=значение
        envp_child[line_count] = malloc(strlen(key) + strlen(value) + 2);
        if (!envp_child) {
            perror("malloc failure");
            abort();
        }

        //Копирование строки
        strcpy(envp_child[line_count], key);
        strcat(envp_child[line_count], "=");
        strcat(envp_child[line_count], value);
        line_count++;
    }
    envp_child[line_count] = NULL; //Окончание массива окружения NULL

    fclose(file); //Закрытие файла

    return envp_child;
}

//Функция чтения файла .env, которая сразу выводит в stdout строки ключ=значение
void read_env_child(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("open file failure");
        exit(EXIT_FAILURE);
    }

    char key[64];

    while (fgets(key, sizeof(key), file)) {
        if (key[strlen(key) - 1] == '\n') {
            key[strlen(key) - 1] = '\0';
        }

        char* value = getenv(key);
        if(!value) {
            fprintf(stderr, "error with .env file or with value of key\n");
            exit(EXIT_FAILURE);
        }

        char* envp = malloc(strlen(key) + strlen(value) + 2);
        if (!envp) {
            perror("malloc failure");
            abort();
        }

        strcpy(envp, key);
        strcat(envp, "=");
        strcat(envp, value);

        puts(envp); //Вывод строки на консоль
        free(envp); //Очистка памяти
    }

    fclose(file);
}

//Функция создания имени для дочернего процесса
char* create_name_child(int count) {
    char* name = malloc(9); //Выделение памяти
    if (!name) {
        perror("malloc failure");
        abort();
    }
    
    //Запись имени в массив
    snprintf(name, 9, "child_%02d", count);

    return name;
}

//Функция чтения одного символа из stdin без канонического режима и эхо ввода
char getch() {
    struct termios oldt, newt; //Структуры управления терминалом
    char ch;

    tcgetattr(STDIN_FILENO, &oldt); //Получение текущих настроек терминала
    newt = oldt; //Копирование настроек

    newt.c_lflag &= ~(ICANON | ECHO); //Отключение канонического режима и эхо ввода

    tcsetattr(STDIN_FILENO, TCSANOW, &newt); //Сохранение настроек

    ch = getchar(); //Чтение символа

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); //Восстановление старых настроек
    
    return ch;
}

//Функция обработки нажатия клавиш
void interface(const char* path) {
    char* env_path = get_env_path(path);
    char** envp_child = read_env(env_path);

    char ch;
    char* child_path = getenv("CHILD_PATH");

    int count = 0;
    while (1) {
        ch = getch();
        if (ch == '+' || ch == '*' || ch == '&') { //Проверка корректности ввода
            if (count < 100) { //Проверка кол-ва созданных дочерних процессов
                char ch_str[2] = {ch, '\0'}; //Создание строки из символа
                char* name = create_name_child(count);
                pid_t pid = fork(); //Создание дочернего процесса
    
                if (pid < 0) { //Если процесс не создался..
                    fprintf(stderr, "error with fork function\n");
                    exit(EXIT_FAILURE);
                }
                else if (pid == 0) { //Если это дочерний процесс
                    char* args[] = {name, ch_str, NULL, NULL}; //Создание массива argv
                    if (ch == '+') {
                        args[2] = env_path; //Добавление пути к .env файлу
                        execve(child_path, args, envp_child); //Вызов функции, заменяющая текущий процесс
                    }
                    else {
                        execve(child_path, args, envp_child);
                    }

                    //Если execve завершится ошибкой, этот код выполнится
                    perror("CHILD_PATH");

                    //Если была ошибка, обязательно надо очистить память
                    for (int i = 0; envp_child[i] != NULL; i++) {
                        free(envp_child[i]);
                    }

                    free(envp_child);
                    free(name);
                    free(env_path);

                    exit(EXIT_FAILURE);
                }
                free(name);
                count++;
            }
            else {
                fprintf(stderr, "error: can't create more than 100 child processes\n");
            }
        }
        else if (ch == 'q') break;
        else {
            printf("Invalid operator.\n");
        }
    }

    while (wait(NULL) > 0); //Завершение всех дочерних процессов

    //Очистка памяти
    for (int i = 0; envp_child[i] != NULL; i++) {
        free(envp_child[i]);
    }
    free(envp_child);

    free(env_path);
}
