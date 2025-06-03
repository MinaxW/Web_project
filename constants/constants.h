#ifndef CONSTANTS_H
#define CONSTANTS_H

// Базовые пути
#define DIR_TEMPLATES "src/templates/"
#define DIR_CSS       "src/css/"

// Пути к HTML-шаблонам
#define PATH_LOGIN_HTML DIR_TEMPLATES   "login.html"
#define PATH_SUCCESS_HTML DIR_TEMPLATES "main_screen.html"
#define PATH_ERROR_HTML DIR_TEMPLATES   "error.html"
#define PATH_RESULT_HTML DIR_TEMPLATES "result.html"
#define PATH_NOT_VALID_HTML DIR_TEMPLATES "not_valid.html"

// Путь к CSS
#define PATH_LOGIN_CSS DIR_CSS "login.css"
#define PATH_ERROR_CSS DIR_CSS "error.css"
#define PATH_MAIN_SCREEN_CSS DIR_CSS "main_screen.css"
#define PATH_RESULT_CSS DIR_CSS "result.css"

//Content-Type заголовки
#define CONTENT_TYPE_HTML "Content-Type: text/html; charset=utf-8\r\n"
#define CONTENT_TYPE_CSS  "Content-Type: text/css; charset=utf-8\r\n"

#endif
