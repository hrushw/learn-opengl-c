#include <stdio.h>

#include "fileio.h"

/* Read file into buffer as null-terminated string */
/* Returns 0 on success, nonzero on failure */
enum e_filetobuf_error f_io_filetobuf(
	const char* path, unsigned int* len,
	char* buf, unsigned int buflen
) {
	if(!buflen) return ERR_F2B_ZERO_SIZE_BUFFER;

	/* Open file */
	FILE* f = fopen(path, "rb");
	long l = 0;
	int ret = ERR_F2B_SUCCESS;

	/* Attempts in order: */
	/* 1 . Check if file opened successfully */
	/* 2 . Seek to end of file */
	/* 3 . Get file size */
	/* 4 . Check if buffer is large enough to store the file */
	/* 5 . Seek to start of file and read contents into buffer */
	/* read size is only useful on success or failures 4, 5, else set to 0 */

	if(!f)
		return ERR_F2B_FAILED_OPEN;
	else if( fseek(f, 0L, SEEK_END) == -1 )
		ret = ERR_F2B_FAILED_SEEK;
	else if( (l = ftell(f)) < 0 )
		l = 0, ret = ERR_F2B_FAILED_GET_SIZE;
	else if( l > buflen - 1 )
		ret = ERR_F2B_BUFFER_TOO_SMALL;
	else if( rewind(f), fread(buf, sizeof(char), l, f) != (size_t)l )
		ret = ERR_F2B_FAILED_READ;
	else {
		/* Add null terminator */
		buf[l] = 0;
		goto end;
	}

	buf[0] = 0;

	end:
	/* not bothering to check long to int conversion */
	/* length is set to size excluding null terminator */
	if(len) *len = l;
	if(fclose(f)) return ERR_F2B_FAILED_CLOSE;
	return ret;
}

