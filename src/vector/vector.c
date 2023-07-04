#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct Vector {
	char *data;
	size_t cap;
	size_t len;
	size_t sz;
};

typedef struct Vector *Vector;

static void resize(Vector v);

Vector vector_new(size_t len, size_t cap, size_t sz) {
	assert(cap && sz);
	assert(len <= cap);

	Vector v = calloc(1, sizeof(struct Vector));

	v->data = calloc(cap, sz);
	v->cap = cap;
	v->len = len;
	v->sz = sz;

	return v;
}

static void resize(Vector v) {
	size_t old = v->cap;
	v->cap *= 2;
	v->data = realloc(v->data, v->cap * v->sz);
	memset(v->data + old * v->sz, 0, (v->cap - old) * v->sz);
}

void vector_append(Vector v, void *elem) {
	assert(v && elem);

	if (v->len >= v->cap) {
		assert(v->len == v->cap); // sanity check
		resize(v);
	}

	memcpy(v->data + v->len * v->sz, elem, v->sz);
	++v->len;
}

void vector_extend(Vector dst, Vector src) {
	assert(dst && src);
	assert(dst->sz == src->sz);

	while ((dst->cap - dst->len) < src->len) {
		resize(dst);
	}

	memcpy(dst->data + dst->len * dst->sz, src->data, src->len * src->sz);

	dst->len += src->len;
}

void vector_get(Vector v, size_t index, void *out) {
	assert(v && out);
	assert(index < v->len);

	memcpy(out, v->data + index * v->sz, v->sz);
}

void vector_set(Vector v, size_t index, void *elem) {
	assert(v && elem);
	assert(index < v->len);

	memcpy(v->data + index * v->sz, elem, v->sz);
}

size_t vector_len(Vector v) {
	if (!v) {
		return 0;
	}

	return v->len;
}

void vector_len_set(Vector v, size_t len) {
	assert(v && len <= v->cap);

	v->len = len;
}

size_t vector_cap(Vector v) {
	if (!v) {
		return 0;
	}

	return v->cap;
}

void vector_free(Vector *v) {
	assert(v);

	free((*v)->data);
	free(*v);
	*v = NULL;
}
