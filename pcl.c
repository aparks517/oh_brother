/**
 * Emit PCL for certain Brother printers.
 * @author Aaron D. Parks
 * @copyright 2022 Parks Digital LLC
 */

#include "compress.h"
#include "parameters.h"
#include "pcl.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

void raster_data(uint8_t *buffer, size_t *buffer_length, uint8_t *buffer_rows,
	const uint8_t *row, size_t row_length);

/**
 * Emit PCL that is required at the beginning of a job.
 */
void pcl_begin() {
	// Printer Reset command. I think this just resets the PCL environment,
	// not the whole printer.
	fputs("\eE", stdout);

	// Some page sizes are set up with PJL and some are set up with a
	// hard-coded PCL command. All commands appear to set page size to
	// 4096 (undocumented), then set page size to the standard value for
	// the selected paper, followed by setting the line spacing to 6 LPI
	// and the top margin to one line (1/6").
	switch (p_paper) {
		case P_LEGAL:
			fputs("\e&l4096a3a6d1E", stdout);
			break;
		case P_LETTER:
			fputs("\e&l4096a2a6d1E", stdout);
			break;
		case P_A4:
			fputs("\e&l4096a26a6d1E", stdout);
			break;
		case P_A5:
			fputs("\e&l4096a25a6d1E", stdout);
			break;
		case P_A6:
			fputs("\e&l4096a24a6d1E", stdout);
			break;
		case P_EXECUTIVE:
		case P_JISB5:
		case P_B5:
		case P_B6:
		case P_C5:
		case P_DL:
		case P_COM10:
		case P_MONARCH:
		default:
			break;
	}

	// Unit of Measure and Raster Graphics Resolution depend on the
	// selected printer resolution.
	switch (p_resolution) {
		case RES_300:
			fputs("\e&u300D", stdout);
			fputs("\e*t300R", stdout);
			break;
		case RES_1200:
		case RES_HQ1200B:
			fputs("\e&u1200D", stdout);
			fputs("\e*t1200R", stdout);
			break;
		case RES_HQ1200A:
			fputs("\e&u1200D", stdout);
			fputs("\e*t600R" , stdout);
			break;
		case RES_600:
		case RES_600x300:
		default:
			fputs("\e&u600D", stdout);
			fputs("\e*t600R", stdout);
			break;
	}

	// If the source tray is manual, set the paper source to manual feed.
	if (p_source_tray == ST_MANUAL)
		fputs("\e&l2H", stdout);

	// Set number of copies if more than one.
	if (p_copies > 1)
		printf("\e&l%dX", p_copies);

	// Duplex type (no need to emit a command for simplex).
	switch (p_duplex) {
		case DPX_LONG:
			fputs("\e&l1S", stdout);
			break;
		case DPX_SHORT:
			fputs("\e&l2S", stdout);
			break;
		case DPX_OFF:
		default:
			break;
	}
}

/**
 * Emit PCL for one page of raw data.
 * @param in Input data buffer
 * @param row_length Length of input data rows in bytes
 * @param row_count Number of input data rows
 */
void pcl_page(uint8_t *in, size_t row_length, size_t row_count) {
	// Begin a continuing Set Compression Method command. The method parameter
	// is set to 1030, which appears to be proprietary and undocumented. The
	// parameter character is given in lower-case, so more parameters can be
	// given. Transfer Raster Data parameters will be added for each block
	// of 128 rows or 16kB, whichever is less. After all the data parameters
	// for the page have been emitted, a final Set Compression Method
	// parameter will be added with an upper-case parameter character to
	// conclude the command.
	fputs("\e*b1030m", stdout);

	// Horizontal and vertical margins are 1/6". The size in bytes or rows
	// depends on the resolution mode. Calculate the printable length in
	// bytes of each row and the number of printable rows within these margins.
	// The printable length is limited to 16.64". Advance the input buffer to
	// the first printable byte.
	size_t printable_length, printable_rows;
	switch (p_resolution) {
		case RES_300:
			printable_length = row_length - 12;
			if (printable_length > 624) printable_length = 624;
			printable_rows = row_count - 100;
			in += 50 * row_length + 6;
			break;
		case RES_1200:
		case RES_HQ1200A:
		case RES_HQ1200B:
			printable_length = row_length - 50;
			if (printable_length + p_padding > 2496)
				printable_length = 2496 - p_padding;
			printable_rows = row_count - 400;
			in += 200 * row_length + 25;
			break;
		case RES_600x300:
			printable_length = row_length - 24;
			if (printable_length + p_padding > 1248)
				printable_length = 1248 - p_padding;
			printable_rows = row_count - 100;
			in += 50 * row_length + 12;
			break;
		case RES_600:
		default:
			printable_length = row_length - 24;
			if (printable_length + p_padding > 1248)
				printable_length = 1248 - p_padding;
			printable_rows = row_count - 200;
			in += 100 * row_length + 12;
	}

	// Initialize the output block buffer. Output block size is limited
	// to the lesser of 128 rows or 16kB.
	uint8_t *out_block = calloc(16384, sizeof(uint8_t));
	if (!out_block) err(EX_OSERR, "allocate output block buffer");
	size_t block_len = 0;
	uint8_t block_rows = 0;

	// Initialize the output row buffer. I think twice the input row length is
	// more than enough.
	uint8_t *out_row = calloc(2, printable_length);
	if (!out_row) err(EX_OSERR, "allocate output row buffer");

	// Compress each input row and put it into the output block buffer. When
	// the block buffer is full, emit it as a continuing raster data parameter
	// for the ongoing command.
	for (size_t row = 0; row < printable_rows; row++) {
		// In HQ1200A resolution mode, encode odd lines as duplicates of even
		// lines and skip over the input. I'm guessing it's a sort-of 1200x600
		// mode that takes 1200x1200 input?
		if (p_resolution == RES_HQ1200A && row & 1) {
			out_row[0] = 0;
			raster_data(out_block, &block_len, &block_rows, out_row, 1);
			in += row_length;
			continue;
		}

		// Compress the printable part of the row and append it to the
		// output block buffer, then advance to the next input row. The
		// last line is not used for compressing the first row of a block.
		// I think a new block resets the printer's last-row buffer.
		uint8_t *last_row = (block_rows < 128 && row) ? in - row_length : 0;
		size_t out_length = compress(out_row, in, last_row, printable_length);
		raster_data(out_block, &block_len, &block_rows, out_row, out_length);
		in += row_length;

		// In 600x300 resolution mode, encode a duplicate line after each
		// input line. I guess I'm not sure if this is purely a "software"
		// mode to save communication time or if the printer can do some
		// optimization too.
		if (p_resolution == RES_600x300) {
			out_row[0] = 0;
			raster_data(out_block, &block_len, &block_rows, out_row, 1);
		}
	}

	// All rows have been compressed, no need for this buffer anymore.
	free(out_row);

	// If there are any rows in the output block buffer, emit one more
	// continuing raster data parameter.
	if (block_len) {
		fprintf(stdout, "%zuw%c%c", block_len + 2, 0, block_rows);
		fwrite(out_block, 1, block_len, stdout);
	}

	// All rows have been emitted, no need for this buffer anymore.
	free(out_block);

	// Conclude the ongoing command with a (redundant?) Set Compression
	// Method parameter (upper-case to end the command).
	fputs("1030M\f", stdout);
}

/**
 * Buffer (and possibly emit) raster data.
 *
 * If the given block buffer is already full (by bytes or by rows), it is
 * emitted and the buffer is reset. Then the given row of raster data is
 * appended to the buffer.
 *
 * It is assumed that the data emitted by this function will be part of a
 * parameterized command with the same parameterized character and group
 * character as the Transfer Raster Data command (*b). The parameter
 * character (w) is emitted in lower-case so that more parameters may be
 * added to the command. The command must eventually be concluded with
 * an upper-case parameter.
 *
 * @param buffer Block buffer
 * @param buffer_length Number of bytes in the block buffer
 * @param buffer_rows Number of rows in the block buffer
 * @param row Row data to be appended
 * @param row_length Number of bytes in the row
 */
void raster_data(uint8_t *buffer, size_t *buffer_length, uint8_t *buffer_rows,
		const uint8_t *row, size_t row_length) {
	// Flush the buffer if it's full by bytes or rows
	if(row_length + *buffer_length > 16384 || *buffer_rows >= 128)
	{
		printf("%zuw%c%c", *buffer_length + 2, 0, *buffer_rows);
		fwrite(buffer, sizeof(*buffer), *buffer_length, stdout);
		*buffer_length = 0;
		*buffer_rows = 0;
	}
	// Append row to buffer
	memcpy(buffer + *buffer_length, row, row_length);
	*buffer_length += row_length;
	++*buffer_rows;
}
