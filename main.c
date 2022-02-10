/**
 * Filter raster data for certain Brother printers.
 * @author Aaron D. Parks
 * @copyright 2022 Parks Digital LLC
 */

#include "parameters.h"
#include "pcl.h"
#include "pjl.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

int main(int argc, char **argv) {
	// Get parameters from program arguments.
	for(size_t i = 2; i < argc; i += 2) {
		if (!strcmp(argv[i - 1], "-resolution"))
			param_resolution(argv[i]);
		else if (!strcmp(argv[i - 1], "-econo_mode"))
			param_econo_mode(argv[i]);
		else if (!strcmp(argv[i - 1], "-source_tray"))
			param_source_tray(argv[i]);
		else if (!strcmp(argv[i - 1], "-media_type"))
			param_media_type(argv[i]);
		else if (!strcmp(argv[i - 1], "-time_out_sleep"))
			param_time_out_sleep(argv[i]);
		else if (!strcmp(argv[i - 1], "-paper"))
			param_paper(argv[i]);
		else if (!strcmp(argv[i - 1], "-suppress_job"))
			param_suppress_job(argv[i]);
		else if (!strcmp(argv[i - 1], "-emit_hqmmode"))
			param_emit_hqmmode(argv[i]);
		else if (!strcmp(argv[i - 1], "-suppress_ras1200mode_off"))
			param_suppress_ras1200mode_off(argv[i]);
		else if (!strcmp(argv[i - 1], "-copies"))
			param_copies(argv[i]);
		else if (!strcmp(argv[i - 1], "-duplex"))
			param_duplex(argv[i]);
		else if (!strcmp(argv[i - 1], "-width"))
			param_width(argv[i]);
		else if (!strcmp(argv[i - 1], "-height"))
			param_height(argv[i]);
		else
			errx(EX_USAGE, "unrecognized argument %s", argv[i - 1]);
	}

	// Update defaults, validate parameters, and calculate padding.
	param_validate();

	// Allocate a buffer for one page of input.
	size_t row_length = (p_width + 7) >> 3;
	uint8_t *page = calloc(p_height, row_length);
	if (!page) err(EX_OSERR, "allocate page buffer");

	// Set up the printer for this job.
	pjl_begin();
	pcl_begin();

	// Read, compress, and emit one page at a time until the input data
	// is consumed. Don't process partial pages of data.
	while (fread(page, row_length, p_height, stdin) == p_height)
		pcl_page(page, row_length, p_height);

	// Wrap up the job and put the printer back in a known state.
	pjl_end();
}
