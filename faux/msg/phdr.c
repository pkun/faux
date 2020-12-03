/** @file phdr.c
 * @brief Class represents a parameter header.
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
#include <arpa/inet.h>

#include <faux/faux.h>
#include <faux/str.h>
#include <faux/list.h>
#include <faux/msg.h>


/** @brief Sets type to parameter header.
 *
 * @param [in] phdr Allocated faux_phdr_t object.
 * @param [in] param_type Type of parameter.
 */
void faux_phdr_set_type(faux_phdr_t *phdr, uint16_t param_type)
{
	assert(phdr);
	if (!phdr)
		return;
	phdr->param_type = htons(param_type);
}

/** @brief Gets type from parameter header.
 *
 * @param [in] phdr Allocated faux_phdr_t object.
 * @return Type of parameter or 0 on error.
 */
uint16_t faux_phdr_get_type(const faux_phdr_t *phdr)
{
	assert(phdr);
	if (!phdr)
		return 0;

	return ntohs(phdr->param_type);
}


/** @brief Sets length to parameter header.
 *
 * @param [in] phdr Allocated faux_phdr_t object.
 * @param [in] param_len Length of parameter.
 */
void faux_phdr_set_len(faux_phdr_t *phdr, uint32_t param_len)
{
	assert(phdr);
	if (!phdr)
		return;
	phdr->param_len = htonl(param_len);
}


/** @brief Gets length from parameter header.
 *
 * @param [in] phdr Allocated faux_phdr_t object.
 * @return Length of parameter or 0 on error.
 */
uint32_t faux_phdr_get_len(const faux_phdr_t *phdr)
{
	assert(phdr);
	if (!phdr)
		return 0;

	return ntohl(phdr->param_len);
}
