#ifndef CARD_H
#define	CARD_H

#include <time.h>
#include "applim.h"

struct field {
	char *key;
	char *val;
};

struct card {
	int nfield, leadnewl, trainewl;
	struct field field[NFIELD];
	char *sep;
};

void destrcard(struct card *card);
char *getmod(struct card *card);
char *getques(struct card *card);
char *getansw(struct card *card);
int getprev(struct card *card, time_t *tp);
int getnext(struct card *card, time_t *tp);
int setprev(struct card *card, time_t prev);
int setnext(struct card *card, time_t next);
int parsetm(char *s, time_t *tp);
int validkey(char *key);
int validfield(struct field *field);
int validcard(struct card *card);
char *normval(char *s, char *buf, int n);

#endif
