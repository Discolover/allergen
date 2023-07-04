#include <stdio.h>
#include <assert.h>
#include "vector.h"
#include "string.h"

struct Vector {
	char *data;
	size_t cap;
	size_t len;
	size_t sz;
};

typedef struct Vector *Vector_t;

void test1() {
	Vector v = NULL;

	assert(vector_len(v) == 0);
	assert(vector_cap(v) == 0);
	v = vector_new(0, 8, sizeof(int));
	assert(vector_cap(v) == 8);

	int tmp;
	for (int i = 0; i < 8; ++i) {
		tmp = (i + 1) * 2;
		vector_append(v, &tmp);
	}
	vector_free(&v);
	assert(v == NULL);
}

void test2() {
	Vector v;
	int tmp;

	v = vector_new(10, 10, sizeof(int));
	assert(vector_len(v) == 10);
	assert(vector_cap(v) == 10);

	for (int i = 0; i < 100; ++i) {
		tmp = i * 10;
		vector_append(v, &tmp);
	}

	for (int i = 0; i < 10; ++i) {
		vector_get(v, i, &tmp);
		assert(tmp == 0);
	}

	vector_cap(v);
	vector_len(v);
	for (int i = 10; i < 110; ++i) {
		vector_get(v, i, &tmp);
		assert(tmp == (i - 10) * 10);
	}

	vector_free(&v);
}

void test3() {
	struct Dummy {
		int n;
		char *s;
	} tmp;

	// assert(vector_new(10, 10, 0) == NULL);
	// assert(vector_new(10, 0, 10) == NULL);
	// assert(vector_new(10, 0, 0) == NULL);
	// assert(vector_new(6, 5, sizeof(struct Dummy)) == NULL);

	Vector v;

	v = vector_new(1, 1, sizeof(struct Dummy));
	assert(vector_len(v) == 1);
	assert(vector_cap(v) == 1);

	tmp.n = 4;
	tmp.s = "four";
	vector_append(v, &tmp);
	assert(vector_len(v) == 2);
	assert(vector_cap(v) == 2);

	tmp.n = 5;
	tmp.s = "five";
	vector_append(v, &tmp);
	assert(vector_len(v) == 3);
	assert(vector_cap(v) == 4);

	tmp.n = 6;
	tmp.s = "six";
	vector_append(v, &tmp);
	assert(vector_len(v) == 4);
	assert(vector_cap(v) == 4);

	// assert(vector_get(v, 4, &tmp) < 0);
	// assert(vector_get(NULL, 0, &tmp) < 0);
	// assert(vector_get(NULL, 4, NULL) < 0);
	// assert(vector_get(v, 4, NULL) < 0);

	vector_get(v, 3, &tmp);
	assert(tmp.n == 6);
	assert(!strcmp(tmp.s, "six"));

	vector_get(v, 0, &tmp);
	assert(tmp.n == 0);
	assert(tmp.s == NULL);

	vector_get(v, 2, &tmp);
	assert(tmp.n == 5);
	assert(!strcmp(tmp.s, "five"));

	vector_get(v, 1, &tmp);
	assert(tmp.n == 4);
	assert(!strcmp(tmp.s, "four"));

	tmp.n = 1;
	tmp.s = "one";
	vector_append(v, &tmp);
	assert(vector_len(v) == 5);
	assert(vector_cap(v) == 8);

	tmp.n = 2;
	tmp.s = "two";
	vector_set(v, 0, &tmp);
	assert(vector_len(v) == 5);
	assert(vector_cap(v) == 8);

	vector_get(v, 4, &tmp);
	assert(tmp.n == 1);
	assert(!strcmp(tmp.s, "one"));

	vector_get(v, 0, &tmp);
	assert(tmp.n == 2);
	assert(!strcmp(tmp.s, "two"));

	vector_free(&v);
}

void test4() {
	Vector v;

	v = vector_new(300, 300, sizeof(int));

	int tmp;
	for (size_t i = 0; i < vector_len(v); ++i) {
		tmp = i + 1;
		vector_set(v, i, &tmp);
	}
	assert(vector_len(v) == 300);
	assert(vector_cap(v) == 300);

	for (int i = vector_len(v) - 1; i >= 0 ; --i) {
		vector_get(v, i, &tmp);
		assert(tmp == i + 1);
	}

	tmp = 301;
	vector_append(v, &tmp);
	assert(vector_len(v) == 301);
	assert(vector_cap(v) == 600);

	vector_free(&v);
}

void test_vector_extend() {
	struct Dummy {
		char bar[64];
		int foo;
	} d;

	Vector_t v1 = VECTOR_NEW(struct Dummy);
	Vector_t v2 = VECTOR_NEW(struct Dummy);

	for (size_t i = 0; i < 30; ++i) {
		d.foo = (i + 1) * 3;
		vector_append(v2, &d);
	}
	assert(v2->len == 30);
	assert(v2->cap == 32);

	for (size_t i = 30; i < 32; ++i) {
		memcpy(&d, v2->data + i * v2->sz, v2->sz);
		assert(d.foo == 0);
	}

	vector_extend(v1, v2);

	for (int i = vector_len(v1) - 1; i >= 0; --i) {
		vector_get(v1, i, &d);
		assert(d.foo == (i + 1) * 3);
	}

	for (size_t i = 30; i < 32; ++i) {
		memcpy(&d, v1->data + i * v1->sz, v1->sz);
		assert(d.foo == 0);
	}

	vector_extend(v1, v2);
	assert(v1->len == 60);
	assert(v1->cap == 64);

	for (size_t i = 60; i < 63; ++i) {
		memcpy(&d, v1->data + i * v1->sz, v1->sz);
		assert(d.foo == 0);
	}

	for (size_t i = 0; i < v1->len; ++i) {
		vector_get(v1, i, &d);
		assert(d.foo == ((int)i % 30 + 1) * 3);
	}

	vector_extend(v2, v1);
	assert(v2->len == 90);
	assert(v2->cap == 128);

	for (size_t i = 90; i < 128; ++i) {
		struct Dummy t;
		memcpy(&t, v2->data + i * v2->sz, v2->sz);
		assert(t.foo == 0);
		for (int j = 0; j < 64; ++j) {
			assert(t.bar[j] == 0);
		}
	}

	for (size_t i = 0; i < vector_len(v2); ++i) {
		struct Dummy t;
		vector_get(v2, i, &t);
		assert(t.foo == ((int)i % 30 + 1) * 3);
	}

	vector_free(&v1);
	vector_free(&v2);
}

int main() {
	test1();
	test2();
	test3();
	test4();
	test_vector_extend();

	printf("ALL TESTS PASSED\n");

	return 0;
}
