#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//Подключение POSIX-совместимых функций и расширение X/Open
#define _XOPEN_SOURCE 700

//Подключение библиотек
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <locale.h>
#include <linux/limits.h>
#include <libgen.h>
#include <termios.h>

//Объявление прототипов функций
int compare_env(const void* a, const void* b);

char* get_env_path(const char* path);

char** read_env(const char* path);

void read_env_child(const char* path);

char* create_name_child(int count);

char getch();

void interface(const char* path);

#endif //FUNCTIONS_H