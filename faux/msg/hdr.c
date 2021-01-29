/** @file hdr.c
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <faux/faux.h>
#include <faux/str.h>
#include <faux/list.h>
#include <faux/net.h>
#include <faux/msg.h>


/** @brief Sets command code to header.
 *
 * See the protocol and header description for possible values.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @param [in] cmd Command code (16 bit).
 */
void faux_hdr_set_cmd(faux_hdr_t *hdr, uint16_t cmd)
{
	assert(hdr);
	if (!hdr)
		return;
	hdr->cmd = htons(cmd);
}


/** @brief Gets command code from header.
 *
 * See the protocol and header description for possible values.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @return Command code or 0 on error.
 */
uint16_t faux_hdr_cmd(const faux_hdr_t *hdr)
{
	assert(hdr);
	if (!hdr)
		return 0;

	return ntohs(hdr->cmd);
}


/** @brief Sets message status to header.
 *
 * See the protocol and header description for possible values.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @param [in] status Message status.
 */
void faux_hdr_set_status(faux_hdr_t *hdr, uint32_t status)
{
	assert(hdr);
	if (!hdr)
		return;
	hdr->status = htonl(status);
}


/** @brief Gets message status from header.
 *
 * See the protocol and header description for possible values.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @return Message status or 0 on error.
 */
uint32_t faux_hdr_status(const faux_hdr_t *hdr)
{
	assert(hdr);
	if (!hdr)
		return 0;

	return ntohl(hdr->status);
}


/** @brief Sets request ID to header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @param [in] req_id Request ID.
 */
void faux_hdr_set_req_id(faux_hdr_t *hdr, uint32_t req_id)
{
	assert(hdr);
	if (!hdr)
		return;
	hdr->req_id = htonl(req_id);
}


/** @brief Gets request ID from header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @return Request ID or 0 on error.
 */
uint32_t faux_hdr_req_id(const faux_hdr_t *hdr)
{
	assert(hdr);
	if (!hdr)
		return 0;

	return ntohl(hdr->req_id);
}


/** @brief Sets number of parameters to header.
 *
 * It's a static function because external user can add or remove parameters
 * but class calculates total number of parameters internally.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @param [in] param_num Number of parameters.
 */
void faux_hdr_set_param_num(faux_hdr_t *hdr, uint32_t param_num)
{
	assert(hdr);
	if (!hdr)
		return;
	hdr->param_num = htonl(param_num);
}


/** @brief Gets number of parameters from header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @return Number of parameters or 0 on error.
 */
uint32_t faux_hdr_param_num(const faux_hdr_t *hdr)
{
	assert(hdr);
	if (!hdr)
		return -1;

	return ntohl(hdr->param_num);
}


/** @brief Sets total length of message to header.
 *
 * It's a static function because external user can add or remove parameters
 * but class calculates total length of message internally.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @param [in] len Total length of message.
 */
void faux_hdr_set_len(faux_hdr_t *hdr, uint32_t len)
{
	assert(hdr);
	if (!hdr)
		return;
	hdr->len = htonl(len);
}


/** @brief Gets total length of message from header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @return Total length of message or 0 on error.
 */
int faux_hdr_len(const faux_hdr_t *hdr)
{
	assert(hdr);
	if (!hdr)
		return 0;

	return ntohl(hdr->len);
}


/** @brief Sets magic number to header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @param [in] magic Magic number.
 */
void faux_hdr_set_magic(faux_hdr_t *hdr, uint32_t magic)
{
	assert(hdr);
	if (!hdr)
		return;

	hdr->magic = htonl(magic);
}


/** @brief Gets magic number from header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @return Magic number or 0 on error.
 */
uint32_t faux_hdr_magic(const faux_hdr_t *hdr)
{
	assert(hdr);
	if (!hdr)
		return 0;

	return ntohl(hdr->magic);
}


/** @brief Sets major version to header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @param [in] major Major protocol version.
 * @return Major version number or 0 on error.
 */
void faux_hdr_set_major(faux_hdr_t *hdr, uint8_t major)
{
	assert(hdr);
	if (!hdr)
		return;

	hdr->major = major;
}


/** @brief Gets major version from header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @return Major version number or 0 on error.
 */
uint8_t faux_hdr_major(const faux_hdr_t *hdr)
{
	assert(hdr);
	if (!hdr)
		return 0;

	return hdr->major;
}


/** @brief Sets minor version to header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @param [in] minor Minor protocol version.
 * @return Major version number or 0 on error.
 */
void faux_hdr_set_minor(faux_hdr_t *hdr, uint8_t minor)
{
	assert(hdr);
	if (!hdr)
		return;

	hdr->minor = minor;
}


/** @brief Gets minor version from header.
 *
 * @param [in] hdr Allocated faux_hdr_t object.
 * @return Minor version number or 0 on error.
 */
uint8_t faux_hdr_minor(const faux_hdr_t *hdr)
{
	assert(hdr);
	if (!hdr)
		return 0;

	return hdr->minor;
}
