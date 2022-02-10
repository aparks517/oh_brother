/**
 * Implement the compression algorithm used by certain Brother printers.
 * @author Aaron D. Parks
 * @copyright 2022 Parks Digital LLC
 */

#include "compress.h"
#include "parameters.h"
#include <string.h>

size_t count_repeat(const uint8_t *buffer, size_t length);
void encode_repeat(uint8_t **buffer, size_t skip, size_t count,	uint8_t byte);
size_t count_no_repeat(const uint8_t *buffer, size_t length);
void encode(uint8_t **buffer, size_t skip, size_t count, const uint8_t *bytes);
void encode_count(uint8_t **buffer, size_t count);

/**
 * Compress a row of raster data.
 *
 * Two means of compression are employed. First, any bytes of the current row
 * which are the same as in the previous row are skipped. Second, if a byte
 * is repeated three or more times, its value and repetition count are
 * encoded rather than the raw bytes.
 *
 * A compressed row begins with a single byte which holds a count of the
 * groups in the row. A count of 0 means the whole row is the same as the
 * last row. A count of 255 means the row is blank.
 *
 * Before encoding new bytes from the current row, each group encodes how many
 * bytes should be skipped since the end of the last group (the beginning of
 * the row for the first group). These skipped bytes are understood to be
 * the same as in the last row. If the remainder of the row is the same as
 * the last row, no more groups are encoded.
 *
 * If no last row is provided, all bytes in the current row are encoded.
 *
 * The output buffer must be big enough to hold the worst-case compressed
 * output. I think this is only a few bytes larger than the input (how many
 * bytes would depend on the input length and padding), but making it twice
 * as large as the input should be more than safe.
 *
 * Padding (blank, or zero bytes) may be added to the beginning of the row.
 * This is useful for centering a narrower row on a wider paper size (give
 * padding as about half the difference in width).
 *
 * @param out Compressed output buffer
 * @param in Uncompressed input row
 * @param last Last uncompressed input row (may be NULL)
 * @param in_length Length in bytes of the input row and last input row
 * @return Number of bytes of compressed output
 */
size_t compress(uint8_t *out, uint8_t *in, uint8_t *last, size_t in_length) {
	// Initialize number of groups encoded (first byte of output).
	uint8_t *groups = out++;
	*groups = 0;

	// If the row is blank, set the number of groups to the special
	// value 255 and return early.
	bool blank = true;
	for (size_t i = 0; i < in_length; i++)
		if (in[i]) {
			blank = false;
			break;
		}
	if (blank) {
		*groups = 255;
		return out - groups;
	}

	// Prepend padding (if any) as a repeated 0 byte. Must be at least two
	// bytes to encode as a repeat.
	if (p_padding > 1) {
		encode_repeat(&out, 0, p_padding, 0);
		++*groups;
	}

	// While there are bytes in the input line, see how many of the remaining
	// bytes are the same as in the last line (if provided). Then see if
	// there are any bytes which are different. If so, encode them.
	while (in_length) {
		// Skip bytes which are the same as the last line.
		size_t skip = 0;
		if (last)
			while (in_length && *in == *last) {
				 skip++;
				 in++;
				 last++;
				 in_length--;
			}

		// If the rest of the line has been skipped, return early.
		if(!in_length)
			return out - groups;

		// Encode up to the next byte to skip or the end of the line.
		size_t different = last ? 0 : in_length;
		while (different < in_length &&	in[different] != last[different])
			different++;
		while (different) {
			// If the next group is a repeated byte, encode the repeat.
			// Otherwise, encode bytes up to the next repeat (or the next
			// byte which can be skipped or the end of the line).
			size_t count;
			if (different >= 3 && in[0] == in[1] &&	in[0] == in[2]) {
				count = count_repeat(in, different);
				encode_repeat(&out, skip, count, in[0]);
			} else {
				count = count_no_repeat(in, different);
				encode(&out, skip, count, in);
			}
			++*groups;

			// Advance past the encoded bytes.
			in += count;
			last += count;
			in_length -= count;
			different -= count;

			// Reset number of bytes to skip (many groups may be encoded
			// before there are more bytes to skip).
			skip = 0;

			// If the there is only one more group available, stop looking for
			// repeat groups to encode.
			if (*groups >= 253) break;
		}
		// If there is only one more group available, encode the remainder of
		// the line as a single group.
		if(*groups >= 253) {
			encode(&out, 0, in_length, in);
			++*groups;
			in_length = 0;
		}
	} // while there are bytes left in the input line
	return out - groups;
}

/**
 * Count repeated byte.
 *
 * Counts the number of times the first byte in a buffer is repeated. At least
 * the first three bytes of the buffer must be the same.
 *
 * @param buffer Buffer to examine
 * @param length Length of buffer
 * @return Repeat count
 */
size_t count_repeat(const uint8_t *buffer, size_t length) {
	int i;
	for(i = 3; i < length && buffer[i] == buffer[i - 1]; i++);
	return i;
}

/**
 * Encode repeated byte.
 *
 * Encodes a byte which is repeated two or more times. The number of bytes
 * preceding the repeated bytes which are the same as in the last row is also
 * encoded. The buffer is advanced to the next free byte.
 *
 * @param buffer Buffer to hold the encoded data
 * @param skip Number of preceding bytes the same as in the last row
 * @param count Repeat count (at least 2)
 * @param byte The repeated byte
 */
void encode_repeat(uint8_t **buffer, size_t skip, size_t count,	uint8_t byte) {
	// The repeat count is reduced by 2 because "repeat" means at least twice.
	count -= 2;
	// Initialize the first byte of encoded data. For a repeat, the high-bit
	// is always 1.
	**buffer = 1 << 7;
	// The next two bits of the first byte encode the number of preceding
	// bytes the same as in the last row. If the count is more than 2, these
	// bits are set to 3 (all ones).
	**buffer |= (skip > 2 ? 3 : skip) << 5;
	// The final five bits of the first byte encode the repeat count (less 2).
	// If the count (less 2) is more than 30, these bits are set to all ones.
	*(*buffer)++ |= (count > 30 ? 31 : count);
	// If the count of preceding bytes the same as in the last row didn't
	// fit into two bits, append it (less 3) to the buffer.
	if (skip >= 3) encode_count(buffer, skip - 3);
	// If the repeat count didn't fit in five bits, append it (less 31) to
	// the buffer.
	if (count >= 31) encode_count(buffer, count - 31);
	// Append the repeated byte to the buffer.
	*(*buffer)++ = byte;
}

/**
 * Count bytes without repeat.
 *
 * Counts the number of bytes in a buffer before the first group of three
 * repeats of the same byte.
 *
 * @param buffer Buffer to examine
 * @param length Length of buffer
 * @return Number of bytes without repeat
 */
size_t count_no_repeat(const uint8_t *buffer, size_t length) {
	size_t i;
	for (i = length <= 2 ? length : 2; i < length; i++)
		if (buffer[i] == buffer[i - 1] && buffer[i] == buffer[i - 2]) {
			i -= 2;
			break;
		}
	return i;
}

/**
 * Encode bytes.
 *
 * Encodes one or more bytes. The number of bytes preceding the bytes to be
 * encoded which are the same as in the last row is also encoded. The buffer
 * is advanced to the next free byte.
 *
 * @param buffer Buffer to hold the encoded data
 * @param skip Count of bytes the same as the previous row
 * @param count Count of bytes to be encoded (at least 1)
 * @param bytes The bytes to be encoded
 */
void encode(uint8_t **buffer, size_t skip, size_t count,
		const uint8_t *bytes) {
	// The count is reduced by 1 because at least one byte must be encoded.
	count--;
	// Initialize the first byte of encoded data. The high-bit is always 0.
	**buffer = 0;
	// The next four bits of the first byte encode the number of preceding
	// bytes the same as in the last row. If the number is more than 14, these
	// bits are set to 15 (all ones).
	**buffer |= (skip > 14 ? 15 : skip) << 3;
	// The final three bits of the first byte encode the count of bytes
	// (less 1) to be encoded. If the count (less 1) is more than 6, these bits
	// are set to all ones.
	*(*buffer)++ |= (count > 6 ? 7 : count);
	// If the number of preceding bytes the same as in the last row didn't
	// fit into four bits, append it (less 15) to the buffer.
	if (skip >= 15) encode_count(buffer, skip - 15);
	// If the byte count didn't fit into three bits, append it (less 7) to
	// the buffer.
	if (count >= 7) encode_count(buffer, count - 7);
	// Append the encoded bytes to the buffer. Increase the count by 1 first
	// since it was reduced for encoding earlier.
	count++;
	memcpy(*buffer, bytes, count);
	*buffer += count;
}

/**
 * Encode a count.
 *
 * The count is encoded into a variable number of bytes: (count / 255) bytes
 * of 255 followed by one byte of (count % 255). The buffer is advanced to
 * the next free byte.
 *
 * @param **buffer Buffer to hold the encoded count
 * @param count Count to encode
 */
void encode_count(uint8_t **buffer, size_t count) {
	for (size_t i = 0; i < count / 255; i++)
		*(*buffer)++ = 255;
	*(*buffer)++ = count % 255;
}
