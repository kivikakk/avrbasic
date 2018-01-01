#ifndef AT_DISPLAY_H
#define AT_DISPLAY_H

void init_display(void);
char getch(void);
void flush(void);
void putch(char c);
void putstr(char const *s);

#define GETLINE_LEN 80
int getline(char line[GETLINE_LEN]);

#endif
