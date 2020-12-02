/** @file crsp_msg.c
 * @brief Class represents a single message of CRSP protocol.
 *
 * CRSP message consist of main header, a block of parameter headers and then
 * parameters themselfs. Class stores these data. Additionally class knows
 * the structure of message and can send and receive messages via socket. It
 * uses external faux_net_t object to do so. The receive function is necessary
 * because message has a variable length and message parsing is needed to get
 * actual length of message. The send function is usefull because class uses
 * struct iovec array to compose outgoing message so it's not necessary to
 * assemble message into the single long memory chunk.
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

// Global variable to switch debug on/off (1/0)
int crsp_debug = 0;


/** @brief Opaque faux_msg_s structure. */
struct faux_msg_s {
	faux_hdr_t *hdr; // Message header
	faux_list_t *params; // List of parameters
};


/** @brief Allocate memory to store message.
 *
 * This static function is needed because new message object can be created
 * in a different ways. The first way is creating outgoing message manually and
 * the second way is receiving CRSP message from network. These ways need
 * different initialization but the same memory allocation.
 *
 * @return Allocated but not fully initialized faux_msg_t object
 * or NULL on error
 */
static faux_msg_t *faux_msg_allocate(void)
{
	faux_msg_t *msg = NULL;

	msg = faux_zmalloc(sizeof(*msg));
	assert(msg);
	if (!msg)
		return NULL;

	// Init message header
	msg->hdr = faux_zmalloc(sizeof(*msg->hdr));
	assert(msg->hdr);
	if (!msg->hdr) {
		faux_msg_free(msg);
		return NULL;
	}

	msg->params = faux_list_new(
		FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE, NULL, NULL, faux_free);

	return msg;
}


/** @brief Creates new faux_msg_t object. It's usually outgoing message.
 *
 * Function initializes main message header with default values. Usually
 * only outgoing messages need initialized header.
 *
 * @param [in] magic Protocol's magic number.
 * @param [in] major Protocol's version major number.
 * @param [in] minor Protocol's version minor number.
 
 * @return Allocated and initilized faux_msg_t object or NULL on error.
 */
faux_msg_t *faux_msg_new(uint32_t magic, uint8_t major, uint8_t minor)
{
	faux_msg_t *msg = NULL;

	msg = faux_msg_allocate();
	assert(msg);
	if (!msg)
		return NULL;

	// Init
	msg->hdr->magic = htonl(magic);
	msg->hdr->major = major;
	msg->hdr->minor = minor;
	faux_msg_set_cmd(msg, 0);
	faux_msg_set_status(msg, 0);
	faux_msg_set_req_id(msg, 0l);
	faux_msg_set_param_num(msg, 0l);
	faux_msg_set_len(msg, sizeof(*msg->hdr));

	return msg;
}


/** @brief Frees allocated message.
 *
 * @param [in] msg Allocated faux_msg_t object.
 */
void faux_msg_free(faux_msg_t *msg)
{
	if (!msg)
		return;

	faux_list_free(msg->params);
	faux_free(msg->hdr);
	faux_free(msg);
}


/** @brief Sets command code to header.
 *
 * See the protocol and header description for possible values.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] cmd Command code (16 bit).
 */
void faux_msg_set_cmd(faux_msg_t *msg, uint16_t cmd)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return;
	msg->hdr->cmd = htons(cmd);
}


/** @brief Gets command code from header.
 *
 * See the protocol and header description for possible values.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [out] cmd Command code.
 * @return 0 - success, < 0 - fail
 */
int faux_msg_get_cmd(const faux_msg_t *msg, uint16_t *cmd)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;
	if (cmd)
		*cmd = ntohs(msg->hdr->cmd);

	return 0;
}


/** @brief Sets message status to header.
 *
 * See the protocol and header description for possible values.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] status Message status.
 */
void faux_msg_set_status(faux_msg_t *msg, uint32_t status)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return;
	msg->hdr->status = htonl(status);
}


/** @brief Gets message status from header.
 *
 * See the protocol and header description for possible values.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [out] status Message status.
 * @return 0 - success, < 0 -fail
 */
int faux_msg_get_status(const faux_msg_t *msg, uint32_t *status)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;
	if (status)
		*status = ntohl(msg->hdr->status);

	return 0;
}


/** @brief Sets request ID to header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] req_id Request ID.
 */
void faux_msg_set_req_id(faux_msg_t *msg, uint32_t req_id)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return;
	msg->hdr->req_id = htonl(req_id);
}


/** @brief Gets request ID from header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [out] req_id Request ID.
 * @return 0 - success, < 0 - fail
 */
int faux_msg_get_req_id(const faux_msg_t *msg, uint32_t *req_id)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;
	if (req_id)
		*req_id = ntohl(msg->hdr->req_id);

	return 0;
}


/** @brief Sets number of parameters to header.
 *
 * It's a static function because external user can add or remove parameters
 * but class calculates total number of parameters internally.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] param_num Number of parameters.
 */
static void faux_msg_set_param_num(faux_msg_t *msg, uint32_t param_num)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return;
	msg->hdr->param_num = htonl(param_num);
}


/** @brief Gets number of parameters from header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @return Number of parameters.
 */
int faux_msg_get_param_num(const faux_msg_t *msg, uint32_t *param_num)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;
	if (param_num)
		*param_num = ntohl(msg->hdr->param_num);

	return 0;
}


/** @brief Sets total length of message to header.
 *
 * It's a static function because external user can add or remove parameters
 * but class calculates total length of message internally.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] len Total length of message.
 */
static void faux_msg_set_len(faux_msg_t *msg, uint32_t len)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return;
	msg->hdr->len = htonl(len);
}


/** @brief Gets total length of message from header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [out] len Total length of message.
 * @return 0 - success, < 0 - fail
 */
int faux_msg_get_len(const faux_msg_t *msg, uint32_t *len)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;
	if (len)
		*len = ntohl(msg->hdr->len);

	return 0;
}


/** @brief Gets magic number from header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [out] magic Magic number.
 * @return 0 - success, < 0 - fail
 */
int faux_msg_get_magic(const faux_msg_t *msg, uint32_t *magic)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;
	if (magic)
		*magic = ntohl(msg->hdr->magic);

	return 0;
}


/** @brief Gets version from header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [out] major Major version number.
 * @param [out] minor Minor version number.
 * @return 0 - success, < 0 - fail
 */
int faux_msg_get_version(const faux_msg_t *msg, uint8_t *major, uint8_t *minor)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;
	if (major)
		*major = msg->hdr->major;
	if (minor)
		*minor = msg->hdr->minor;

	return 0;
}


/** @brief Internal function to add message parameter
 *
 * Internal function can update or don't update number of parameters and
 * whole length within message header. It can be used while
 * message receive to don't break already calculated header
 * values. So when user is constructing message the values must be updated.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] type Type of parameter.
 * @param [in] buf Parameter's data buffer.
 * @param [in] len Parameter's data length.
 * @param [in] upadte_len Flag that says to update or don't update number of
 * parameters and total message length within header. BOOL_TRUE - update,
 * BOOL_FALSE - don't update.
 * @return Length of parameter's data or < 0 on error.
 */
static ssize_t faux_msg_add_param_internal(faux_msg_t *msg,
	uint16_t type, const void *buf, size_t len, bool_t update_len)
{
	faux_phdr_t *phdr = NULL;
	char *param = NULL;

	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;

	// Allocate parameter header and data
	param = faux_zmalloc(sizeof(*phdr) + len);
	assert(param);
	if (!param)
		return -1;
	// Init param hdr
	phdr = (faux_phdr_t *)param;
	phdr->param_type = htonl(type);
	phdr->param_len = htonl(len);
	// Copy data
	memcpy(param + sizeof(*phdr), buf, len);

	if (update_len) {
		uint32_t cur_param_num = 0;
		uint32_t cur_len = 0;

		// Update number of parameters
		faux_msg_get_param_num(msg, &cur_param_num);
		faux_msg_set_param_num(msg, cur_param_num + 1);

		// Update whole message length
		faux_msg_get_len(msg, &cur_len);
		crsp_msg_set_len(crsp_msg, cur_len + sizeof(*phdr) + len);
	}

	// Add to parameter list
	faux_list_add(msg->params, param);

	return len;
}


/** @brief Adds parameter to message.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] type Type of parameter.
 * @param [in] buf Parameter's data buffer.
 * @param [in] len Parameter's data length.
 * @return Length of parameter's data or < 0 on error.
 */
ssize_t faux_msg_add_param(faux_msg_t *msg, uint16_t type,
	const void *buf, size_t len)
{
	return faux_msg_add_param_internal(msg, type, buf, len, BOOL_TRUE);
}


/** @brief Initializes iterator to iterate through the message parameters.
 *
 * The iterator must be initialized before iteration.
 *
 * @param [in] crsp_msg Allocated crsp_msg_t object.
 * @return Initialized iterator.
 */
faux_list_node_t *crsp_msg_init_param_iter(const crsp_msg_t *crsp_msg)
{
	assert(crsp_msg);
	assert(crsp_msg->params);
	if (!crsp_msg || !crsp_msg->params)
		return NULL;

	return faux_list_head(crsp_msg->params);
}


/** @brief Internal function to get parameter's data by node (faux_list_node_t).
 *
 * Note function returns the main data by output arguments.
 *
 * @param [in] node Node from the parameter's list.
 * @param [out] param_type Type of parameter. See the crsp_param_e enumeration.
 * @param [out] param_buf Parameter's data buffer.
 * @param [out] param_len Parameter's data length.
 * @return Pointer to parameter's header or NULL on error.
 */
static crsp_phdr_t *crsp_msg_get_param_by_node(const faux_list_node_t *node,
	crsp_param_e *param_type, void **param_data, uint32_t *param_len)
{
	char *param = NULL;
	crsp_phdr_t *phdr = NULL;
	char *data = NULL;

	if (!node)
		return NULL;

	param = faux_list_data(node);
	phdr = (crsp_phdr_t *)param;
	data = param + sizeof(*phdr);

	if (param_type)
		*param_type = phdr->param_type;
	if (param_data)
		*param_data = data;
	if (param_len)
		*param_len = ntohl(phdr->param_len);

	return phdr;
}


/** @brief Iterate through the message parameters.
 *
 * First parameter (iterator/node) must be initialized first by
 * crsp_msg_init_param_iter().
 *
 * @param [in] node Initialized iterator of parameter list.
 * @param [out] param_type Type of parameter. See the crsp_param_e enumeration.
 * @param [out] param_buf Parameter's data buffer.
 * @param [out] param_len Parameter's data length.
 * @return Pointer to parameter's header or NULL on error.
 */
crsp_phdr_t *crsp_msg_get_param_each(faux_list_node_t **node,
	crsp_param_e *param_type, void **param_data, uint32_t *param_len)
{
	faux_list_node_t *current_node = NULL;

	if (!node || !*node)
		return NULL;

	current_node = *node;
	*node = faux_list_next_node(current_node);

	return crsp_msg_get_param_by_node(current_node,
		param_type, param_data, param_len);
}


/** @brief Gets message parameter by the index.
 *
 * @param [in] crsp_msg Allocated crsp_msg_t object.
 * @param [in] index Parameter's index.
 * @param [out] param_type Type of parameter. See the crsp_param_e enumeration.
 * @param [out] param_buf Parameter's data buffer.
 * @param [out] param_len Parameter's data length.
 * @return Pointer to parameter's header or NULL on error.
 */
crsp_phdr_t *crsp_msg_get_param_by_index(const crsp_msg_t *crsp_msg, unsigned int index,
	crsp_param_e *param_type, void **param_data, uint32_t *param_len)
{
	faux_list_node_t *iter = NULL;
	unsigned int i = 0;

	assert(crsp_msg);
	assert(crsp_msg->hdr);
	if (!crsp_msg || !crsp_msg->hdr)
		return NULL;
	if (index >= crsp_msg_get_param_num(crsp_msg)) // Non-existent entry
		return NULL;

	iter = crsp_msg_init_param_iter(crsp_msg);
	while ((i != index) && iter) {
		i++;
		iter = faux_list_next_node(iter);
	}

	return crsp_msg_get_param_by_node(iter,
		param_type, param_data, param_len);
}


/** @brief Gets message parameter by parameter's type.
 *
 * @param [in] crsp_msg Allocated crsp_msg_t object.
 * @param [in] param_type Type of parameter. See the crsp_param_e enumeration.
 * @param [out] param_buf Parameter's data buffer.
 * @param [out] param_len Parameter's data length.
 * @return Pointer to parameter's header or NULL on error.
 */
crsp_phdr_t *crsp_msg_get_param_by_type(const crsp_msg_t *crsp_msg,
	crsp_param_e param_type, void **param_data, uint32_t *param_len)
{
	faux_list_node_t *iter = NULL;

	assert(crsp_msg);
	assert(crsp_msg->hdr);
	if (!crsp_msg || !crsp_msg->hdr)
		return NULL;

	for (iter = crsp_msg_init_param_iter(crsp_msg);
		iter; iter = faux_list_next_node(iter)) {
		crsp_phdr_t *phdr = NULL;
		phdr = (crsp_phdr_t *)faux_list_data(iter);
		if (phdr->param_type == param_type)
			return crsp_msg_get_param_by_node(iter,
				NULL, param_data, param_len);
	}

	// Not found
	return NULL;
}


/** @brief Sends CRSP message to network.
 *
 * Function sends message to network using preinitialized faux_net_t object.
 * User can specify timeout, signal mask, etc while faux_net_t object creation.
 *
 * Function can return length less than whole message length in the following
 * cases:
 * - An error has occured like broken file descriptor.
 * - Interrupted by allowed signal (see signal mask).
 * - Timeout.
 *
 * @param [in] crsp_msg Allocated crsp_msg_t object.
 * @param [in] faux_net Preinitialized faux_net_t object.
 * @return Length of sent data or < 0 on error.
 */
ssize_t crsp_msg_send(crsp_msg_t *crsp_msg, faux_net_t *faux_net)
{
	unsigned int vec_entries_num = 0;
	struct iovec *iov = NULL;
	unsigned int i = 0;
	faux_list_node_t *iter = NULL;
	size_t ret = 0;

	assert(crsp_msg);
	assert(crsp_msg->hdr);
	if (!crsp_msg || !crsp_msg->hdr)
		return -1;

	// Calculate number if struct iovec entries.
	// n = (msg header) + ((param hdr) + (param data)) * (param_num)
	vec_entries_num = 1 + (2 * crsp_msg_get_param_num(crsp_msg));
	iov = faux_zmalloc(vec_entries_num * sizeof(*iov));

	// Message header
	iov[i].iov_base = crsp_msg->hdr;
	iov[i].iov_len = sizeof(*crsp_msg->hdr);
	i++;

	// Parameter headers
	for (iter = crsp_msg_init_param_iter(crsp_msg);
		iter; iter = faux_list_next_node(iter)) {
		crsp_phdr_t *phdr = NULL;
		phdr = (crsp_phdr_t *)faux_list_data(iter);
		iov[i].iov_base = phdr;
		iov[i].iov_len = sizeof(*phdr);
		i++;
	}

	// Parameter data
	for (iter = crsp_msg_init_param_iter(crsp_msg);
		iter; iter = faux_list_next_node(iter)) {
		crsp_phdr_t *phdr = NULL;
		void *data = NULL;
		phdr = (crsp_phdr_t *)faux_list_data(iter);
		data = (char *)phdr + sizeof(*phdr);
		iov[i].iov_base = data;
		iov[i].iov_len = ntohl(phdr->param_len);
		i++;
	}

	ret = faux_net_sendv(faux_net, iov, vec_entries_num);
	faux_free(iov);

	// Debug
	if (crsp_msg && ret > 0 && crsp_debug) {
		printf("(o) ");
		crsp_msg_debug(crsp_msg);
	}

	return ret;
}


/** @brief Receives full CRSP message and allocates crsp_msg_t object for it.
 *
 * Function receives message from network using preinitialized faux_net_t object.
 * User can specify timeout, signal mask, etc while faux_net_t object creation.
 *
 * Function can return length less than whole message length in the following
 * cases:
 * - An error has occured like broken file descriptor.
 * - Interrupted by allowed signal (see signal mask).
 * - Timeout.
 *
 * It can be an logical errors while message receiving like wrong protocol
 * version. So function has additional parameter named 'status'. It will
 * be CRSP_STATUS_OK in a case when all is ok but function can return NULL and
 * set appropriate status to this parameter. It can be
 * CRSP_STATUS_WRONG_VERSION for example. The function will return NULL
 * and CRSP_STATUS_OK on some system errors like illegal parameters or
 * insufficient of memory.
 *
 * @param [in] faux_net Preinitialized faux_net_t object.
 * @param [out] status Status while message receiving. Can be NULL.
 * @return Allocated crsp_msg_t object. Object contains received message.
 */
crsp_msg_t *crsp_msg_recv(faux_net_t *faux_net, crsp_recv_e *status)
{
	crsp_msg_t *crsp_msg = NULL;
	size_t received = 0;
	crsp_phdr_t *phdr = NULL;
	unsigned int param_num = 0;
	size_t phdr_whole_len = 0;
	size_t max_data_len = 0;
	size_t cur_data_len = 0;
	unsigned int i = 0;
	char *data = NULL;

	if (status)
		*status = CRSP_RECV_OK;
	crsp_msg = crsp_msg_allocate();
	assert(crsp_msg);
	if (!crsp_msg)
		return NULL;

	// Receive message header
	received = faux_net_recv(faux_net,
		crsp_msg->hdr, sizeof(*crsp_msg->hdr));
	if (received != sizeof(*crsp_msg->hdr)) {
		crsp_msg_free(crsp_msg);
		return NULL;
	}
	if (!crsp_msg_check_hdr(crsp_msg, status)) {
		crsp_msg_free(crsp_msg);
		return NULL;
	}

	// Receive parameter headers
	param_num = crsp_msg_get_param_num(crsp_msg);
	if (param_num != 0) {
		phdr_whole_len = param_num * sizeof(*phdr);
		phdr = faux_zmalloc(phdr_whole_len);
		received = faux_net_recv(faux_net, phdr, phdr_whole_len);
		if (received != phdr_whole_len) {
			faux_free(phdr);
			crsp_msg_free(crsp_msg);
			if (status)
				*status = CRSP_RECV_BROKEN_PARAM;
			return NULL;
		}
		// Find out maximum data length
		for (i = 0; i < param_num; i++) {
			cur_data_len = ntohl(phdr[i].param_len);
			if (cur_data_len > max_data_len)
				max_data_len = cur_data_len;
		}

		// Receive parameter data
		data = faux_zmalloc(max_data_len);
		for (i = 0; i < param_num; i++) {
			cur_data_len = ntohl(phdr[i].param_len);
			if (0 == cur_data_len)
				continue;
			received = faux_net_recv(faux_net,
				data, cur_data_len);
			if (received != cur_data_len) {
				faux_free(data);
				faux_free(phdr);
				crsp_msg_free(crsp_msg);
				if (status)
					*status = CRSP_RECV_BROKEN_PARAM;
				return NULL;
			}
			crsp_msg_add_param_internal(crsp_msg, phdr[i].param_type,
				data, cur_data_len, BOOL_FALSE);
		}

		faux_free(data);
		faux_free(phdr);
	}

	// Debug
	if (crsp_msg && crsp_debug) {
		printf("(i) ");
		crsp_msg_debug(crsp_msg);
	}

	return crsp_msg;
}


/** @brief Gets CRSP_PARAM_CDP_URI parameter from message.
 *
 * Protocol helper.
 *
 * @param [in] crsp_msg Allocated crsp_msg_t object.
 * @return Allocated C-string or NULL on error. Must be freed by faux_str_free().
 */
char *crsp_msg_get_param_cdp_uri(const crsp_msg_t *crsp_msg)
{
	char *req_uri = NULL;
	uint32_t req_uri_len = 0;

	if (!crsp_msg_get_param_by_type(crsp_msg, CRSP_PARAM_CDP_URI,
		(void **)&req_uri, &req_uri_len))
		return NULL;

	if (0 == req_uri_len)
		return NULL;

	return faux_str_dupn(req_uri, req_uri_len);
}


/** @brief Print CRSP message debug info.
 *
 * Function prints header values and parameters.
 *
 * @param [in] crsp_msg Allocated crsp_msg_t object.
 */
void crsp_msg_debug(crsp_msg_t *crsp_msg)
#ifdef DEBUG
{
	faux_list_node_t *iter = 0;
	crsp_param_e param_type = CRSP_PARAM_NULL;
	void *param_data = NULL;
	uint32_t param_len = 0;

	assert(crsp_msg);
	if (!crsp_msg)
		return;

	// Header
	printf("%c%c%c%c(%u.%u): %c%c %u %u %u |%lub\n",
		((char *)crsp_msg->hdr)[0],
		((char *)crsp_msg->hdr)[1],
		((char *)crsp_msg->hdr)[2],
		((char *)crsp_msg->hdr)[3],
		crsp_msg->hdr->major,
		crsp_msg->hdr->minor,
		crsp_msg_get_cmd(crsp_msg) != CRSP_CMD_NULL ? crsp_msg_get_cmd(crsp_msg) : '_',
		crsp_msg_get_status(crsp_msg) != CRSP_STATUS_NULL ? crsp_msg_get_status(crsp_msg) : '_',
		crsp_msg_get_req_id(crsp_msg),
		crsp_msg_get_param_num(crsp_msg),
		crsp_msg_get_len(crsp_msg),
		sizeof(*crsp_msg->hdr)
		);

	// Parameters
	iter = crsp_msg_init_param_iter(crsp_msg);
	while (crsp_msg_get_param_each(&iter, &param_type, &param_data, &param_len)) {
		printf("  %c %u [", param_type, param_len);
		if ((CRSP_PARAM_CDP_URI == param_type) ||
			(CRSP_PARAM_CRL_FILENAME == param_type)) {
			fwrite(param_data, param_len, 1, stdout);
		} else {
			printf("...");
		}
		printf("] |%lub\n", sizeof(crsp_phdr_t) + param_len);
	}
}
#else
{
	crsp_msg = crsp_msg; // Happy compiler
}
#endif
