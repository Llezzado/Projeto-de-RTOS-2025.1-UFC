// webpage.h
#ifndef WEBPAGE_H
#define WEBPAGE_H

extern const char *HTML_CONTENT;

const char *HTML_CONTENT = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n"
"\r\n"
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <title>Estufa Automatizada</title>\n"
"    <style>\n"
"        body { font-family: Arial; text-align: center; }\n"
"        h1 { color: #118911ff; }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <h1>Estufa Automatizada</h1>\n"
"    <h2>Exposição solar diaria: /6horas</h2>\n"
"    <h2>Umidade: 60%</h2>\n"
"</body>\n"
"</html>\r\n";

#endif