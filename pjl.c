/**
 * Emit PJL for certain Brother printers.
 * @author Aaron D. Parks
 * @copyright 2022 Parks Digital LLC
 */

#include "parameters.h"
#include "pjl.h"
#include <stdio.h>

/**
 * Emit PJL that is required at the beginning of a job.
 */
void pjl_begin() {
	// Emit Universal Exit Language command and enter PJL mode.
	fputs("\e%-12345X", stdout);
	puts("@PJL");

	// The JOB/EOJ commands can be suppressed. I imagine certain models don't
	// support JOB/EOJ commands? I don't know if the hard-coded name means
	// something to the printer or if it's just a place-holder.
	if (!p_suppress_job)
		puts("@PJL JOB NAME=\"Brother HL-XXX\"");

	// Set Current Environment variables which depend on the selected
	// resolution. Some settings can be suppressed. I suppose different
	// models behave differently.
	switch (p_resolution)	{
		case RES_300:
			if (!p_suppress_ras1200mode_off)
				puts("@PJL SET RAS1200MODE = OFF");
			puts("@PJL SET RESOLUTION = 300");
			break;
		case RES_1200:
			puts("@PJL SET RESOLUTION = 1200");
			puts("@PJL SET PAPERFEEDSPEED=HALF");
			break;
		case RES_HQ1200A:
			puts("@PJL SET RESOLUTION = 600");
			puts("@PJL SET RAS1200MODE = TRUE");
			break;
		case RES_HQ1200B:
			puts("@PJL SET RESOLUTION = 1200");
			puts("@PJL SET PAPERFEEDSPEED=FULL");
			break;
		case RES_600x300:
			puts("@PJL SET RESOLUTION = 600");
			break;
		case RES_600:
		default:
			if (!p_suppress_ras1200mode_off)
				puts("@PJL SET RAS1200MODE = OFF");
			puts("@PJL SET RESOLUTION = 600");
			if (p_emit_hqmmode)
				puts("@PJL SET HQMMODE = ON");
	}

	// Enable or disable toner-saving feature.
	printf("@PJL SET ECONOMODE = %s\n", p_econo_mode ? "ON" : "OFF");

	// Set source tray, unless "MANUAL" was given.
	switch (p_source_tray) {
		case ST_TRAY1:
			puts("@PJL SET SOURCETRAY = TRAY1");
			break;
		case ST_TRAY2:
			puts("@PJL SET SOURCETRAY = TRAY2");
			break;
		case ST_TRAY3:
			puts("@PJL SET SOURCETRAY = TRAY3");
			break;
		case ST_TRAY4:
			puts("@PJL SET SOURCETRAY = TRAY4");
			break;
		case ST_TRAY5:
			puts("@PJL SET SOURCETRAY = TRAY5");
			break;
		case ST_MANUAL:
			break;
		case ST_MPTRAY:
			puts("@PJL SET SOURCETRAY = MPTRAY");
			break;
		case ST_AUTO:
		default:
			puts("@PJL SET SOURCETRAY = AUTO");
	}

	// Set media type.
	switch (p_media_type)	{
		case MT_THIN:
			puts("@PJL SET MEDIATYPE = THIN");
			break;
		case MT_THICK:
			puts("@PJL SET MEDIATYPE = THICK");
			break;
		case MT_THICK2:
			puts("@PJL SET MEDIATYPE = THICK2");
			break;
		case MT_TRANSPARENCY:
			puts("@PJL SET MEDIATYPE = TRANSPARENCY");
			break;
		case MT_ENVELOPES:
			puts("@PJL SET MEDIATYPE = ENVELOPES");
			break;
		case MT_ENVTHICK:
			puts("@PJL SET MEDIATYPE = ENVTHICK");
			break;
		case MT_RECYCLED:
			puts("@PJL SET MEDIATYPE = RECYCLED");
			break;
		case MT_REGULAR:
		default:
			puts("@PJL SET MEDIATYPE = REGULAR");
	}

	// Configure sleep settings. Also sets the defaults, so it sticks. I'm
	// not sure how you'd turn off auto-sleep. Maybe it's not possible.
	if (p_time_out_sleep) {
		puts("@PJL DEFAULT AUTOSLEEP = ON");
		printf("@PJL DEFAULT TIMEOUTSLEEP = %u\n", p_time_out_sleep);
		puts("@PJL SET AUTOSLEEP = ON");
		printf("@PJL SET TIMEOUTSLEEP = %u\n", p_time_out_sleep);
	}

	// I guess the orientation is always portrait.
	puts("@PJL SET ORIENTATION = PORTRAIT");

	// Set paper size name, if appropriate (some paper sizes are set up with
	// a PCL command instead).
	switch (p_paper) {
		case P_EXECUTIVE:
			puts("@PJL SET PAPER = EXECUTIVE");
			break;
		case P_JISB5:
			puts("@PJL SET PAPER = JISB5");
			break;
		case P_B5:
			puts("@PJL SET PAPER = B5");
			break;
		case P_B6:
			puts("@PJL SET PAPER = B6");
			break;
		case P_C5:
			puts("@PJL SET PAPER = C5");
			break;
		case P_DL:
			puts("@PJL SET PAPER = DL");
			break;
		case P_COM10:
			puts("@PJL SET PAPER = COM10");
			break;
		case P_MONARCH:
			puts("@PJL SET PAPER = MONARCH");
			break;
		case P_LEGAL:
		case P_LETTER:
		case P_A4:
		case P_A5:
		case P_A6:
		default:
			break;
	}

	// I think usually this is supposed to reserve a block of memory for the
	// page. I'm not sure exactly what effect it has on these printers or
	// if other values are valid.
	puts("@PJL SET PAGEPROTECT = AUTO");

	// Enter PCL mode.
	puts("@PJL ENTER LANGUAGE = PCL");
}

/**
 * Emit PJL that is required at the end of a job.
 */
void pjl_end() {
	// Unless suppressed, emit Universal Exit Language command to exit from
	// PCL to PJL, then emit a PJL EOJ command to match the JOB command
	// which was emitted at the beginning of the job (they go in pairs).
	if(!p_suppress_job) {
		fputs("\e%-12345X", stdout);
		fputs("@PJL EOJ NAME=\"Brother HL-XXX\"\n", stdout);
	}
	// The last thing emitted is a Universal Exit Language command. I think
	// this is just to be sure the printer is in a known state after the
	// job is finished.
	fputs("\e%-12345X", stdout);
}

