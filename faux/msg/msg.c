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

// Global variable to switch debug on/off (true/false)
bool_t faux_msg_debug_flag = BOOL_FALSE;


/** @brief Opaque faux_msg_s structure. */
struct faux_msg_s {
	faux_hdr_t *hdr; // Message header
	faux_list_t *params; // List of parameters
};


static void faux_msg_set_len(faux_msg_t *msg, uint32_t len);
static void faux_msg_set_param_num(faux_msg_t *msg, uint32_t param_num);


/** @brief Allocate memory to store message.
 *
 * This static function is needed because new message object can be created
 * in a different ways. The first way is creating outgoing message manually and
 * the second way is receiving message from network. These ways need
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
 * @return Command code or 0 on error.
 */
uint16_t faux_msg_get_cmd(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return 0;

	return ntohs(msg->hdr->cmd);
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
 * @return Message status or 0 on error.
 */
uint32_t faux_msg_get_status(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return 0;

	return ntohl(msg->hdr->status);
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
 * @return Request ID or 0 on error.
 */
uint32_t faux_msg_get_req_id(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return 0;

	return ntohl(msg->hdr->req_id);
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
 * @return Number of parameters or 0 on error.
 */
uint32_t faux_msg_get_param_num(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;

	return ntohl(msg->hdr->param_num);
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
 * @return Total length of message or 0 on error.
 */
int faux_msg_get_len(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return 0;

	return ntohl(msg->hdr->len);
}


/** @brief Gets magic number from header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @return Magic number or 0 on error.
 */
uint32_t faux_msg_get_magic(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return 0;

	return ntohl(msg->hdr->magic);
}


/** @brief Gets major version from header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @return Major version number or 0 on error.
 */
int faux_msg_get_major(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return 0;

	return msg->hdr->major;
}


/** @brief Gets minor version from header.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @return Minor version number or 0 on error.
 */
int faux_msg_get_minor(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return 0;

	return msg->hdr->minor;
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
	faux_phdr_set_type(phdr, type);
	faux_phdr_set_len(phdr, len);
	// Copy data
	memcpy(param + sizeof(*phdr), buf, len);

	if (update_len) {
		// Update number of parameters
		faux_msg_set_param_num(msg, faux_msg_get_param_num(msg) + 1);
		// Update whole message length
		faux_msg_set_len(msg,
			faux_msg_get_len(msg) + sizeof(*phdr) + len);
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
 * @param [in] msg Allocated faux_msg_t object.
 * @return Initialized iterator.
 */
faux_list_node_t *faux_msg_init_param_iter(const faux_msg_t *msg)
{
	assert(msg);
	assert(msg->params);
	if (!msg || !msg->params)
		return NULL;

	return faux_list_head(msg->params);
}


/** @brief Internal function to get parameter's data by node (faux_list_node_t).
 *
 * Note function returns the main data by output arguments.
 *
 * @param [in] node Node from the parameter's list.
 * @param [out] param_type Type of parameter.
 * @param [out] param_buf Parameter's data buffer.
 * @param [out] param_len Parameter's data length.
 * @return Pointer to parameter's header or NULL on error.
 */
static faux_phdr_t *faux_msg_get_param_by_node(const faux_list_node_t *node,
	uint16_t *param_type, void **param_data, uint32_t *param_len)
{
	char *param = NULL;
	faux_phdr_t *phdr = NULL;
	char *data = NULL;

	if (!node)
		return NULL;

	param = faux_list_data(node);
	phdr = (faux_phdr_t *)param;
	data = param + sizeof(*phdr);

	if (param_type)
		*param_type = faux_phdr_get_type(phdr);
	if (param_len)
		*param_len = faux_phdr_get_len(phdr);
	if (param_data)
		*param_data = data;

	return phdr;
}


/** @brief Iterate through the message parameters.
 *
 * First parameter (iterator/node) must be initialized first by
 * faux_msg_init_param_iter().
 *
 * @param [in] node Initialized iterator of parameter list.
 * @param [out] param_type Type of parameter.
 * @param [out] param_buf Parameter's data buffer.
 * @param [out] param_len Parameter's data length.
 * @return Pointer to parameter's header or NULL on error.
 */
faux_phdr_t *faux_msg_get_param_each(faux_list_node_t **node,
	uint16_t *param_type, void **param_data, uint32_t *param_len)
{
	faux_list_node_t *current_node = NULL;

	if (!node || !*node)
		return NULL;

	current_node = *node;
	*node = faux_list_next_node(current_node);

	return faux_msg_get_param_by_node(current_node,
		param_type, param_data, param_len);
}


/** @brief Gets message parameter by the index.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] index Parameter's index.
 * @param [out] param_type Type of parameter.
 * @param [out] param_buf Parameter's data buffer.
 * @param [out] param_len Parameter's data length.
 * @return Pointer to parameter's header or NULL on error.
 */
faux_phdr_t *faux_msg_get_param_by_index(const faux_msg_t *msg, unsigned int index,
	uint16_t *param_type, void **param_data, uint32_t *param_len)
{
	faux_list_node_t *iter = NULL;
	unsigned int i = 0;

	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return NULL;
	if (index >= faux_msg_get_param_num(msg)) // Non-existent entry
		return NULL;

	iter = faux_msg_init_param_iter(msg);
	while ((i != index) && iter) {
		i++;
		iter = faux_list_next_node(iter);
	}

	return faux_msg_get_param_by_node(iter,
		param_type, param_data, param_len);
}


/** @brief Gets message parameter by parameter's type.
 *
 * Note message can contain many parameters with the same type. This function
 * will find only the first parameter with specified type. You can iterate
 * through all parameters to find all entries with type you need.
 *
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] param_type Type of parameter.
 * @param [out] param_buf Parameter's data buffer.
 * @param [out] param_len Parameter's data length.
 * @return Pointer to parameter's header or NULL on error.
 */
faux_phdr_t *faux_msg_get_param_by_type(const faux_msg_t *msg,
	uint16_t param_type, void **param_data, uint32_t *param_len)
{
	faux_list_node_t *iter = NULL;

	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return NULL;

	for (iter = faux_msg_init_param_iter(msg);
		iter; iter = faux_list_next_node(iter)) {
		faux_phdr_t *phdr = NULL;
		phdr = (faux_phdr_t *)faux_list_data(iter);
		if (faux_phdr_get_type(phdr) == param_type)
			return faux_msg_get_param_by_node(iter,
				NULL, param_data, param_len);
	}

	// Not found
	return NULL;
}


/** @brief Sends message to network.
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
 * @param [in] msg Allocated faux_msg_t object.
 * @param [in] faux_net Preinitialized faux_net_t object.
 * @return Length of sent data or < 0 on error.
 */
ssize_t faux_msg_send(faux_msg_t *msg, faux_net_t *faux_net)
{
	unsigned int vec_entries_num = 0;
	struct iovec *iov = NULL;
	unsigned int i = 0;
	faux_list_node_t *iter = NULL;
	size_t ret = 0;

	assert(msg);
	assert(msg->hdr);
	if (!msg || !msg->hdr)
		return -1;

	// Calculate number if struct iovec entries.
	// n = (msg header) + ((param hdr) + (param data)) * (param_num)
	vec_entries_num = 1 + (2 * faux_msg_get_param_num(msg));
	iov = faux_zmalloc(vec_entries_num * sizeof(*iov));

	// Message header
	iov[i].iov_base = msg->hdr;
	iov[i].iov_len = sizeof(*msg->hdr);
	i++;

	// Parameter headers
	for (iter = faux_msg_init_param_iter(msg);
		iter; iter = faux_list_next_node(iter)) {
		faux_phdr_t *phdr = NULL;
		phdr = (faux_phdr_t *)faux_list_data(iter);
		iov[i].iov_base = phdr;
		iov[i].iov_len = sizeof(*phdr);
		i++;
	}

	// Parameter data
	for (iter = faux_msg_init_param_iter(msg);
		iter; iter = faux_list_next_node(iter)) {
		faux_phdr_t *phdr = NULL;
		void *data = NULL;
		phdr = (faux_phdr_t *)faux_list_data(iter);
		data = (char *)phdr + sizeof(*phdr);
		iov[i].iov_base = data;
		iov[i].iov_len = faux_phdr_get_len(phdr);
		i++;
	}

	ret = faux_net_sendv(faux_net, iov, vec_entries_num);
	faux_free(iov);

#ifdef DEBUG
	// Debug
	if (msg && ret > 0 && faux_msg_debug_flag) {
		printf("(o) ");
		faux_msg_debug(msg);
	}
#endif

	return ret;
}


/** @brief Receives full message and allocates faux_msg_t object for it.
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
 * @param [in] faux_net Preinitialized faux_net_t object.
 * @param [out] status Status while message receiving. Can be NULL.
 * @return Allocated faux_msg_t object. Object contains received message.
 */
faux_msg_t *faux_msg_recv(faux_net_t *faux_net)
{
	faux_msg_t *msg = NULL;
	size_t received = 0;
	faux_phdr_t *phdr = NULL;
	size_t phdr_whole_len = 0;
	size_t max_data_len = 0;
	unsigned int i = 0;
	char *data = NULL;
	uint32_t param_num = 0;

	msg = faux_msg_allocate();
	assert(msg);
	if (!msg)
		return NULL;

	// Receive message header
	received = faux_net_recv(faux_net, msg->hdr, sizeof(*msg->hdr));
	if (received != sizeof(*msg->hdr)) {
		faux_msg_free(msg);
		return NULL;
	}

	// Receive parameter headers
	param_num = faux_msg_get_param_num(msg);
	if (param_num != 0) {
		phdr_whole_len = param_num * sizeof(*phdr);
		phdr = faux_zmalloc(phdr_whole_len);
		received = faux_net_recv(faux_net, phdr, phdr_whole_len);
		if (received != phdr_whole_len) {
			faux_free(phdr);
			faux_msg_free(msg);
			return NULL;
		}
		// Find out maximum data length
		for (i = 0; i < param_num; i++) {
			size_t cur_data_len = faux_phdr_get_len(phdr + i);
			if (cur_data_len > max_data_len)
				max_data_len = cur_data_len;
		}

		// Receive parameter data
		data = faux_zmalloc(max_data_len);
		for (i = 0; i < param_num; i++) {
			size_t cur_data_len = faux_phdr_get_len(phdr + i);
			if (0 == cur_data_len)
				continue;
			received = faux_net_recv(faux_net, data, cur_data_len);
			if (received != cur_data_len) {
				faux_free(data);
				faux_free(phdr);
				faux_msg_free(msg);
				return NULL;
			}
			faux_msg_add_param_internal(msg,
				faux_phdr_get_type(phdr + i),
				data, cur_data_len, BOOL_FALSE);
		}

		faux_free(data);
		faux_free(phdr);
	}

#ifdef DEBUG
	// Debug
	if (msg && faux_msg_debug_flag) {
		printf("(i) ");
		faux_msg_debug(msg);
	}
#endif

	return msg;
}


/** @brief Prints message debug info.
 *
 * Function prints header values and parameters.
 *
 * @param [in] msg Allocated faux_msg_t object.
 */
void faux_msg_debug(faux_msg_t *msg)
#ifdef DEBUG
{
	faux_list_node_t *iter = 0;
	// Parameter vars
	void *param_data = NULL;
	uint16_t param_type = 0;
	uint32_t param_len = 0;

	assert(msg);
	if (!msg)
		return;

	// Header
	printf("%lx(%u.%u): c%04x s%08x i%08x p%u l%u |%lub\n",
		faux_msg_get_magic(msg),
		faux_msg_get_major(msg),
		faux_msg_get_minor(msg),
		faux_msg_get_cmd(msg),
		faux_msg_get_status(msg),
		faux_msg_get_req_id(msg),
		faux_msg_get_param_num(msg),
		faux_msg_get_len(msg),
		sizeof(*msg->hdr)
		);

	// Parameters
	iter = faux_msg_init_param_iter(msg);
	while (faux_msg_get_param_each(&iter, &param_type, &param_data, &param_len)) {
		printf("  t%04x l%u |%lub\n",
			param_type,
			param_len,
			sizeof(faux_phdr_t) + param_len
			);
	}
}
#else
{
	msg = msg; // Happy compiler
}
#endif
