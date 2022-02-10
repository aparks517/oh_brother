/**
 * Set parameter defaults and parse from arguments.
 *
 * The manual page describes the function of each of these parameters.
 *
 * @author Aaron D. Parks
 * @copyright 2022 Parks Digital LLC
 */

#include "parameters.h"
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

enum Resolution p_resolution = RES_600;
bool p_econo_mode = false;
enum SourceTray p_source_tray = ST_AUTO;
enum MediaType p_media_type = MT_REGULAR;
unsigned int p_time_out_sleep = 0;
enum Paper p_paper = P_LETTER;
bool p_suppress_job = false;
bool p_emit_hqmmode = false;
bool p_suppress_ras1200mode_off = false;
unsigned int p_copies = 1;
enum Duplex p_duplex = DPX_OFF;
size_t p_width = 0;
size_t p_height = 0;
size_t p_padding = 0;

void param_resolution(const char *arg) {
	if (!strcmp(arg, "300")) p_resolution = RES_300;
	else if (!strcmp(arg, "600")) p_resolution = RES_600;
	else if (!strcmp(arg, "1200")) p_resolution = RES_1200;
	else if (!strcmp(arg, "HQ1200A")) p_resolution = RES_HQ1200A;
	else if (!strcmp(arg, "HQ1200B")) p_resolution = RES_HQ1200B;
	else if (!strcmp(arg, "600x300")) p_resolution = RES_600x300;
	else errx(EX_USAGE, "resolution must be one of "
		"300, 600, 1200, HQ1200A, HQ1200B, or 600x300");
}

void param_econo_mode(const char *arg) {
	if (!strcmp(arg, "OFF")) p_econo_mode = false;
	else if (!strcmp(arg, "ON")) p_econo_mode = true;
	else errx(EX_USAGE, "econo_mode must be one of "
		"OFF or ON");
}

void param_source_tray(const char *arg) {
	if (!strcmp(arg, "AUTO")) p_source_tray = ST_AUTO;
	else if (!strcmp(arg, "TRAY1")) p_source_tray = ST_TRAY1;
	else if (!strcmp(arg, "TRAY2")) p_source_tray = ST_TRAY2;
	else if (!strcmp(arg, "TRAY3")) p_source_tray = ST_TRAY3;
	else if (!strcmp(arg, "TRAY4")) p_source_tray = ST_TRAY4;
	else if (!strcmp(arg, "TRAY5")) p_source_tray = ST_TRAY5;
	else if (!strcmp(arg, "MANUAL")) p_source_tray = ST_MANUAL;
	else if (!strcmp(arg, "MPTRAY")) p_source_tray = ST_MPTRAY;
	else errx(EX_USAGE, "source_tray must be one of "
		"AUTO, TRAY1, TRAY2, TRAY3, TRAY4, TRAY5, MANUAL, or MPTRAY");
}

void param_media_type(const char *arg) {
	if (!strcmp(arg, "REGULAR")) p_media_type = MT_REGULAR;
	else if (!strcmp(arg, "THIN")) p_media_type = MT_THIN;
	else if (!strcmp(arg, "THICK")) p_media_type = MT_THICK;
	else if (!strcmp(arg, "THICK2")) p_media_type = MT_THICK2;
	else if (!strcmp(arg, "TRANSPARENCY")) p_media_type = MT_TRANSPARENCY;
	else if (!strcmp(arg, "ENVELOPES")) p_media_type = MT_ENVELOPES;
	else if (!strcmp(arg, "ENVTHICK")) p_media_type = MT_ENVTHICK;
	else if (!strcmp(arg, "RECYCLED")) p_media_type = MT_RECYCLED;
	else errx(EX_USAGE, "media_type must be one of "
		"REGULAR, THIN, THICK, THICK2, TRANSPARENCY, ENVELOPES, "
		"ENVTHICK, or RECYCLED");
}

void param_time_out_sleep(const char *arg) {
	if (!sscanf(arg, "%u", &p_time_out_sleep))
		errx(EX_USAGE, "time_out_sleep must be an unsigned integer");
	if (p_time_out_sleep > 99)
		errx(EX_USAGE, "time_out_sleep must be no more than 99");
}

void param_paper(const char *arg) {
	if (!strcmp(arg, "LEGAL")) p_paper = P_LEGAL;
	else if (!strcmp(arg, "LETTER")) p_paper = P_LETTER;
	else if (!strcmp(arg, "A4")) p_paper = P_A4;
	else if (!strcmp(arg, "EXECUTIVE")) p_paper = P_EXECUTIVE;
	else if (!strcmp(arg, "JISB5")) p_paper = P_JISB5;
	else if (!strcmp(arg, "B5")) p_paper = P_B5;
	else if (!strcmp(arg, "A5")) p_paper = P_A5;
	else if (!strcmp(arg, "B6")) p_paper = P_B6;
	else if (!strcmp(arg, "A6")) p_paper = P_A6;
	else if (!strcmp(arg, "C5")) p_paper = P_C5;
	else if (!strcmp(arg, "DL")) p_paper = P_DL;
	else if (!strcmp(arg, "COM10")) p_paper = P_COM10;
	else if (!strcmp(arg, "MONARCH")) p_paper = P_MONARCH;
	else errx(EX_USAGE, "paper must be one of "
		"LEGAL, LETTER, A4, EXECUTIVE, JISB5, B5, A5, B6, A6, "
		"C5, DL, COM10, or MONARCH");
}

void param_suppress_job(const char *arg) {
	if (!strcmp(arg, "NO")) p_suppress_job = false;
	else if (!strcmp(arg, "YES")) p_suppress_job = true;
	else errx(EX_USAGE, "suppress_job must be one of "
		"NO or YES");
}

void param_emit_hqmmode(const char *arg) {
	if (!strcmp(arg, "NO")) p_emit_hqmmode = false;
	else if (!strcmp(arg, "YES")) p_emit_hqmmode = true;
	else errx(EX_USAGE, "emit_hqmmode must be one of "
		"NO or YES");
}

void param_suppress_ras1200mode_off(const char *arg) {
	if (!strcmp(arg, "NO")) p_suppress_ras1200mode_off = false;
	else if (!strcmp(arg, "YES")) p_suppress_ras1200mode_off = true;
	else errx(EX_USAGE, "suppress_ras1200mode_off must be one of "
		"NO or YES");
}

void param_copies(const char *arg) {
	if (!sscanf(arg, "%u", &p_copies))
		errx(EX_USAGE, "copies must be an unsigned integer");
	if (p_copies < 1)
		errx(EX_USAGE, "copies must be at least 1");
	if (p_copies > 999)
		errx(EX_USAGE, "copies must be no more than 999");
}

void param_duplex(const char *arg) {
	if (!strcmp(arg, "OFF")) p_duplex = DPX_OFF;
	else if (!strcmp(arg, "LONG")) p_duplex = DPX_LONG;
	else if (!strcmp(arg, "SHORT")) p_duplex = DPX_SHORT;
	else errx(EX_USAGE, "duplex must be one of "
		"OFF, LONG, or SHORT");
}

void param_width(const char *arg) {
	if (!sscanf(arg, "%lu", &p_width))
		errx(EX_USAGE, "width must be an unsigned long");
}

void param_height(const char *arg) {
	if (!sscanf(arg, "%lu", &p_height))
		errx(EX_USAGE, "height must be an unsigned long");
}

/**
 * Set defaults, validate parameters, calculate padding.
 *
 * If width and height are not set, set them to the selected page height
 * and width. Check that the height and width fit the selected page.
 * Calculate padding to center input data width on the page.
 */
void param_validate() {
	// Get width and height in dots at 120 DPI from selected paper.
	size_t paper_width, paper_height;
	switch (p_paper) {
		case P_LEGAL:
			paper_width = 1020;
			paper_height = 1680;
  			break;
		case P_A4:
			paper_width = 992;
			paper_height = 1403;
  			break;
		case P_EXECUTIVE:
			paper_width = 870;
			paper_height = 1260;
  			break;
		case P_JISB5:
			paper_width = 860;
			paper_height = 1214;
  			break;
		case P_B5:
			paper_width = 832;
			paper_height = 1180;
  			break;
		case P_A5:
			paper_width = 701;
			paper_height = 992;
  			break;
		case P_B6:
			paper_width = 590;
			paper_height = 832;
  			break;
		case P_A6:
			paper_width = 496;
			paper_height = 701;
  			break;
		case P_C5:
			paper_width = 767;
			paper_height = 1082;
  			break;
		case P_DL:
			paper_width = 520;
			paper_height = 1039;
  			break;
		case P_COM10:
			paper_width = 495;
			paper_height = 1140;
  			break;
		case P_MONARCH:
			paper_width = 465;
			paper_height = 900;
  			break;
		case P_LETTER:
		default:
			paper_width = 1020;
			paper_height = 1320;
	}

	// Bring paper width and height to dots at the selected resolution.
	switch(p_resolution) {
		case RES_300:
			paper_width *= 5; paper_width /= 2;
			paper_height *= 5; paper_height /= 2;
			break;
		case RES_1200:
		case RES_HQ1200A:
		case RES_HQ1200B:
			paper_width *= 10;
			paper_height *= 10;
			break;
		case RES_600x300:
			paper_width *= 5;
			paper_height *= 5; paper_height /= 2;
			break;
		case RES_600:
		default:
			paper_width *= 5;
			paper_height *= 5;
	}

	// Set input data with and height if not set.
	if (!p_width) p_width = paper_width;
	if (!p_height) p_height = paper_height;

	// Validate width and height of input data fit on the selected paper.
	if (p_width > paper_width)
		errx(EX_USAGE, "width must not be greater than paper width");
	if (p_height > paper_height)
		errx(EX_USAGE, "height must not be greater than paper height");

	// Calculate padding in bytes to place the input data in the middle
	// of the page. Rounds down to the nearest byte.
	p_padding = ((paper_width - p_width) / 2) >> 3;
}
