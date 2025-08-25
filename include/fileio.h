#ifndef __H__FILEIO_H___
#define __H__FILEIO_H___

enum e_filetobuf_error {
	ERR_F2B_SUCCESS = 0,

	ERR_F2B_ZERO_SIZE_BUFFER = 1,
	ERR_F2B_FAILED_OPEN = 2,
	ERR_F2B_FAILED_SEEK = 3,
	ERR_F2B_FAILED_GET_SIZE = 4,
	ERR_F2B_BUFFER_TOO_SMALL = 5,
	ERR_F2B_FAILED_READ = 6,

	ERR_F2B_FAILED_CLOSE = 0x1000,
};

enum e_filetobuf_error f_io_filetobuf (
	const char*, unsigned int*, char*, unsigned int
);

#endif
