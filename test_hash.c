/* $Id: tstavl.c,v 1.8 2014/01/22 08:01:21 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Thu Aug 13 19:38:00     2009
 * Copyright: (C) 2009 Luis Colorado.  All rights reserved.
 * License: BSD.
 */

#define IN_TSTAVL_C

/* Standard include files */
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "hash.h"

#define FLAG_USEPAGER (1 << 0)
static int flags = 0;
static char *pager = "less";

void help()
{
	printf(
		"test_hash: test sobre el funcionamiento de la tabla hash\n"
		"(c) 2012 Luis Colorado.\n"
		"All rights reseved.\n"
		"Ayuda:\n"
		"+id   añade <id,null> a la tabla.\n"
		"-id   elimina <id,null> de la tabla. Si no existe no hace nada.\n"
		"?id   consulta si <id,...> existe en la tabla.\n"
		"=id   consulta el valor asociado a la entrada.\n"
		"#     devuelve el número de entradas de la tabla.\n"
		"!     devuelve el numero de colisiones de la tabla.\n"
		"%%     borra todas las entradas de la tabla.\n"
		".     termina el programa.\n"
	);
} /* help */

static struct timespec beg_ts;
static int			   beg_res;
void set_timestamp()
{

	beg_res = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &beg_ts);
	if (beg_res < 0) {
		fprintf(stderr, "ERROR: clock_gettime: %s (errno = %d)\n",
			strerror(errno), errno);
	} /* if */
} /* set_timestamp */

void print_timestamp()
{
	struct timespec end_ts;
	int res = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_ts);
	end_ts.tv_nsec -= beg_ts.tv_nsec;
	end_ts.tv_sec -= beg_ts.tv_sec;
	if (end_ts.tv_nsec < 0) {
		end_ts.tv_nsec += 1000000000L;
		end_ts.tv_sec--;
	} /* if */
	printf("CPU time elapsed: %llu.%09lu\n",
		(unsigned long long) end_ts.tv_sec,
		(unsigned long) end_ts.tv_nsec);
} /* print_timestamp */

size_t my_hash(const char *k)
{
	printf("my_hash(%s) -> ", k);
	int i;
	size_t ret_val = 13;
	for (i = 0; (i < 3) && *k; i++) {
		ret_val *= 11;
		ret_val += *k++;
	}
	printf("%lu\n", ret_val);
	return ret_val;
}

int my_equals(const char *a, const char *b)
{
	int res = strcmp(a, b) == 0;
	printf("my_equals(%s, %s) -> %s\n",
		a, b, res ? "true" : "false");
	return res;
}

/* main program */
int main (int argc, char **argv)
{
	char buffer[1024];

	hash_t *t = new_hash(113, my_hash, my_equals);
	long NN = 0;
	int opt;
	while((opt = getopt(argc, argv, "ph")) != EOF) {
		switch (opt) {
			case 'h': default:
					  help(); exit(EXIT_SUCCESS); break; 
		} /* switch */
	} /* while */

	help();
	while (fgets(buffer, sizeof buffer, stdin)) {
		char *p = strtok(buffer, "\n");
		switch (*p) {
		case '-': p++;
            set_timestamp();
			if (!ht_remove(t, p))
				printf("Error: no puedo borrar [%s]\n", p);
            print_timestamp();
			continue;
		case '?': p++;
            set_timestamp();
			printf("%s: %s\n", p,
				ht_get(t, p)
					? "TRUE"
					: "FALSE");
            print_timestamp();
			continue;
		case '+':
            p++;
            set_timestamp();
            ht_put(t, p, (void *)++NN);
            print_timestamp();
            continue;
		default:
            printf("ERROR: entrada inválida: %s\n",
                    p);
            help(); continue;
		case '#':
            set_timestamp();
			printf("#: %zd\n", ht_get_size(t));
            print_timestamp();
			continue;
        case '!':
            set_timestamp();
            printf("!: %zu\n", ht_get_collisions(t));
            print_timestamp();
            continue;
		case '=': { p++;
			set_timestamp();
			long i = (long) ht_get(t, p);
			printf("Busco \"%s\" -> [%lu]\n", buffer, i);
			print_timestamp();
			continue;
                  }
		case '%':
			set_timestamp();
			free_hash(t);
			print_timestamp();
			t = new_hash(113, my_hash, my_equals);
			NN = 0;
			continue;
		case '.': goto exit;
		} /* switch */
		/*NOTREACHED*/
	} /* while */

exit:
	printf(
		"fin del programa\n"
	);

	return 0;
} /* main */

/* $Id: tstavl.c,v 1.8 2014/01/22 08:01:21 luis Exp $ */
/* vim: ts=4 sw=4 nu
 */
