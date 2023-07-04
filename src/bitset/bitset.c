#include "bitset.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

struct BitSet {
	unsigned long *words;
	size_t len;
};

#define BPW (sizeof(unsigned long) * 8)

#define CEIL_DIV(A, B) (((A) + (B) - 1) / (B))

BitSet bitset_new(size_t len) {
	BitSet bs;

	bs = calloc(1, sizeof(struct BitSet));
	assert(bs);

	PRINT("CEIL_DIV(%u/%u) = %u\n", len, BPW, CEIL_DIV(len, BPW));

	if (len) {
		bs->words = calloc(CEIL_DIV(len, BPW), sizeof(unsigned long));
	}

	bs->len = len;

	return bs;
}

size_t bitset_len(BitSet bs) {
	assert(bs);
	return bs->len;
}

void bitset_on(BitSet bs, size_t index) {
	assert(bs && index < bs->len);

	size_t i = index - (index / BPW) * BPW;

	bs->words[index / BPW] |= ((unsigned long)1 << i);
}

void bitset_off(BitSet bs, size_t index) {
	assert(bs && index < bs->len);

	size_t i = index - (index / BPW) * BPW;

	bs->words[index / BPW] &= ~((unsigned long)1 << i);
}

bool bitset_is_on(BitSet bs, size_t index) {
	assert(bs && index < bs->len);

	size_t i = index - (index / BPW) * BPW;

	return bs->words[index / BPW] & ((unsigned long)1 << i);
}

void bitset_or(BitSet dst, BitSet src) {
	assert(dst && src && dst->len == src->len);

	for (size_t i = 0; i < CEIL_DIV(dst->len, BPW); ++i) {
		assert(dst->len);
		assert(src->len);
		dst->words[i] |= src->words[i];
	}
}

size_t bitset_size(BitSet bs) {
	assert(bs);

	//0 0000 1 0001 2 0010 3 0011
	//4 0100 5 0101 6 0110 7 0111
	//8 1000 9 1001 A 1010 B 1011
	//C 1100 D 1101 E 1110 F 1111
	static char hex_to_nbits[0x10] = {
		0, 1, 1, 2,
		1, 2, 2, 3,
		1, 2, 2, 3,
		2, 3, 3, 4
	};

	size_t n = CEIL_DIV(bs->len, BPW) * sizeof(unsigned long);

	size_t size = 0;
	for (char *b = (char *)bs->words; b < (char *)bs->words + n; ++b) {
		size += hex_to_nbits[*b & 0x0f];
		size += hex_to_nbits[*b >> 4 & 0x0f];
	}

	return size;
}

bool bitset_next(BitSet bs, size_t *index) {
	assert(bs && *index <= bs->len);

	size_t i;

	if (*index == bs->len) {
		i = 0;
	} else {
		i = *index + 1;
	}

	for (; i < bs->len; ++i) {
		if (bitset_is_on(bs, i)) {
			*index = i;
			return true;
		}
	}

	return false;
}

void bitset_all_on(BitSet bs) {
	assert(bs);
	if (!bs->len) {
		return;
	}

	memset(bs->words, ~0, CEIL_DIV(bs->len, BPW) * sizeof(unsigned long));

	size_t gap = CEIL_DIV(bs->len, BPW) * BPW - bs->len;

	size_t last = CEIL_DIV(bs->len, BPW) - 1;
	bs->words[last] &= ~(unsigned long)0 >> gap;
}

void bitset_all_off(BitSet bs) {
	assert(bs);
	if (!bs->len) {
		return;
	}
	memset(bs->words, 0, CEIL_DIV(bs->len, BPW) * sizeof(unsigned long));
}

void bitset_free(BitSet *bs) {
	assert(bs && *bs);

	free((*bs)->words);
	free(*bs);
	*bs = NULL;
}
