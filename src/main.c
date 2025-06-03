#include "../mongoose/mongoose.h"
#include "../input/input.h"
#include "../constants/constants.h"
#include "../src/cache/cache.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/**
 * @brief Рассчитывает продолжительность сна исходя из времени засыпания и пробуждения.
 * @param sleep_time Время отхода ко сну (формат "часы:минуты").
 * @param wake_time Время пробуждения (формат "часы:минуты").
 * @return Структура tm с рассчитанным временем сна (час/минуты). Возвращается нулевое значение в случае ошибки.
 */
struct tm calculate_sleep_time(const char *sleep_time,
                               const char *wake_time){
    struct tm sleep_tm, wake_tm, result = {0};
    char *sleep_time_str                = strdup(sleep_time);
    char *wake_time_str                 = strdup(wake_time);
    bool error                          = false;

    if (sscanf(sleep_time_str, "%d:%d", &sleep_tm.tm_hour,
        &sleep_tm.tm_min) != 2 || sleep_tm.tm_hour < 0 ||
        sleep_tm.tm_hour > 23 || sleep_tm.tm_min < 0 ||
	sleep_tm.tm_min > 59 || sscanf(wake_time_str, "%d:%d",
	&wake_tm.tm_hour, &wake_tm.tm_min) != 2 ||
	wake_tm.tm_hour < 0 || wake_tm.tm_hour > 23 ||
	wake_tm.tm_min < 0 || wake_tm.tm_min > 59)
	    error = true;

    if(!error){
        // Преобразуем время в секунды
        int sleep_seconds = sleep_tm.tm_hour * 3600 + sleep_tm.tm_min * 60;
        int wake_seconds  = wake_tm.tm_hour * 3600 + wake_tm.tm_min * 60;

        // Если время пробуждения меньше или равно
        // времени отхода ко сну, добавляем 24 часа
        if (wake_seconds <= sleep_seconds){
            wake_seconds += 24 * 3600;
        }

        // Вычисляем разницу
        int total_seconds = wake_seconds - sleep_seconds;
        int hours         = total_seconds / 3600;
        int minutes       = (total_seconds % 3600) / 60;

        // Возвращаем результат
        result.tm_hour = hours;
        result.tm_min  = minutes;
    }
    return result;
}

/**
 * @brief Перечисляет возможные коды ошибок.
 */
enum {
    ERR_OK                  = 0,
    // Ошибки нет, всё прошло успешно
    ERR_MISSING_ENV         = 1,
    // Не установлены переменные окружения LOGIN_USER или LOGIN_PASS
    ERR_INVALID_CREDENTIALS = 2,
    // Неверное имя пользователя или пароль
    ERR_FILE_NOT_FOUND      = 3
    // Не удалось открыть нужный файл
};

// Устанавливает стандартные параметры для успешной обработки запроса
static void set_success_status(int *status_code, int *error_code) {
    *status_code = 200;
    *error_code = ERR_OK;
}

static void login_handler(struct mg_http_message *hm, char **response, int *status_code,
			  const char **ctype, int *error_code){
    char username[100], password[100]; // Буферы для хранения введённого имени пользователя и пароля    
    const char *expected_user = getenv("LOGIN_USER"); // Получение ожидаемого имени из переменной окружения 
    const char *expected_pass = getenv("LOGIN_PASS"); // Получение ожидаемого пароля из переменной окружения

    if (!expected_user || !expected_pass)
        *error_code = ERR_MISSING_ENV;
        // Если переменные окружения не заданы — ошибка
    else{
        // Извлечение значений из тела POST-запроса
        mg_http_get_var(&hm->body,
                        "username", username, sizeof(username));
        mg_http_get_var(&hm->body,
                        "password", password, sizeof(password));

        // Проверка введённых данных
        if (!strcmp(username, expected_user) &&
            !strcmp(password, expected_pass))
            *response = read_file(PATH_SUCCESS_HTML);
            // Если всё верно — читаем HTML с приветствием
        else{
            *response = read_file(PATH_ERROR_HTML);
            // Иначе — HTML с ошибкой
            *error_code = ERR_INVALID_CREDENTIALS;
        }
  
        // Если файл успешно прочитан — подготавливаем ответ
        if (*response){
            set_success_status(status_code, error_code);
	    *ctype = CONTENT_TYPE_HTML;
	}
    }
}

struct tm check_cache_and_calculate(const char *sleep_time, const char *wake_time){
    CacheEntry *cached_result = find_in_cache(sleep_time, wake_time);
    struct tm result;

    if (cached_result != NULL)
	result = cached_result->result;
    else {
        result = calculate_sleep_time(sleep_time, wake_time);
        if(result.tm_hour != 0 || result.tm_min != 0)
            insert_into_cache(sleep_time, wake_time, result);
    }
    return result;
}

static void calculate_handler(struct mg_http_message *hm, char **response,
			      int *status_code, const char **ctype, int *error_code){
    char sleep_time[10], wake_time[10];
    mg_http_get_var(&hm->body, "sleep-time",
                    sleep_time, sizeof(sleep_time));
    mg_http_get_var(&hm->body, "wake-time",
                    wake_time, sizeof(wake_time));

    struct tm result = check_cache_and_calculate(sleep_time, wake_time);
    if(result.tm_hour == 0 && result.tm_min == 0)
        *response = read_file(PATH_NOT_VALID_HTML);

    else{
        char result_str[100];
        snprintf(result_str, sizeof(result_str),
             "Вы поспали <strong>%d</strong> часов и <strong>%d</strong> минут.",
              result.tm_hour, result.tm_min);

        char comment[100];
        if (result.tm_hour >= 8){
            snprintf(comment, sizeof(comment), "Отлично! Вы поспали достаточно.");
        }
        else if (result.tm_hour >= 5 && result.tm_hour <= 7){
            snprintf(comment, sizeof(comment), "Маловато. Человеку нужно спать от 8 часов в день.");
        }
        else if (result.tm_hour < 5){
            snprintf(comment, sizeof(comment), "Плохо. У вас недосып.");
         }

         char *html_template = read_file(PATH_RESULT_HTML);

        asprintf(response, html_template, result_str, comment);
    }
    
    if (*response){
        set_success_status(status_code, error_code);
	*ctype = CONTENT_TYPE_HTML;
    }
}

static void default_handler(char **response, int *status_code,
			    const char **ctype, int *error_code){
    *response = read_file(PATH_LOGIN_HTML);
     // Загружаем страницу логина
    if (*response){
        set_success_status(status_code, error_code);
	*ctype = CONTENT_TYPE_HTML;
    }
}

/**
 * @brief Обрабатывает входящие HTTP-запросы и формирует ответ.
 * @param c Соединение Mongoose.
 * @param hm Сообщение HTTP-запроса.
 * @return Целое число, обозначающее код ошибки: ERR_OK при успехе, иначе другой код ошибки.
 */
static int process_request(struct mg_connection *c, struct mg_http_message *hm){
    int status_code   = 500; // Код состояния по умолчанию — внутренняя ошибка сервера    
    const char *ctype = ""; // Тип содержимого (Content-Type) будет выбран позже    
    char *response    = NULL; // Ответ, который будет отправлен клиенту    
    int error_code    = ERR_FILE_NOT_FOUND; // Ошибка по умолчанию — файл не найден

    // Проверка: если пришёл POST-запрос на /login
    if (!mg_strcmp(hm->uri, mg_str("/login")) &&
        !mg_strcasecmp(hm->method, mg_str("POST")))
        login_handler(hm, &response, &status_code, &ctype, &error_code);

    else if (!mg_strcmp(hm->uri, mg_str("/calculate")) &&
             !mg_strcasecmp(hm->method, mg_str("POST")))
        calculate_handler(hm, &response, &status_code, &ctype, &error_code);

    // Если запрашивается файл стилей
    else if (!mg_strcmp(hm->uri, mg_str("/login.css"))){
        response = read_file(PATH_LOGIN_CSS);
        if (response){
            set_success_status(&status_code, &error_code);
	    ctype = CONTENT_TYPE_CSS;
        }
    }

    else if (!mg_strcmp(hm->uri, mg_str("/error.css"))){
        response = read_file(PATH_ERROR_CSS);        
	if (response){
            set_success_status(&status_code, &error_code);
	    ctype = CONTENT_TYPE_CSS;
        }
    }

    else if (!mg_strcmp(hm->uri, mg_str("/main_screen.css"))){
        response = read_file(PATH_MAIN_SCREEN_CSS);
        if (response){
           set_success_status(&status_code, &error_code);
	   ctype = CONTENT_TYPE_CSS;
	}
    }

    else if (!mg_strcmp(hm->uri, mg_str("/result.css"))){
        response = read_file(PATH_RESULT_CSS);
        if (response){
            set_success_status(&status_code, &error_code);
	    ctype = CONTENT_TYPE_CSS;
	}
    }

    // Если запрашивается что-либо другое — возвращаем login.html
    else
        default_handler(&response, &status_code, &ctype, &error_code);

    // Если не было ошибок — отправляем ответ клиенту
    if (error_code == ERR_OK)
        mg_http_reply(c, status_code, ctype, "%s", response);
    else
        mg_http_reply(c, 500, "", "");
        // В случае ошибки — пустой ответ с кодом 500

    free(response); // Освобождение выделенной памяти
    return error_code; // Возврат кода ошибки
}

/**
 * @brief Главная функция обработки событий сервера.
 * @param c Соединение Mongoose.
 * @param ev Код события.
 * @param ev_data Данные события.
 */
static void main_fun(struct mg_connection *c,
                     int ev,
                     void *ev_data) {

    if (ev == MG_EV_HTTP_MSG) { // Если произошло событие HTTP-запроса

        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        // Преобразуем данные события к нужному типу
        process_request(c, hm); // Передаём запрос в обработчик

        // Здесь можно добавить логирование результата обработки
        // printf("Результат обработки: %d\n", result);
    }
}

/**
 * @brief Основная точка входа приложения.
 * @return 0 при нормальном завершении работы.
 */
int main(void) {
    init_cache();
    const char *server_address = "http://localhost:8081";
    // Адрес, по которому будет запущен сервер

    struct mg_mgr mgr; // Контекст менеджера соединений
    mg_mgr_init(&mgr); // Инициализация менеджера
    mg_http_listen(&mgr, server_address, main_fun, NULL);
    // Запуск сервера на указанном адресе и обработчиком событий

    printf("Сервер запущен на %s\n", server_address);
    // Вывод сообщения в консоль

    for (;;) mg_mgr_poll(&mgr, 1000);
    // Основной цикл обработки событий с интервалом 1000 мс

    mg_mgr_free(&mgr);
    // Очистка ресурсов перед завершением
    // (не выполнится из-за бесконечного цикла)

    return 0; // Завершение программы
}
