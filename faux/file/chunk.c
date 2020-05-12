#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "private.h"

faux_chunk_t *faux_chunk_new(size_t size) {

	faux_chunk_t *chunk = NULL;

	if (0 == size) // Illegal 0 size
		return NULL;

	chunk = faux_zmalloc(sizeof(*chunk));
	if (!chunk)
		return NULL;

	// Init
	chunk->buf = faux_zmalloc(size);
	if (!chunk->buf) {
		faux_free(chunk);
		return NULL;
	}
	chunk->size = size;
	chunk->start = chunk->buf;
	chunk->end = chunk->buf;

	return chunk;
}


void faux_chunk_free(faux_chunk_t *chunk) {

	// Without assert()
	if (!chunk)
		return;

	faux_free(chunk->buf);
	faux_free(chunk);
}


size_t faux_chunk_len(faux_chunk_t *chunk) {

	assert(chunk);
	if (!chunk)
		return 0;

	return (chunk->end - chunk->start);
}


ssize_t faux_chunk_inc_len(faux_chunk_t *chunk, size_t inc_len) {

	assert(chunk);
	if (!chunk)
		return -1;
	assert((faux_chunk_len(chunk) + inc_len) <= chunk->size);
	if ((faux_chunk_len(chunk) + inc_len) > chunk->size)
		return -1;
	chunk->end += inc_len;

	return faux_chunk_len(chunk);
}


ssize_t faux_chunk_dec_len(faux_chunk_t *chunk, size_t dec_len) {

	assert(chunk);
	if (!chunk)
		return -1;
	assert(faux_chunk_len(chunk) >= dec_len);
	if (faux_chunk_len(chunk) < dec_len)
		return -1;
	chunk->start += dec_len;

	return faux_chunk_len(chunk);
}


ssize_t faux_chunk_size(faux_chunk_t *chunk) {

	assert(chunk);
	if (!chunk)
		return -1;

	return chunk->size;
}


void *faux_chunk_buf(faux_chunk_t *chunk) {

	assert(chunk);
	if (!chunk)
		return NULL;

	return chunk->buf;
}


void *faux_chunk_write_pos(faux_chunk_t *chunk) {

	assert(chunk);
	if (!chunk)
		return NULL;

	return chunk->end;
}


void *faux_chunk_read_pos(faux_chunk_t *chunk) {

	assert(chunk);
	if (!chunk)
		return NULL;

	return chunk->start;
}


ssize_t faux_chunk_left(faux_chunk_t *chunk) {

	assert(chunk);
	if (!chunk)
		return -1;

	return (chunk->buf + chunk->size - chunk->end);
}
