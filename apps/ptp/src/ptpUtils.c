/*
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "ptpCompact.h"

static int verbose = 0;
static int print_level = LOG_INFO;
static int use_syslog = 1;

static const char *progname;
static const char *message_tag;

void print_set_progname(const char *name)
{
	progname = name;
}

void print_set_tag(const char *tag)
{
	message_tag = tag;
}

void print_set_syslog(int value)
{
	use_syslog = value ? 1 : 0;
}

void print_set_level(int level)
{
	print_level = level;
}

void print_set_verbose(int value)
{
	verbose = value ? 1 : 0;
}

void print(int level, char const *format, ...)
{
	struct timespec ts;
	va_list ap;
	char buf[1024];
	FILE *f;

	if (level > print_level)
		return;

	PTP_GET_SYS_TIME_MONOTONIC(&ts);

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	if (verbose) {
		f = level >= LOG_NOTICE ? stdout : stderr;
		fprintf(f, "%s[%ld.%03ld]: %s%s%s\n",
			progname ? progname : "",
			ts.tv_sec, ts.tv_nsec / 1000000,
			message_tag ? message_tag : "", message_tag ? " " : "",
			buf);
		fflush(f);
	}
	if (use_syslog) {
		syslog(level, "[%ld.%03ld] %s%s%s",
		       ts.tv_sec, ts.tv_nsec / 1000000,
		       message_tag ? message_tag : "", message_tag ? " " : "",
		       buf);
	}
}


#include <stdlib.h>
#include <string.h>


#define HASH_TABLE_SIZE 200

struct HashNode
{
	char				*key;
	void				*data;
	struct HashNode	*next;
};

struct hash
{
	struct HashNode *table[HASH_TABLE_SIZE];
};

static unsigned int hash_function(const char* s)
{
	unsigned int i;

	for (i = 0; *s; s++) {
		i = 131 * i + *s;
	}
	return i % HASH_TABLE_SIZE;
}

struct hash *hash_create(void)
{
	struct hash *ht = calloc(1, sizeof(*ht));
	return ht;
}

void hash_destroy(struct hash *ht, void (*func)(void *))
{
	unsigned int i;
	struct HashNode *n, *next, **table = ht->table;

	for (i = 0; i < HASH_TABLE_SIZE; i++)
	{
		for (n = table[i] ; n; n = next)
		{
			next = n->next;
			if (func)
			{
				func(n->data);
			}
			
			free(n->key);
			free(n);
		}
	}

	free(ht);
}

int hash_insert(struct hash *ht, const char* key, void *data)
{
	unsigned int h;
	struct HashNode *n, **table = ht->table;

	h = hash_function(key);

	for (n = table[h] ; n; n = n->next)
	{
		if (!strcmp(n->key, key))
		{/* reject duplicate keys */
			return -1;
		}
	}
	n = calloc(1, sizeof(*n));
	if (!n)
	{
		return -1;
	}
	n->key = strdup(key);
	if (!n->key)
	{
		free(n);
		return -1;
	}
	n->data = data;
	n->next = table[h];
	table[h] = n;
	
	return 0;
}

void *hash_lookup(struct hash *ht, const char* key)
{
	unsigned int h;
	struct HashNode *n, **table = ht->table;

	h = hash_function(key);

	for (n = table[h] ; n; n = n->next)
	{
		if (!strcmp(n->key, key))
		{
			return n->data;
		}
	}
	return NULL;
}

#include <errno.h>

#define parent(x)	(((1 + (x)) >> 1) - 1)
#define left(x)		(((1 + (x)) << 1) - 1)
#define right(x)	(((1 + (x)) << 1))

struct pqueue {
	int len;
	int max;
	int (*cmp)(void *a, void *b);
	void **data;
};

static int pq_greater(struct pqueue *q, int a, int b)
{
	return q->cmp(q->data[a], q->data[b]) > 0 ? 1 : 0;
}

static void heapify(struct pqueue *q, int index)
{
	int i_max = index;
	int left = left(index);
	int right = right(index);

	if (left < q->len) {
		if (pq_greater(q, left, i_max))
			i_max = left;
	}

	if (right < q->len && pq_greater(q, right, i_max)) {
		i_max = right;
	}

	if (i_max != index) {
		void *tmp = q->data[index];
		q->data[index] = q->data[i_max];
		q->data[i_max] = tmp;
		heapify(q, i_max);
	}
}

/* public methods */

struct pqueue *pqueue_create(int max_length,
			     int (*compare)(void *a, void *b))
{
	struct pqueue *q = calloc(1, sizeof(*q));
	if (!q) {
		return NULL;
	}
	q->len = 0;
	q->max = max_length;
	q->cmp = compare;
	q->data = calloc(max_length, sizeof(void *));
	if (!q->data) {
		free(q);
		return NULL;
	}
	return q;
}

void pqueue_destroy(struct pqueue *q)
{
	free(q->data);
	free(q);
}

void *pqueue_extract(struct pqueue *q)
{
	void *data;

	if (!q->len) {
		return NULL;
	}
	data = q->data[0];
	q->data[0] = q->data[q->len - 1];
	q->len--;
	heapify(q, 0);

	return data;
}

int pqueue_insert(struct pqueue *q, void *d)
{
	int index;

	if (q->len >= q->max) {
		void **buf = realloc(q->data, 2 * q->max * sizeof(void *));
		if (buf) {
			q->data = buf;
			q->max *= 2;
		} else {
			return -ENOMEM;
		}
	}
	index = q->len;
	q->len++;

	while (index && (q->cmp(q->data[parent(index)], d) < 0)) {
		q->data[index] = q->data[parent(index)];
		index = parent(index);
	}
	q->data[index] = d;

	return 0;
}

int pqueue_length(struct pqueue *q)
{
	return q->len;
}

void *pqueue_peek(struct pqueue *q)
{
	return q->len ? q->data[0] : (void *) 0;
}


#include "ptpProtocol.h"

static const char *fault_type_str[FT_CNT] = {
	"FT_UNSPECIFIED",
	"FT_BAD_PEER_NETWORK",
	"FT_SWITCH_PHC",
};

const char *ft_str(enum fault_type ft)
{
	if (ft < 0 || ft >= FT_CNT)
		return "INVALID_FAULT_TYPE_ENUM";
	return fault_type_str[ft];
}

static const char *version = STRINGIFY(VER);

void version_show(FILE *fp)
{
	fprintf(fp, "%s\n", version);
}

const char *version_string(void)
{
	return version;
}

