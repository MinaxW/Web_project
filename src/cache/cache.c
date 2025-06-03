#include "cache.h"
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

// Глобальная таблица кэша
CacheEntry cache_table[CACHE_SIZE];

// Переменная флага инициализации кэша
bool is_cache_initialized = false;

// Простая хеш-функция для двух временных строк
unsigned long hash_function(const char *sleep_time, const char *wake_time){
    unsigned long hash = 5381;
    size_t iter;
    for(iter = 0; sleep_time[iter]; ++iter)
        hash = ((hash << 5) + hash) + sleep_time[iter]; /* hash * 33 + c */
    
    for(iter = 0; wake_time[iter]; ++iter)
        hash = ((hash << 5) + hash) + wake_time[iter]; /* hash * 33 + c */
        
    return hash;
}

// Инициализация кэша
void init_cache(){
    memset(cache_table, 0, sizeof(cache_table));
    is_cache_initialized = true;
}

// Поиск элемента в кэше
CacheEntry* find_in_cache(const char *sleep_time, const char *wake_time){
    if (!is_cache_initialized) return NULL;
    
    unsigned long index = hash_function(sleep_time, wake_time) % CACHE_SIZE;
    CacheEntry *found_entry = NULL;
    bool done = false;
    
    while (!done) {
        if(strcmp(cache_table[index].sleep_time, "") == 0){
            done = true;
        } else if(strcmp(cache_table[index].sleep_time, sleep_time) == 0 &&
                   strcmp(cache_table[index].wake_time, wake_time) == 0){
            found_entry = &cache_table[index];
            done = true;
        } else {
            index = (index + 1) % CACHE_SIZE;
        }
    }
    
    return found_entry;
}

// Вставка нового элемента в кэш
void insert_into_cache(const char *sleep_time, const char *wake_time, struct tm result){
    if (!is_cache_initialized) return;
    
    unsigned long index = hash_function(sleep_time, wake_time) % CACHE_SIZE;
    while(strcmp(cache_table[index].sleep_time, "") != 0){
        index = (index + 1) % CACHE_SIZE;
    }
    
    strcpy(cache_table[index].sleep_time, sleep_time);
    strcpy(cache_table[index].wake_time, wake_time);
    memcpy(&(cache_table[index].result), &result, sizeof(struct tm));
}
