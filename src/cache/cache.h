#ifndef CACHE_H
#define CACHE_H

#include <time.h>

// Максимальная длина строки для времени (hh:mm)
#define MAX_TIME_LEN 10

// Размер таблицы кэша
#define CACHE_SIZE 1024

// Структура для хранения одного элемента кэша
typedef struct {
    char sleep_time[MAX_TIME_LEN+1];
    char wake_time[MAX_TIME_LEN+1];
    struct tm result;               
} CacheEntry;

// Внешняя глобальная таблица кэша
extern CacheEntry cache_table[];

// Инициализирует таблицу кэша
void init_cache();

// Найти элемент в кэше по ключу
CacheEntry* find_in_cache(const char *sleep_time, const char *wake_time);

// Вставить новый элемент в кэш
void insert_into_cache(const char *sleep_time, const char *wake_time, struct tm result);

#endif // CACHE_H