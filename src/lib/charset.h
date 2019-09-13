/*
 */

#ifndef _CHARSET_H_
#define _CHARSET_H_

struct codepage {
	char * name;

	unsigned char* from;
	unsigned char* to;
};

extern struct codepage codepages[];
//#define NCODEPAGES (sizeof(codepages) / sizeof(struct codepage))
#define NCODEPAGES	(6)

extern unsigned char koi8_koi8[256];

extern unsigned char alt_koi8[256];
extern unsigned char koi8_alt[256];

extern unsigned char win_koi8[256];
extern unsigned char koi8_win[256];

extern unsigned char iso_koi8[256];
extern unsigned char koi8_iso[256];

extern unsigned char mac_koi8[256];
extern unsigned char koi8_mac[256];

extern unsigned char koi8_vola[256];

#endif
