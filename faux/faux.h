/** @file faux.h
 * @brief Additional usefull data types and base functions.
 */

#ifndef _faux_types_h
#define _faux_types_h

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

/**
 * A standard boolean type. The possible values are
 * BOOL_FALSE and BOOL_TRUE.
 */
typedef enum {
	BOOL_FALSE = 0,
	BOOL_TRUE = 1
} bool_t;


/**
 * A tri-state boolean. The possible values are
 * TRI_FALSE, TRI_TRUE, TRI_UNDEFINED.
 */
typedef enum {
	TRI_UNDEFINED = -1,
	TRI_FALSE = 0,
	TRI_TRUE = 1
} tri_t;


/** @def C_DECL_BEGIN
 * This macro can be used instead standard preprocessor
 * directive like this:
 * @code
 * #ifdef __cplusplus
 * extern "C" {
 * #endif
 *
 * int foobar(void);
 *
 * #ifdef __cplusplus
 * }
 * #endif
 * @endcode
 * It make linker to use C-style linking for functions.
 * Use C_DECL_BEGIN before functions declaration and C_DECL_END
 * after declaration:
 * @code
 * C_DECL_BEGIN
 *
 * int foobar(void);
 *
 * C_DECL_END
 * @endcode
 */

/** @def C_DECL_END
 * See the macro C_DECL_BEGIN.
 * @sa C_DECL_BEGIN
 */

#ifdef __cplusplus
#define C_DECL_BEGIN extern "C" {
#define C_DECL_END }
#else
#define C_DECL_BEGIN
#define C_DECL_END
#endif

C_DECL_BEGIN

// Memory
void faux_free(void *ptr);
void *faux_malloc(size_t size);
void faux_bzero(void *ptr, size_t size);
void *faux_zmalloc(size_t size);

// I/O
ssize_t faux_write(int fd, const void *buf, size_t n);
ssize_t faux_read(int fd, void *buf, size_t n);
ssize_t faux_write_block(int fd, const void *buf, size_t n);
size_t faux_read_block(int fd, void *buf, size_t n);
ssize_t faux_read_whole_file(const char *path, void **data);

// Filesystem
ssize_t faux_filesize(const char *path);
bool_t faux_isdir(const char *path);
int faux_rm(const char *path);
char *faux_expand_tilde(const char *path);

C_DECL_END

#endif /* _faux_types_h */
