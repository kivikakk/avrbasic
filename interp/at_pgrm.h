#ifndef AT_PGRM_H
#define AT_PGRM_H

#define MAX_LINE_LEN 80

void init_pheap(void);
void add_line(uint16_t lno, char const *line, char *err);
int get_line(uint16_t lno, char line[MAX_LINE_LEN], char *err);

extern uint16_t MIN_LINE, MAX_LINE;

#endif
