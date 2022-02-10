#include <stdbool.h>
#include <stddef.h>

extern enum Resolution {
	RES_300,
	RES_600,
	RES_1200,
	RES_HQ1200A,
	RES_HQ1200B,
	RES_600x300
} p_resolution;

extern bool p_econo_mode;

extern enum SourceTray {
	ST_AUTO,
	ST_TRAY1,
	ST_TRAY2,
	ST_TRAY3,
	ST_TRAY4,
	ST_TRAY5,
	ST_MANUAL,
	ST_MPTRAY
} p_source_tray;

extern enum MediaType {
	MT_REGULAR,
	MT_THIN,
	MT_THICK,
	MT_THICK2,
	MT_TRANSPARENCY,
	MT_ENVELOPES,
	MT_ENVTHICK,
	MT_RECYCLED
} p_media_type;

extern unsigned int p_time_out_sleep;

extern enum Paper {
	P_LEGAL,
	P_LETTER,
	P_A4,
	P_EXECUTIVE,
	P_JISB5,
	P_B5,
	P_A5,
	P_B6,
	P_A6,
	P_C5,
	P_DL,
	P_COM10,
	P_MONARCH
} p_paper;

extern bool p_suppress_job;
extern bool p_emit_hqmmode;
extern bool p_suppress_ras1200mode_off;
extern unsigned int p_copies;

extern enum Duplex {
	DPX_OFF,
	DPX_LONG,
	DPX_SHORT
} p_duplex;

extern size_t p_width;
extern size_t p_height;
extern size_t p_padding;

void param_resolution(const char *arg);
void param_econo_mode(const char *arg);
void param_source_tray(const char *arg);
void param_media_type(const char *arg);
void param_time_out_sleep(const char *arg);
void param_paper(const char *arg);
void param_suppress_job(const char *arg);
void param_emit_hqmmode(const char *arg);
void param_suppress_ras1200mode_off(const char *arg);
void param_copies(const char *arg);
void param_duplex(const char *arg);
void param_width(const char *arg);
void param_height(const char *arg);
void param_validate();
