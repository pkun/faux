/** @file buf.c
 * @brief Dynamic buffer.
 *
 * Dynamic buffer can be written to and readed from. It grows while write
 * commands.
 *
 * User can get direct access to this buffer. For example we need to
 * read from file some data and save it to dynamic buffer. We pre-allocate
 * necessary space within buffer and lock it. Lock function returns a
 * "struct iovec" array to write to. After that we unlock buffer. So we don't
 * need additional temporary buffer beetween file's read() and dynamic buffer.
 * Dynamic buffer has the same functionality for reading from it.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "faux/faux.h"
#include "faux/str.h"
#include "faux/buf.h"

// Default chunk size
#define DATA_CHUNK 4096

struct faux_buf_s {
	faux_list_t *list; // List of chunks
	faux_list_node_t *wchunk; // Chunk to write to. NULL if list is empty
	size_t rpos; // Read position within first chunk
	size_t wpos; // Write position within wchunk (can be non-last chunk)
	size_t chunk_size; // Size of chunk
	size_t len; // Whole data length
	size_t limit; // Overflow limit
	size_t rlocked; // How much space is locked for reading
	size_t wlocked; // How much space is locked for writing
};


/** @brief Create new dynamic buffer object.
 *
 * @param [in] chunk_size Chunk size. If "0" then default size will be used.
 * @return Allocated object or NULL on error.
 */
faux_buf_t *faux_buf_new(size_t chunk_size)
{
	faux_buf_t *buf = NULL;

	buf = faux_zmalloc(sizeof(*buf));
	assert(buf);
	if (!buf)
		return NULL;

	// Init
	buf->chunk_size = (chunk_size != 0) ? chunk_size : DATA_CHUNK;
	buf->limit = FAUX_BUF_UNLIMITED;
	buf->list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, faux_free);
	buf->rpos = 0;
	buf->wpos = buf->chunk_size;
	buf->len = 0;
	buf->wchunk = NULL;
	buf->rlocked = 0; // Unlocked
	buf->wlocked = 0; // Unlocked

	return buf;
}


/** @brief Free dynamic buffer object.
 *
 * @param [in] buf Buffer object.
 */
void faux_buf_free(faux_buf_t *buf)
{
	if (!buf)
		return;

	faux_list_free(buf->list);

	faux_free(buf);
}


/** @brief Returns length of buffer.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @return Length of buffer or < 0 on error.
 */
ssize_t faux_buf_len(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return -1;

	return buf->len;
}


/** @brief Returns number of allocated data chunks.
 *
 * Function is not exported to DSO.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @return Number of allocated chunks or < 0 on error.
 */
FAUX_HIDDEN ssize_t faux_buf_chunk_num(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return -1;
	assert(buf->list);
	if (!buf->list)
		return -1;

	return faux_list_len(buf->list);
}


/** @brief Returns limit of buffer length.
 *
 * The returned "0" means unlimited.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @return Maximum buffer length or < 0 on error.
 */
ssize_t faux_buf_limit(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return -1;

	return buf->limit;
}


/** @brief Set buffer length limit.
 *
 * Writing more data than this limit will lead to error. The "0" value means
 * unlimited buffer. Default is unlimited.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @param [in] limit Maximum buffer length.
 * @return BOOL_TRUE - success, BOOL_FALSE - error.
 */
bool_t faux_buf_set_limit(faux_buf_t *buf, size_t limit)
{
	assert(buf);
	if (!buf)
		return BOOL_FALSE;

	buf->limit = limit;

	return BOOL_TRUE;
}


/** @brief Get amount of unused space within current data chunk.
 *
 * Inernal static function. Current chunk is "wchunk".
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @return Size of unused space or < 0 on error.
 */
static ssize_t faux_buf_wavail(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return -1;

	if (!buf->wchunk)
		return 0; // Empty list

	return (buf->chunk_size - buf->wpos);
}


/** @brief Get amount of available data within current data chunk.
 *
 * Inernal static function. Current chunk first chunk.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @return Size of available data or < 0 on error.
 */
static ssize_t faux_buf_ravail(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return -1;

	// Empty list
	if (buf->len == 0)
		return 0;
	// Read and write within the same chunk
	if (faux_list_head(buf->list) == buf->wchunk)
		return (buf->wpos - buf->rpos);

	// Write pointer is far away from read pointer (more than chunk)
	return (buf->chunk_size - buf->rpos);
}


/** @brief Get amount of locked space for writing.
 *
 * The "0" means that buffer is not locked for writing.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @return Size of locked space or "0" if unlocked.
 */
size_t faux_buf_is_wlocked(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return BOOL_FALSE;

	return buf->wlocked;
}


/** @brief Get amount of locked space for reading.
 *
 * The "0" means that buffer is not locked for reading.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @return Size of locked data or "0" if unlocked.
 */
size_t faux_buf_is_rlocked(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return BOOL_FALSE;

	return buf->rlocked;
}


/** @brief Allocates new chunk and adds it to the end of chunk list.
 *
 * Static internal function.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @return Newly created list node or NULL on error.
 */
static faux_list_node_t *faux_buf_alloc_chunk(faux_buf_t *buf)
{
	char *chunk = NULL;

	assert(buf);
	if (!buf)
		return NULL;
	assert(buf->list);
	if (!buf->list)
		return NULL;

	chunk = faux_malloc(buf->chunk_size);
	assert(chunk);
	if (!chunk)
		return NULL;

	return faux_list_add(buf->list, chunk);
}


/** @brief Checks if it will be overflow while writing some data.
 *
 * It uses previously set "limit" value for calculations.
 *
 * @param [in] buf Allocated and initialized buffer object.
 * @param [in] add_len Length of data we want to write to buffer.
 * @return BOOL_TRUE - it will be overflow, BOOL_FALSE - enough space.
 */
bool_t faux_buf_will_be_overflow(const faux_buf_t *buf, size_t add_len)
{
	assert(buf);
	if (!buf)
		return BOOL_FALSE;

	if (FAUX_BUF_UNLIMITED == buf->limit)
		return BOOL_FALSE;

	if ((buf->len + add_len) > buf->limit)
		return BOOL_TRUE;

	return BOOL_FALSE;
}


/** @brief Reads dynamic buffer data to specified linear buffer.
 *
 * @param [in] buf Allocated and initialized dynamic buffer object.
 * @param [in] data Linear buffer to read data to.
 * @param [in] len Length of data to read.
 * @return Length of data actually readed or < 0 on error.
 */
ssize_t faux_buf_read(faux_buf_t *buf, void *data, size_t len)
{
	struct iovec *iov = NULL;
	size_t iov_num = 0;
	ssize_t total = 0;
	char *dst = (char *)data;
	size_t i = 0;

	assert(data);
	if (!data)
		return -1;

	total = faux_buf_dread_lock(buf, len, &iov, &iov_num);
	if (total <= 0)
		return total;

	for (i = 0; i < iov_num; i++) {
		memcpy(dst, iov[i].iov_base, iov[i].iov_len);
		dst += iov[i].iov_len;
	}

	if (faux_buf_dread_unlock(buf, total, iov) != total)
		return -1;

	return total;
}


/** @brief Gets "struct iovec" array for direct reading and locks data.
 *
 * The length of actually locked data can differ from length specified by user.
 * When buffer length is less than specified length then return value will be
 * equal to buffer length.
 *
 * @param [in] buf Allocated and initialized dynamic buffer object.
 * @param [in] len Length of data to read.
 * @param [out] iov_out "struct iovec" array to direct read from.
 * @param [out] iov_num_out Number of "struct iovec" array elements.
 * @return Length of data actually locked or < 0 on error.
 */
ssize_t faux_buf_dread_lock(faux_buf_t *buf, size_t len,
	struct iovec **iov_out, size_t *iov_num_out)
{
	size_t vec_entries_num = 0;
	struct iovec *iov = NULL;
	unsigned int i = 0;
	faux_list_node_t *iter = NULL;
	size_t len_to_lock = 0;
	size_t avail = 0;
	size_t must_be_read = 0;

	assert(buf);
	if (!buf)
		return -1;
	assert(iov_out);
	if (!iov_out)
		return -1;
	assert(iov_num_out);
	if (!iov_num_out)
		return -1;

	// Don't use already locked buffer
	if (faux_buf_is_rlocked(buf))
		return -1;

	len_to_lock = (len < buf->len) ? len : buf->len;
	// Nothing to lock
	if (0 == len_to_lock) {
		*iov_out = NULL;
		*iov_num_out = 0;
		return 0;
	}

	// Calculate number of struct iovec entries
	avail = faux_buf_ravail(buf);
	if (avail > 0)
		vec_entries_num++;
	if (avail < len_to_lock) {
		size_t l = buf->len - avail; // length w/o first chunk
		vec_entries_num += l / buf->chunk_size;
		if ((l % buf->chunk_size) > 0)
			vec_entries_num++;
	}
	iov = faux_zmalloc(vec_entries_num * sizeof(*iov));

	// Iterate chunks. Suppose list is not empty
	must_be_read = len_to_lock;
	iter = NULL;
	while (must_be_read > 0) {
		char *data = NULL;
		off_t data_offset = 0;
		size_t data_len = buf->chunk_size;
		size_t p_len = 0;

		// First chunk
		if (!iter) {
			iter = faux_list_head(buf->list);
			if (avail > 0) {
				data_offset = buf->rpos;
				data_len = avail; // Calculated earlier
			} else { // Empty chunk. Go to next
				iter = faux_list_next_node(iter);
			}
		// Not-first chunks
		} else {
			iter = faux_list_next_node(iter);
		}

		data = (char *)faux_list_data(iter) + data_offset;
		p_len = (must_be_read < data_len) ? must_be_read : data_len;

		must_be_read -= p_len;
		iov[i].iov_base = data;
		iov[i].iov_len = p_len;
		i++;
	}

	*iov_out = iov;
	*iov_num_out = vec_entries_num;
	buf->rlocked = len_to_lock;

	return len_to_lock;
}


/** @brief Frees "struct iovec" array and unlocks read data.
 *
 * The length of actually readed data can be less than length of locked data.
 * In this case all the data will be unlocked but only actually readed length
 * will be removed from buffer.
 *
 * Function gets "struct iovec" array to free it. It was previously allocated
 * by faux_dread_lock() function.
 *
 * @param [in] buf Allocated and initialized dynamic buffer object.
 * @param [in] really_readed Length of data actually read.
 * @param [out] iov "struct iovec" array to free.
 * @param [out] iov_num_out Number of "struct iovec" array elements.
 * @return Length of data actually unlocked or < 0 on error.
 */
ssize_t faux_buf_dread_unlock(faux_buf_t *buf, size_t really_readed,
	struct iovec *iov)
{
	size_t must_be_read = really_readed;

	assert(buf);
	if (!buf)
		return -1;
	// Can't unlock non-locked buffer
	if (!faux_buf_is_rlocked(buf))
		return -1;

	if (buf->rlocked < really_readed)
		return -1; // Something went wrong
	if (buf->len < really_readed)
		return -1; // Something went wrong

	if (0 == really_readed)
		goto unlock;

	// Suppose list is not empty
	while (must_be_read > 0) {
		size_t avail = faux_buf_ravail(buf);
		ssize_t data_to_rm = (must_be_read < avail) ? must_be_read : avail;
		faux_list_node_t *iter = faux_list_head(buf->list);

		buf->len -= data_to_rm;
		buf->rpos += data_to_rm;
		must_be_read -= data_to_rm;

		// Current chunk was fully readed. So remove it from list.
		// Chunk is not wchunk
		if ((iter != buf->wchunk) &&
			(buf->rpos == buf->chunk_size)) {
			buf->rpos = 0; // 0 position within next chunk
			faux_list_del(buf->list, iter);
			if (faux_buf_chunk_num(buf) == 0) { // Empty list w/o locks
				buf->wchunk = NULL;
				buf->wpos = buf->chunk_size;
			}
		// Chunk is wchunk
		} else if ((iter == buf->wchunk) &&
			(buf->rpos == buf->wpos) &&
			(!buf->wlocked ||  // Chunk can be locked for writing
			(buf->wpos == buf->chunk_size))) { // Chunk can be filled
			buf->rpos = 0; // 0 position within next chunk
			buf->wchunk = NULL;
			buf->wpos = buf->chunk_size;
			faux_list_del(buf->list, iter);
		}
	}

unlock:
	// Unlock whole buffer. Not 'really readed' bytes only
	buf->rlocked = 0;
	faux_free(iov);

	return really_readed;
}


/** @brief Write data from linear buffer to dynamic buffer.
 *
 * @param [in] buf Allocated and initialized dynamic buffer object.
 * @param [in] data Linear buffer. Source of data.
 * @param [in] len Length of data to write.
 * @return Length of data actually written or < 0 on error.
 */
ssize_t faux_buf_write(faux_buf_t *buf, const void *data, size_t len)
{
	struct iovec *iov = NULL;
	size_t iov_num = 0;
	ssize_t total = 0;
	char *src = (char *)data;
	size_t i = 0;

	assert(data);
	if (!data)
		return -1;

	total = faux_buf_dwrite_lock(buf, len, &iov, &iov_num);
	if (total <= 0)
		return total;

	for (i = 0; i < iov_num; i++) {
		memcpy(iov[i].iov_base, src, iov[i].iov_len);
		src += iov[i].iov_len;
	}

	if (faux_buf_dwrite_unlock(buf, total, iov) != total)
		return -1;

	return total;
}


/** @brief Gets "struct iovec" array for direct writing and locks data.
 *
 * @param [in] buf Allocated and initialized dynamic buffer object.
 * @param [in] len Length of data to lock.
 * @param [out] iov_out "struct iovec" array to direct write to.
 * @param [out] iov_num_out Number of "struct iovec" array elements.
 * @return Length of data actually locked or < 0 on error.
 */
ssize_t faux_buf_dwrite_lock(faux_buf_t *buf, size_t len,
	struct iovec **iov_out, size_t *iov_num_out)
{
	size_t vec_entries_num = 0;
	struct iovec *iov = NULL;
	unsigned int i = 0;
	faux_list_node_t *iter = NULL;
	size_t avail = 0;
	size_t must_be_write = len;

	assert(buf);
	if (!buf)
		return -1;
	assert(iov_out);
	if (!iov_out)
		return -1;
	assert(iov_num_out);
	if (!iov_num_out)
		return -1;

	// Don't use already locked buffer
	if (faux_buf_is_wlocked(buf))
		return -1;

	// It will be overflow after writing
	if (faux_buf_will_be_overflow(buf, len))
		return -1;

	// Nothing to lock
	if (0 == len) {
		*iov_out = NULL;
		*iov_num_out = 0;
		return 0;
	}

	// Write lock
	buf->wlocked = len;

	// Calculate number of struct iovec entries
	avail = faux_buf_wavail(buf);
	if (avail > 0)
		vec_entries_num++;
	if (avail < len) {
		size_t i = 0;
		size_t new_chunk_num = 0;
		size_t l = len - avail; // length w/o first chunk
		new_chunk_num += l / buf->chunk_size;
		if ((l % buf->chunk_size) > 0)
			new_chunk_num++;
		vec_entries_num += new_chunk_num;
		for (i = 0; i < new_chunk_num; i++)
			faux_buf_alloc_chunk(buf);
	}
	iov = faux_zmalloc(vec_entries_num * sizeof(*iov));
	assert(iov);

	// Iterate chunks
	iter = buf->wchunk;
	i = 0;
	while ((must_be_write > 0)) {
		char *data = NULL;
		off_t data_offset = 0;
		size_t data_len = buf->chunk_size;
		size_t p_len = 0;

		// List was empty before writing
		if (!iter) {
			iter = faux_list_head(buf->list);
		// Not empty list. First element
		} else if (iter == buf->wchunk) {
			size_t l = faux_buf_wavail(buf);
			if (0 == l) { // Not enough space within current chunk
				iter = faux_list_next_node(iter);
			} else {
				data_offset = buf->wpos;
				data_len = l;
			}
		// Not empty list. Fully free chunk
		} else {
			iter = faux_list_next_node(iter);
		}

		p_len = (must_be_write < data_len) ? must_be_write : data_len;
		data = (char *)faux_list_data(iter) + data_offset;
		must_be_write -= p_len;
		iov[i].iov_base = data;
		iov[i].iov_len = p_len;
		i++;
	}

	*iov_out = iov;
	*iov_num_out = vec_entries_num;

	return len;
}


/** @brief Frees "struct iovec" array and unlocks written data.
 *
 * The length of actually written data can be less than length of locked data.
 * In this case all the data will be unlocked but only actually written length
 * will be stored within buffer.
 *
 * Function gets "struct iovec" array to free it. It was previously allocated
 * by faux_dwrite_lock() function.
 *
 * @param [in] buf Allocated and initialized dynamic buffer object.
 * @param [in] really_written Length of data actually written.
 * @param [out] iov "struct iovec" array to free.
 * @param [out] iov_num_out Number of "struct iovec" array elements.
 * @return Length of data actually unlocked or < 0 on error.
 */
ssize_t faux_buf_dwrite_unlock(faux_buf_t *buf, size_t really_written,
	struct iovec *iov)
{
	size_t must_be_write = really_written;

	assert(buf);
	if (!buf)
		return -1;
	// Can't unlock non-locked buffer
	if (!faux_buf_is_wlocked(buf))
		return -1;

	if (buf->wlocked < really_written)
		return -1; // Something went wrong

	while (must_be_write > 0) {
		size_t avail = 0;
		ssize_t data_to_add = 0;

		avail = faux_buf_wavail(buf);
		// Current chunk was fully written. So move to next one
		if (0 == avail) {
			buf->wpos = 0; // 0 position within next chunk
			if (buf->wchunk)
				buf->wchunk = faux_list_next_node(buf->wchunk);
			else
				buf->wchunk = faux_list_head(buf->list);
			avail = faux_buf_wavail(buf);
		}
		data_to_add = (must_be_write < avail) ? must_be_write : avail;

		buf->len += data_to_add;
		buf->wpos += data_to_add;
		must_be_write -= data_to_add;
	}

	if (buf->wchunk) {
		faux_list_node_t *iter = NULL;
		// Remove trailing empty chunks after wchunk
		while ((iter = faux_list_next_node(buf->wchunk)))
			faux_list_del(buf->list, iter);
		// When really_written == 0 then all data can be read after
		// dwrite_lock() and dwrite_unlock() so chunk can be empty.
		if ((faux_list_head(buf->list) == buf->wchunk) &&
			(buf->wpos == buf->rpos)) {
			faux_list_del(buf->list, buf->wchunk);
			buf->wchunk = NULL;
			buf->wpos = buf->chunk_size;
		}
	}

	// Unlock whole buffer. Not 'really written' bytes only
	buf->wlocked = 0;
	faux_free(iov);

	return really_written;
}
