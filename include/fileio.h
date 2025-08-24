#ifndef __H__FILEIO_H___
#define __H__FILEIO_H___

enum e_filetobuf_error {
	ERR_F2B_SUCCESS = 0,

	ERR_F2B_ZERO_SIZE_BUFFER,
	ERR_F2B_FAILED_OPEN,
	ERR_F2B_FAILED_SEEK,
	ERR_F2B_FAILED_GET_SIZE,
	ERR_F2B_BUFFER_TOO_SMALL,
	ERR_F2B_FAILED_READ,
	ERR_F2B_FAILED_CLOSE,
};

enum e_filetobuf_error f_io_filetobuf (
	const char*, unsigned int*, char*, unsigned int
);

#endif
