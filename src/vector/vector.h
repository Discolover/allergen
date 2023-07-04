typedef struct Vector *Vector;

Vector vector_new(size_t len, size_t cap, size_t sz);
void vector_append(Vector v, void *elem);
void vector_get(Vector v, size_t index, void *out);
void vector_set(Vector v, size_t index, void *elem);

void vector_extend(Vector dst, Vector src);

void vector_len_set(Vector v, size_t len);

size_t vector_len(Vector v);

size_t vector_cap(Vector v);

void vector_free(Vector *v);

#define VECTOR_NEW(TYPE) (vector_new(0, 8, sizeof(TYPE)))
