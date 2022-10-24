#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "apperr.h"
#include "applim.h"
#include "card.h"
#include "parse.h"

#define INCNLINE(n) do { \
	ECHK(maxnl - (*nline) < (n), AENLINE, return -1); \
	(*nline) += (n); \
} while (0)

#define CHK_VALSZ(n) ECHK(val - field->val >= VALSZ - (n), AEVALSZ, \
	return -1)

#define TRIM() do { \
	if (field && val != field->val && val[-1] == '\n') \
		val[-1] = '\0'; \
} while (0)

int readcard(FILE *fp, struct card *card, int *nline, int maxnl)
{
	char line[LINESZ], *val;
	int n, sep, nblank, ch;
	struct field *field, *i;

	*nline = 0;
	while ((ch = fgetc(fp)) == '\n')
		INCNLINE(1);
	ECHK(ungetc(ch, fp) != ch, AESYS, return -1);
	field = NULL, val = NULL;
	memset(card, 0, sizeof *card);
	while (fgets(line, sizeof line, fp)) {
		INCNLINE(1);
		n = strlen(line);
		ECHK(line[n - 1] != '\n' && !feof(fp), AELINESZ,
			return -1);
		if (line[0] == '\t') {	/* successive value line */
			ECHK(!field, AENOKEY, return -1);
			CHK_VALSZ(n);
			val = stpcpy(val, line);
		} else if (line[0] != '\n') {	/* new field key */
			ECHK(++card->nfield > NFIELD, AENFIELD,
				return -1);
			/*TRIM();*/
			ECHK(field && validfield(field), apperr,
				return -1);
			field = &card->field[card->nfield - 1];
			val = field->val;
			sep = strcspn(line, "\n\t");
			ECHK(sep >= KEYSZ, AEKEYSZ, return -1);
			strncpy(field->key, line, sep);
			for (i = card->field; i != field; i++)
				ECHK(!strcmp(field->key, i->key),
					AEDUPKEY, return -1);
			if (!line[sep])
				continue;
			CHK_VALSZ(n - sep);
			val = stpcpy(val, &line[sep]);
		} else {	/* blank line */
			ECHK(ungetc('\n', fp) != '\n', AESYS,
				return -1);
			nblank = 0;
			(*nline)--;
			while ((ch = fgetc(fp)) == '\n') {
				INCNLINE(1);
				nblank++;
			}
			ECHK(ungetc(ch, fp) != ch, AESYS, return -1);
			if (ch != '\t')
				break;
			ECHK(!field, AENOKEY, return -1);
			CHK_VALSZ(nblank);
			while (nblank-- > 0)
				*val++ = '\n';
			*val = '\0';
		}
	}
	ECHK(ferror(fp), AESYS, return -1);
	if (card->nfield) {
		/*TRIM();*/
		ECHK(validfield(field), apperr, return -1);
		ECHK(validcard(card), apperr, return -1);
	}
	return card->nfield;
}

int writecard(FILE *fp, struct card *card)
{
	int i;

	for (i = 0; i < card->nfield; i++)
		ECHK(fprintf(fp, "%s%s", card->field[i].key,
			card->field[i].val) < 0, AESYS, return -1);
	return 0;
}
