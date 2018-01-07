#ifndef AT_DISPLAY_H
#define AT_DISPLAY_H

void init_display(void);
char getch(void);
void flush(void);
void putch(char c);
void putstr(char const *s);

#define GETLN_LEN 80
int getln(char line[GETLN_LEN]);

#endif
