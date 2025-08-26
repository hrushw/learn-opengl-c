#include <stdio.h>

#include "fileio.h"

enum e_filetobuf_error f_io_filetobuf(
	const char* path, unsigned int* len,
	char* buf, unsigned int buflen
) {
	if(!buflen) return ERR_F2B_ZERO_SIZE_BUFFER;

	FILE* f = fopen(path, "rb");
	if(!f) return ERR_F2B_FAILED_OPEN;

	/* Attempts in order: */
	/* 1 . Check if file opened successfully */
	/* 2 . Seek to end of file */
	/* 3 . Get file size */
	/* 4 . Check if buffer is large enough to store the file */
	/* 5 . Rewind and read contents into buffer */
	/* read size is only useful on success or failures 4, 5, else set to 0 */
	/* If read operation fails, set buf[0] = 0 to avoid using corrupted data */

	long l = 0;
	int ret = ERR_F2B_SUCCESS;
	if( fseek(f, 0L, SEEK_END) == -1 )
		ret = ERR_F2B_FAILED_SEEK;
	else if( (l = ftell(f)) < 0 )
		l = 0, ret = ERR_F2B_FAILED_GET_SIZE;
	else if( l > buflen - 1 )
		ret = ERR_F2B_BUFFER_TOO_SMALL;
	else if( rewind(f), fread(buf, 1, l, f) != (size_t)l )
		buf[0] = 0, ret = ERR_F2B_FAILED_READ;
	else
		buf[l] = 0;

	if(len) *len = l;

	return fclose(f) ? ( ERR_F2B_FAILED_CLOSE | ret ) : ret;
}

