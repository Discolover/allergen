#include <stdbool.h>
#include <stdlib.h>

typedef struct BitSet *BitSet;

BitSet bitset_new(size_t len);

void bitset_on(BitSet bs, size_t index);
void bitset_off(BitSet bs, size_t index);
bool bitset_is_on(BitSet bs, size_t index);
size_t bitset_len(BitSet bs);

size_t bitset_size(BitSet bs);

void bitset_all_off(BitSet bs);
void bitset_all_on(BitSet bs);

void bitset_or(BitSet dst, BitSet src);

void bitset_free(BitSet *bs);
bool bitset_next(BitSet bs, size_t *index);
