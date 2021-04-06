/** @file buf.c
 * @brief Dynamic buffer.
 *
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
	faux_list_node_t *wchunk; // Chunk to write to
	size_t rpos; // Read position within first chunk
	size_t wpos; // Write position within wchunk (can be non-last chunk)
	size_t chunk_size; // Size of chunk
	size_t len; // Whole data length
	size_t limit; // Overflow limit
	size_t rlocked;
	size_t wlocked;
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


ssize_t faux_buf_len(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return -1;

	return buf->len;
}


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


ssize_t faux_buf_limit(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return -1;

	return buf->limit;
}


/** @brief Set size limit.
 *
 * Read limits define conditions when the read callback will be executed.
 * Buffer must contain data amount greater or equal to "min" value. Callback
 * will not get data amount greater than "max" value. If min == max then
 * callback will be executed with fixed data size. The "max" value can be "0".
 * It means indefinite i.e. data transferred to callback can be really large.
 *
 * @param [in] buf Allocated and initialized buf I/O object.
 * @param [in] min Minimal data amount.
 * @param [in] max Maximal data amount. The "0" means indefinite.
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
 * Inernal static function.
 *
 * @param [in] list Internal buffer (list of chunks) to inspect.
 * @param [in] pos Current write position within last chunk
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


size_t faux_buf_is_wlocked(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return BOOL_FALSE;

	return buf->wlocked;
}


size_t faux_buf_is_rlocked(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return BOOL_FALSE;

	return buf->rlocked;
}


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


/** @brief buf data write.
 *
 * All given data will be stored to internal buffer (list of data chunks).
 * Then function will try to write stored data to file descriptor in
 * non-locking mode. Note some data can be left within buffer. In this case
 * the "stall" callback will be executed to inform about it. To try to write
 * the rest of the data user can be call faux_buf_out() function. Both
 * functions will not lock.
 *
 * @param [in] buf Allocated and initialized buf I/O object.
 * @param [in] data Data buffer to write.
 * @param [in] len Data length to write.
 * @return Length of stored/writed data or < 0 on error.
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


/** @brief Write output buffer to fd in non-locking mode.
 *
 * Previously data must be written to internal buffer by faux_buf_write()
 * function. But some data can be left within internal buffer because can't be
 * written to fd in non-locking mode. This function tries to write the rest of
 * data to fd in non-locking mode. So function doesn't lock. It can be called
 * after select() or poll() if fd is ready to be written to. If function can't
 * to write all buffer to fd it executes "stall" callback to inform about it.
 *
 * @param [in] buf Allocated and initialized buf I/O object.
 * @return Length of data actually written or < 0 on error.
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
		size_t l = buf->len - avail; // length wo first chunk
		vec_entries_num += l / buf->chunk_size;
		if ((l % buf->chunk_size) > 0)
			vec_entries_num++;
	}
	iov = faux_zmalloc(vec_entries_num * sizeof(*iov));

	// Iterate chunks
	must_be_read = len_to_lock;
	iter = faux_list_head(buf->list);
	while ((must_be_read > 0) && (iter)) {
		char *p = (char *)faux_list_data(iter);
		size_t l = buf->chunk_size;
		size_t p_len = 0;

		if (iter == faux_list_head(buf->list)) { // First chunk
			p += buf->rpos;
			l = avail;
		}
		p_len = (must_be_read < l) ? must_be_read : l;

		iov[i].iov_base = p;
		iov[i].iov_len = p_len;
		i++;
		must_be_read -= p_len;
		iter = faux_list_next_node(iter);
	}

	*iov_out = iov;
	*iov_num_out = vec_entries_num;
	buf->rlocked = len_to_lock;

	return len_to_lock;
}


ssize_t faux_buf_dread_unlock(faux_buf_t *buf, size_t really_readed,
	struct iovec *iov)
{
	size_t must_be_read = 0;

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

	must_be_read = really_readed;
	while (must_be_read > 0) {
		size_t avail = faux_buf_ravail(buf);
		ssize_t data_to_rm = (must_be_read < avail) ? must_be_read : avail;

		buf->len -= data_to_rm;
		buf->rpos += data_to_rm;
		must_be_read -= data_to_rm;

		// Current chunk was fully readed. So remove it from list.
		if ((buf->rpos == buf->chunk_size) ||
			((faux_buf_chunk_num(buf) == 1) && (buf->rpos == buf->wpos))
			) {
			buf->rpos = 0; // 0 position within next chunk
			faux_list_del(buf->list, faux_list_head(buf->list));
		}
		if (faux_buf_chunk_num(buf) == 0)
			buf->wpos = buf->chunk_size;
	}

unlock:
	// Unlock whole buffer. Not 'really readed' bytes only
	buf->rlocked = 0;
	faux_free(iov);

	return really_readed;
}


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
	if (!iter)
		iter = faux_list_head(buf->list);
	i = 0;
	while ((must_be_write > 0) && (iter) && (i < vec_entries_num)) {
//	while ((must_be_write > 0) && (iter)) {
		char *p = (char *)faux_list_data(iter);
		size_t l = buf->chunk_size;
		size_t p_len = 0;

		if (iter == buf->wchunk) {
			p += buf->wpos;
			l = faux_buf_wavail(buf);
		}
		p_len = (must_be_write < l) ? must_be_write : l;
printf("num=%lu i=%u must=%lu plen=%lu\n", vec_entries_num, i, must_be_write, p_len);
		iter = faux_list_next_node(iter);
		// If wpos == chunk_size then p_len = 0
		// So go to next iteration without iov filling
		if (0 == p_len)
			continue;
		iov[i].iov_base = p;
		iov[i].iov_len = p_len;
		i++;
		must_be_write -= p_len;
	}

	*iov_out = iov;
	*iov_num_out = vec_entries_num;

	return len;
}


ssize_t faux_buf_dwrite_unlock(faux_buf_t *buf, size_t really_written,
	struct iovec *iov)
{
	size_t must_be_write = 0;
	faux_list_node_t *iter = NULL;

	assert(buf);
	if (!buf)
		return -1;
	// Can't unlock non-locked buffer
	if (!faux_buf_is_wlocked(buf))
		return -1;

	if (buf->wlocked < really_written)
		return -1; // Something went wrong


	must_be_write = really_written;
	while (must_be_write > 0) {
		size_t avail = 0;
		ssize_t data_to_add = 0;

printf("must=%lu\n", must_be_write);
		// Current chunk was fully written. So move to next one
		if (buf->wpos == buf->chunk_size) {
			buf->wpos = 0; // 0 position within next chunk
			if (buf->wchunk)
				buf->wchunk = faux_list_next_node(buf->wchunk);
			else
				buf->wchunk = faux_list_head(buf->list);
		}
		avail = faux_buf_wavail(buf);
		data_to_add = (must_be_write < avail) ? must_be_write : avail;

		buf->len += data_to_add;
		buf->wpos += data_to_add;
		must_be_write -= data_to_add;
	}

	if (buf->wchunk) {
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
