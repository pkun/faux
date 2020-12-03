/** @file proto.h
 *
 * @brief Library to implement simple network protocol
 *
 * CRSP is a protocol to send requests to CRSP server. CRSP server stores
 * information about CDPs, periodically renews CRLs.
 *
 * The CRSP protocol supposes request/answer method. Each request must have an
 * answer. The CRSP server is synchronous. It means that server will answer
 * immediately or nearly immediately. It can plans a long-time operations but
 * it sends answer to client immediately. For example client asks for status
 * of specified CDP but server has no such CDP in its database. So server will
 * answer with status something like "has no such CDP yet" immediately but it
 * schedules the long-time task to get CDP information. Later client can send
 * the same request but now server already has a status for specified CDP and
 * answers with correspondent status.
 *
 * Each CRSP protocol message (request or answer) has the same structure:
 *
 * [ CRSP header ]
 * [ CRSP first parameter's header ]
 * [ ... ]
 * [ CRSP n-th parameter's header ]
 * [ First parameter's data ]
 * [ ... ]
 * [ N-th parameter's data ]
 *
 * The length of CRSP message can vary. CRSP header has constant length and
 * contains total length of whole message and number of parameters. It also
 * contains magic number, CRSP version number, CRSP command, status and
 * request ID (to identify request/answer). There are parameter header for each
 * parameter. Parameter's header contains parameter type and length of
 * parameter. After parameter headers array the parameter's data follows in
 * a sequence.
 */


#ifndef _faux_msg_h
#define _faux_msg_h

#include <stdint.h>
#include <signal.h>
#include <faux/faux.h>
#include <faux/list.h>
#include <faux/net.h>

typedef struct faux_msg_s faux_msg_t;

// Debug variable. BOOL_TRUE for debug and BOOL_FALSE to switch debug off
extern bool_t faux_msg_debug;


/** @brief Parameter header
 */
typedef struct faux_phdr_s {
	uint16_t param_type; // Parameter type
	uint8_t reserved[2];
	uint32_t param_len; // Length of parameter (not including header)
} faux_phdr_t;


/** @brief Message header
 */
typedef struct faux_hdr_s {
	uint32_t magic; // Magic number
	uint8_t major; // Major protocol version number
	uint8_t minor; // Minor protocol version number
	uint16_t cmd; // Command
	uint32_t status; // Status
	uint32_t req_id; // Some abstract ID of request. Identifies request/answer
	uint32_t param_num; // Number of parameters
	uint32_t len; // Length of whole message (including header)
	faux_phdr_t phdr[]; // Parameter headers (unknown length)
} crsp_hdr_t;


// Debug variable. 1 for debug and 0 to switch debug off
extern int crsp_debug;


C_DECL_BEGIN

// Parameter functions
void faux_phdr_set_type(faux_phdr_t *phdr, uint16_t param_type);
uint16_t faux_phdr_get_type(const faux_phdr_t *phdr);
void faux_phdr_set_len(faux_phdr_t *phdr, uint32_t param_len);
uint32_t faux_phdr_get_len(const faux_phdr_t *phdr);

// Message functions
crsp_msg_t *crsp_msg_new(void);
void crsp_msg_free(crsp_msg_t *crsp_msg);
void crsp_msg_set_cmd(crsp_msg_t *crsp_msg, crsp_cmd_e cmd);
crsp_cmd_e crsp_msg_get_cmd(const crsp_msg_t *crsp_msg);
void crsp_msg_set_status(crsp_msg_t *crsp_msg, crsp_status_e status);
crsp_status_e crsp_msg_get_status(const crsp_msg_t *crsp_msg);
void crsp_msg_set_req_id(crsp_msg_t *crsp_msg, uint32_t req_id);
uint32_t crsp_msg_get_req_id(const crsp_msg_t *crsp_msg);
uint32_t crsp_msg_get_param_num(const crsp_msg_t *crsp_msg);
uint32_t crsp_msg_get_len(const crsp_msg_t *crsp_msg);
ssize_t crsp_msg_add_param(crsp_msg_t *crsp_msg, crsp_param_e type,
	const void *buf, size_t len);
faux_list_node_t *crsp_msg_init_param_iter(const crsp_msg_t *crsp_msg);
crsp_phdr_t *crsp_msg_get_param_each(faux_list_node_t **node,
	crsp_param_e *param_type, void **param_data, uint32_t *param_len);
crsp_phdr_t *crsp_msg_get_param_by_index(const crsp_msg_t *crsp_msg, unsigned int index,
	crsp_param_e *param_type, void **param_data, uint32_t *param_len);
crsp_phdr_t *crsp_msg_get_param_by_type(const crsp_msg_t *crsp_msg,
	crsp_param_e param_type, void **param_data, uint32_t *param_len);
ssize_t crsp_msg_send(crsp_msg_t *crsp_msg, faux_net_t *faux_net);
crsp_msg_t *crsp_msg_recv(faux_net_t *faux_net, crsp_recv_e *status);
char *crsp_msg_get_param_cdp_uri(const crsp_msg_t *crsp_msg);
void crsp_msg_debug(crsp_msg_t *crsp_msg);

C_DECL_END

#endif // _faux_msg_h

