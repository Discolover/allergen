#include "bitset.h"
#include <assert.h>
#include <stdio.h>

#include <stdlib.h>

struct BitSet {
	unsigned long *words;
	size_t n;
};

void test1() {
	BitSet s1, s2, s3;

	s1 = bitset_new(63);
	assert(s1);

	assert(!bitset_size(s1));

	s2 = bitset_new(65);
	assert(s2);
	assert(!bitset_size(s2));

	s3 = bitset_new(0);
	assert(s3);
	assert(!bitset_size(s3));

	bitset_free(&s1);
	assert(!s1);
	bitset_free(&s2);
	assert(!s2);
}

void test2() {
	BitSet s1 = bitset_new(1001), s2 = bitset_new(1001);

	bitset_all_on(s2);

	printf("%d\n", bitset_size(s2));

	assert(bitset_len(s2) == bitset_size(s2));

	for (size_t i = 0; i < bitset_len(s2); ++i) {
		assert(bitset_is_on(s2, i));
	}

	size_t cnt = 0;
	for (size_t i = 0; i < bitset_len(s1); ++i) {
		if (rand() % 2) {
			++cnt;
			bitset_on(s1, i);
			assert(bitset_is_on(s1, i));
		} else {
			assert(!bitset_is_on(s1, i));
			bitset_off(s2, i);
		}
	}

	assert(bitset_size(s1) == cnt);
	assert(bitset_size(s2) == cnt);

	assert(bitset_len(s1) == bitset_len(s2));
	for (size_t i = 0; i < bitset_len(s1); ++i) {
		assert(bitset_is_on(s1, i) == bitset_is_on(s2, i));
	}

	bitset_all_off(s2);
	for (char *p = (char *)s2->words; p < (char *)s2->words + 128; ++p) {
		assert(*p == 0);
	}
	assert(bitset_size(s2) == 0);
}

void test3() {
	BitSet s1 = bitset_new(2000);
	BitSet s2 = bitset_new(2000);

	size_t cnt = 0;
	for (size_t i = 0; i < bitset_len(s2); ++i) {
		if (rand() % 2) {
			++cnt;
			bitset_on(s2, i);
		}
	}

	assert(bitset_size(s2) == cnt);

	bitset_or(s1, s2);
	bitset_or(s1, s2);

	assert(bitset_size(s2) == cnt);
	assert(bitset_size(s1) == cnt);

	for (size_t i = 0; i < bitset_len(s1); ++i) {
		assert(bitset_is_on(s1, i) == bitset_is_on(s2, i));
	}

	bitset_free(&s1);
	bitset_free(&s2);

	s1 = bitset_new(0);
	s2 = bitset_new(0);

	bitset_or(s1, s2);
	bitset_or(s1, s2);

	assert(bitset_size(s2) == 0);
	assert(bitset_size(s1) == 0);

	bitset_all_off(s1);
	assert(bitset_size(s1) == 0);
	bitset_all_on(s1);

	assert(bitset_size(s1) == bitset_len(s1));

	bitset_free(&s1);
	bitset_free(&s2);
}

void test_next() {
	BitSet bs = bitset_new(10);

	bitset_on(bs, 6);
	bitset_on(bs, 7);
	bitset_on(bs, 9);

	size_t i = bitset_len(bs);
	assert(bitset_next(bs, &i));
	assert(i == 6);
	assert(bitset_next(bs, &i));
	assert(i == 7);
	assert(bitset_next(bs, &i));
	assert(i == 9);
	assert(!bitset_next(bs, &i));
	assert(i == 9);


	size_t j = bitset_len(bs);
	bitset_on(bs, 0);
	assert(bitset_next(bs, &j));
	assert(j == 0);
	j = 9;
	assert(!bitset_next(bs, &j));
	assert(j == 9);

	for (size_t i = bitset_len(bs); bitset_next(bs, &i);) {
		printf("%u\n", i);
	}

	bitset_free(&bs);
}

void test4() {
	BitSet bs = bitset_new(64);

	bitset_all_on(bs);
	assert(bitset_size(bs) == 64);

	BitSet s1 = bitset_new(63);
	bitset_all_on(s1);
	assert(bitset_size(s1) == 63);

	BitSet s2 = bitset_new(65);
	bitset_all_on(s2);
	assert(bitset_size(s2) == 65);
	assert(bitset_is_on(s2, 64));

	BitSet s3 = bitset_new(3);
	bitset_all_on(s3);
	size_t i = bitset_len(s3);
	assert(bitset_next(s3, &i));
	assert(i == 0);

	assert(bitset_next(s3, &i));
	assert(i == 1);

	assert(bitset_next(s3, &i));
	assert(i == 2);

	assert(!bitset_next(s3, &i));
	assert(i == 2);

	bitset_free(&bs);
	bitset_free(&s1);
	bitset_free(&s2);
	bitset_free(&s3);
}

void test5() {
	BitSet s1 = bitset_new(0);

	assert(bitset_len(s1) == 0);

	bitset_free(&s1);
}

int main() {
	test1();
	test2();
	test3();
	test_next();
	test4();

	return 0;
}
