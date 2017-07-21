#include "fwload.h"

#include "error.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/////////////////////////////////////////////////////////////////////////
// Interface functions
/////////////////////////////////////////////////////////////////////////

error_t firmware_load(mem_addr loc, const char *filename)
{
	FILE *fw_file = fopen(filename, "rb");
	if (fw_file == NULL) {
		return ERR_FILE;
	}

	if (fseek(fw_file, 0, SEEK_END) != 0) {
		return ERR_FILE;
	}

	long fw_size = ftell(fw_file);
	if (fw_size == -1L) {
		return ERR_FILE;
	}

	uint8_t *fw_buf = malloc(fw_size);
	if (fw_buf == NULL) {
		return ERR_NOMEM;
	}

	rewind(fw_file);
	size_t read = fread(fw_buf, 1, fw_size, fw_file);

	if ((long)read != fw_size) {
		free(fw_buf);
		return ERR_FILE;
	}

	mem_write_mem(loc, fw_buf, read);

	free(fw_buf);
	fclose(fw_file);

	return ERR_NOERR;
}

