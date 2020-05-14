#include "faux/faux.h"
#include "faux/file.h"


// Chunk class

struct faux_chunk_s {
	void *buf; // Allocated buffer
	size_t size; // Size of allocated buffer
	void *start; // Pointer to data start
	void *end; // Pointer to data end
};

typedef struct faux_chunk_s faux_chunk_t;

C_DECL_BEGIN

faux_chunk_t *faux_chunk_new(size_t size);
void faux_chunk_free(faux_chunk_t *chunk);
size_t faux_chunk_len(faux_chunk_t *chunk);
ssize_t faux_chunk_inc_len(faux_chunk_t *chunk, size_t inc_len);
ssize_t faux_chunk_dec_len(faux_chunk_t *chunk, size_t dec_len);
ssize_t faux_chunk_size(faux_chunk_t *chunk);
void *faux_chunk_buf(faux_chunk_t *chunk);
void *faux_chunk_write_pos(faux_chunk_t *chunk);
void *faux_chunk_read_pos(faux_chunk_t *chunk);
ssize_t faux_chunk_left(faux_chunk_t *chunk);

C_DECL_END


// File class

/** @brief Chunk size to allocate buffer */
#define FAUX_FILE_CHUNK_SIZE 128

struct faux_file_s {
	int fd; // File descriptor
	char *buf; // Data buffer
	size_t buf_size; // Current buffer size
	size_t len; // Current data length
	bool_t eof; // EOF flag
};
