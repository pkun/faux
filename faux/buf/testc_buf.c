#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "faux/str.h"
#include "faux/buf.h"
#include "faux/testc_helpers.h"

#include "private.h"

#define CHUNK 100

int testc_faux_buf(void)
{
	char *src_fn = NULL;
	char *dst_fn = NULL;
	ssize_t len = 0;
	char *rnd = NULL;
	char *dst = NULL;
	faux_buf_t *buf = NULL;
	ssize_t res = 0;
	ssize_t chunk_num = 0;
	ssize_t e_chunk_num = 0;

	// Prepare files
	len = CHUNK * 3 + 15;
	e_chunk_num = 4;
	rnd = faux_testc_rnd_buf(len);
	src_fn = faux_testc_tmpfile_deploy(rnd, len);

	// Create buf
	buf = faux_buf_new(CHUNK);
	if (!buf) {
		fprintf(stderr, "faux_buf_new() error\n");
		return -1;
	}

	// Write to buffer
	if ((res = faux_buf_write(buf, rnd, len - 5)) != (len - 5)) {
		fprintf(stderr, "faux_buf_write() error %ld\n", res);
		return -1;
	}
	if (faux_buf_write(buf, rnd + len - 5, 5) != 5) {
		fprintf(stderr, "faux_buf_write() the rest error\n");
		return -1;
	}

	// Buf length
	if (faux_buf_len(buf) != len) {
		fprintf(stderr, "faux_buf_len() error\n");
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != e_chunk_num) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%ld\n",
			chunk_num, e_chunk_num);
		return -1;
	}

	// Buf read
	dst = faux_malloc(len);
	if (!dst) {
		fprintf(stderr, "faux_malloc() error\n");
		return -1;
	}
	if (faux_buf_read(buf, dst, len) != len) {
		fprintf(stderr, "faux_buf_read() error\n");
		return -1;
	}
	dst_fn = faux_testc_tmpfile_deploy(dst, len);

	// Buf length == 0
	if (faux_buf_len(buf) != 0) {
		fprintf(stderr, "faux_buf_len() is not 0: error\n");
		return -1;
	}

	// Compare files
	if (faux_testc_file_cmp(dst_fn, src_fn) != 0) {
		fprintf(stderr, "Destination file %s is not equal to source %s\n",
			dst_fn, src_fn);
		return -1;
	}

	faux_free(dst);
	faux_buf_free(buf);

	return 0;
}


int testc_faux_buf_boundaries(void)
{
	char *src_fn = NULL;
	char *dst_fn = NULL;
	ssize_t len = 0;
	char *rnd = NULL;
	char *dst = NULL;
	faux_buf_t *buf = NULL;
	ssize_t chunk_num = 0;
	ssize_t e_chunk_num = 0;
	ssize_t res = 0;

	// Prepare files
	len = CHUNK * 3;
	e_chunk_num = 3;
	rnd = faux_testc_rnd_buf(len);
	src_fn = faux_testc_tmpfile_deploy(rnd, len);

	// Create buf
	printf("faux_buf_new()\n");
	buf = faux_buf_new(CHUNK);
	if (!buf) {
		fprintf(stderr, "faux_buf_new() error\n");
		return -1;
	}

	// Write to buffer
	printf("faux_buf_write() len - CHUNK\n");
	if ((res = faux_buf_write(buf, rnd, len - CHUNK)) != (len - CHUNK)) {
		fprintf(stderr, "faux_buf_write() error %ld\n", res);
		return -1;
	}
	// Buf length
	printf("faux_buf_len()\n");
	if (faux_buf_len(buf) != (len - CHUNK)) {
		fprintf(stderr, "faux_buf_len() error\n");
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != (e_chunk_num - 1)) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%ld\n",
			chunk_num, e_chunk_num - 1);
		return -1;
	}

	printf("faux_buf_write() CHUNK\n");
	if (faux_buf_write(buf, rnd + len - CHUNK, CHUNK) != CHUNK) {
		fprintf(stderr, "faux_buf_write() the rest error\n");
		return -1;
	}

	// Buf length
	printf("faux_buf_len()\n");
	if (faux_buf_len(buf) != len) {
		fprintf(stderr, "faux_buf_len() error\n");
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != e_chunk_num) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%ld\n",
			chunk_num, e_chunk_num);
		return -1;
	}

	// Buf read
	dst = faux_malloc(len);
	if (!dst) {
		fprintf(stderr, "faux_malloc() error\n");
		return -1;
	}
	printf("faux_buf_read()\n");
	if (faux_buf_read(buf, dst, len) != len) {
		fprintf(stderr, "faux_buf_read() error\n");
		return -1;
	}
	dst_fn = faux_testc_tmpfile_deploy(dst, len);

	// Buf length == 0
	printf("faux_buf_len() 2\n");
	if (faux_buf_len(buf) != 0) {
		fprintf(stderr, "faux_buf_len() is not 0: error\n");
		return -1;
	}

	// Compare files
	if (faux_testc_file_cmp(dst_fn, src_fn) != 0) {
		fprintf(stderr, "Destination file %s is not equal to source %s\n",
			dst_fn, src_fn);
		return -1;
	}

	// Write to buffer anoter time
	printf("faux_buf_write() 2\n");
	if (faux_buf_write(buf, rnd, len) != len) {
		fprintf(stderr, "another faux_buf_write() error\n");
		return -1;
	}
	printf("faux_buf_read() 2\n");
	if (faux_buf_read(buf, dst, len) != len) {
		fprintf(stderr, "another faux_buf_read() error\n");
		return -1;
	}
	dst_fn = faux_testc_tmpfile_deploy(dst, len);

	// Compare files another time
	if (faux_testc_file_cmp(dst_fn, src_fn) != 0) {
		fprintf(stderr, "Destination file %s is not equal to source %s\n",
			dst_fn, src_fn);
		return -1;
	}

	faux_free(dst);
	faux_buf_free(buf);

	return 0;
}


int testc_faux_buf_direct(void)
{
//	char *src_fn = NULL;
//	char *dst_fn = NULL;
	ssize_t len = 0;
	char *rnd = NULL;
	char *dst = NULL;
	faux_buf_t *buf = NULL;
	ssize_t chunk_num = 0;
	ssize_t e_chunk_num = 0;
	ssize_t res = 0;
	ssize_t wlocked = 0;
	struct iovec *iov = NULL;
	size_t iov_num = 0;
	struct iovec *riov = NULL;
	size_t riov_num = 0;
	ssize_t bytes_readed = 0;

	// Prepare files
	len = CHUNK * 3;
	e_chunk_num = 3;
	rnd = faux_testc_rnd_buf(len);
//	src_fn = faux_testc_tmpfile_deploy(rnd, len);

	// Create buf
	printf("faux_buf_new()\n");
	buf = faux_buf_new(CHUNK);
	if (!buf) {
		fprintf(stderr, "faux_buf_new() error\n");
		return -1;
	}

	// Write to buffer
	printf("faux_buf_write() len - CHUNK\n");
	if ((res = faux_buf_write(buf, rnd, len - CHUNK)) != (len - CHUNK)) {
		fprintf(stderr, "faux_buf_write() error %ld\n", res);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 2) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 2);
		return -1;
	}

	// Lock buffer for writing
	printf("faux_buf_dwrite_lock()\n");
	if ((wlocked = faux_buf_dwrite_lock(buf, len + 15, &iov, &iov_num)) != (len + 15)) {
		fprintf(stderr, "faux_buf_dwrite_lock() error %ld\n",
			wlocked);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 6) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 6);
		return -1;
	}

	// Buf read
	dst = faux_malloc(len);
	if (!dst) {
		fprintf(stderr, "faux_malloc() error\n");
		return -1;
	}

	printf("faux_buf_dread_lock()\n");
	if ((bytes_readed = faux_buf_dread_lock(buf, len, &riov, &riov_num)) != (len - CHUNK)) {
		fprintf(stderr, "faux_buf_dread_lock() error. need %ld, readed %ld\n",
			len, bytes_readed);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 6) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 6);
		return -1;
	}

	printf("faux_buf_dread_ulock() must be -1\n");
	if ((bytes_readed = faux_buf_dread_unlock(buf, len, riov)) >= 0) {
		fprintf(stderr, "faux_buf_dread_lock() error. need -1, readed %ld\n",
			bytes_readed);
		return -1;
	}

	printf("faux_buf_dread_ulock()\n");
	if ((bytes_readed = faux_buf_dread_unlock(buf, len - CHUNK, riov)) != (len - CHUNK)) {
		fprintf(stderr, "faux_buf_dread_lock() error. need %ld, readed %ld\n",
			len, bytes_readed);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != (e_chunk_num + 1)) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%ld\n",
			chunk_num, e_chunk_num + 1);
		return -1;
	}

	// Unlock buffer for writing
	printf("faux_buf_dwrite_unlock()\n");
	if ((wlocked = faux_buf_dwrite_unlock(buf, len, iov)) != (len)) {
		fprintf(stderr, "faux_buf_dwrite_unlock() error %ld\n",
			wlocked);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 3) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 3);
		return -1;
	}

	printf("faux_buf_read()\n");
	if ((bytes_readed = faux_buf_read(buf, dst, len)) != len) {
		fprintf(stderr, "faux_buf_read() error. need %ld, readed %ld\n",
			len, bytes_readed);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 0) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 0);
		return -1;
	}

	printf("faux_buf_write() CHUNK\n");
	if (faux_buf_write(buf, rnd, CHUNK + 15) != (CHUNK + 15)) {
		fprintf(stderr, "faux_buf_write() the rest error\n");
		return -1;
	}

	// Buf length
	printf("faux_buf_len()\n");
	if (faux_buf_len(buf) != (CHUNK + 15)) {
		fprintf(stderr, "faux_buf_len() error\n");
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 2) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 2);
		return -1;
	}

	// Buf read
	printf("faux_buf_read()\n");
	if (faux_buf_read(buf, dst, len) != (CHUNK + 15)) {
		fprintf(stderr, "faux_buf_read() error\n");
		return -1;
	}

	// Buf length == 0
	printf("faux_buf_len() 2\n");
	if (faux_buf_len(buf) != 0) {
		fprintf(stderr, "faux_buf_len() is not 0: error\n");
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 0) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 0);
		return -1;
	}

	faux_free(dst);
	faux_buf_free(buf);

	return 0;
}


int testc_faux_buf_dwrite_unlock0(void)
{
//	char *src_fn = NULL;
//	char *dst_fn = NULL;
	ssize_t len = 0;
	char *rnd = NULL;
	char *dst = NULL;
	faux_buf_t *buf = NULL;
	ssize_t chunk_num = 0;
	ssize_t res = 0;
	ssize_t wlocked = 0;
	struct iovec *iov = NULL;
	size_t iov_num = 0;
	struct iovec *riov = NULL;
	size_t riov_num = 0;
	ssize_t bytes_readed = 0;

	// Prepare files
	len = CHUNK * 3;
	rnd = faux_testc_rnd_buf(len);
//	src_fn = faux_testc_tmpfile_deploy(rnd, len);

	// Create buf
	printf("faux_buf_new()\n");
	buf = faux_buf_new(CHUNK);
	if (!buf) {
		fprintf(stderr, "faux_buf_new() error\n");
		return -1;
	}

	// Write to buffer
	printf("faux_buf_write() len - CHUNK\n");
	if ((res = faux_buf_write(buf, rnd, CHUNK + 15)) != (CHUNK + 15)) {
		fprintf(stderr, "faux_buf_write() error %ld\n", res);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 2) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 2);
		return -1;
	}

	// Lock buffer for writing
	printf("faux_buf_dwrite_lock()\n");
	if ((wlocked = faux_buf_dwrite_lock(buf, len, &iov, &iov_num)) != len) {
		fprintf(stderr, "faux_buf_dwrite_lock() error %ld\n",
			wlocked);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 5) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 5);
		return -1;
	}

	// Buf read
	dst = faux_malloc(len);
	if (!dst) {
		fprintf(stderr, "faux_malloc() error\n");
		return -1;
	}

	printf("faux_buf_dread_lock()\n");
	if ((bytes_readed = faux_buf_dread_lock(buf, len, &riov, &riov_num)) != (CHUNK + 15)) {
		fprintf(stderr, "faux_buf_dread_lock() error. need %d, readed %ld\n",
			CHUNK + 15, bytes_readed);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 5) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 5);
		return -1;
	}

	printf("faux_buf_dread_ulock() must be -1\n");
	if ((bytes_readed = faux_buf_dread_unlock(buf, len, riov)) >= 0) {
		fprintf(stderr, "faux_buf_dread_lock() error. need -1, readed %ld\n",
			bytes_readed);
		return -1;
	}

	printf("faux_buf_dread_ulock()\n");
	if ((bytes_readed = faux_buf_dread_unlock(buf, CHUNK + 15, riov)) != (CHUNK + 15)) {
		fprintf(stderr, "faux_buf_dread_lock() error. need %ld, readed %ld\n",
			len, bytes_readed);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 4) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 4);
		return -1;
	}

	// Unlock buffer for writing
	printf("faux_buf_dwrite_unlock() with 0 len\n");
	if ((wlocked = faux_buf_dwrite_unlock(buf, 0, iov)) != 0) {
		fprintf(stderr, "faux_buf_dwrite_unlock() error %ld\n",
			wlocked);
		return -1;
	}

	// Buf chunk num
	printf("faux_buf_chunk_num()\n");
	if ((chunk_num = faux_buf_chunk_num(buf)) != 0) {
		fprintf(stderr, "faux_buf_chunk_num() error. num=%ld e=%d\n",
			chunk_num, 0);
		return -1;
	}

	faux_free(dst);
	faux_buf_free(buf);

	return 0;
}


int testc_faux_buf_mass(void)
{
	ssize_t len = 0;
	faux_buf_t *buf = NULL;
	ssize_t res = 0;
	unsigned char valw = 0;
	unsigned char valr = 0;
	size_t tlw = 0;
	size_t tlr = 0;
	size_t len_r = 234;

	// Create buf
	printf("faux_buf_new()\n");
	buf = faux_buf_new(CHUNK);
	if (!buf) {
		fprintf(stderr, "faux_buf_new() error\n");
		return -1;
	}

	// Write to buffer
	printf("faux_buf_write()\n");
	for (len = 3000; len < 8900; len += 3) {
		unsigned char *t = faux_malloc(len);
		int i = 0;
		for (i = 0; i < len; i++)
			t[i] = valw++;
		if ((res = faux_buf_write(buf, t, len)) != len) {
			fprintf(stderr, "faux_buf_write() error %ld\n", res);
			return -1;
		}
		tlw += res;
		faux_free(t);
	}
	// Buf length
	printf("faux_buf_len()\n");
	if (faux_buf_len(buf) != tlw) {
		fprintf(stderr, "faux_buf_len() error\n");
		return -1;
	}

	// Read to buffer
	printf("faux_buf_read()\n");
	while (faux_buf_len(buf) != 0) {
		unsigned char *t = faux_malloc(len_r);
		int i = 0;
		if ((res = faux_buf_read(buf, t, len_r)) < 0) {
			fprintf(stderr, "faux_buf_read() error %ld\n", res);
			return -1;
		}
		for (i = 0; i < res; i++) {
			if (t[i] != valr++) {
				fprintf(stderr, "Incorrect valr %d != %ld\n",
					t[i], tlr + i);
				return -1;
			}
		}
		tlr += res;
		faux_free(t);
		len_r += 7;
	}

	// Buf length
	printf("faux_buf_len()\n");
	if (tlr != tlw) {
		fprintf(stderr, "tlr != tlw\n");
		return -1;
	}

	// val
	printf("valr and valw\n");
	if (valr != valw) {
		fprintf(stderr, "valr != valw\n");
		return -1;
	}

	faux_buf_free(buf);

	return 0;
}
