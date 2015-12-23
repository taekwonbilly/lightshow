/*
 * hardware.c - Jesse Lacika (jesse@lacika.org)
 *
 * last modified: Thu Apr 15 2004
 *
 * The purpose of hardware.c is to provide the hardware level interaction
 * with the ethernet controller for the lights in 23.  The code is based
 * on 23.c written by Kevin McCormick in 2002.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <math.h>
#include "hardware.h"

#define KINET_UDP_PORT 6038
#define KINET_VERSION 0x0001
#define KINET_MAGIC 0x4adc0104
#define KTYPE_DMXOUT 0x0101

// type aliases
#ifndef Byte
#define Byte unsigned char
#endif
#ifndef Word
#define Word unsigned short
#endif
#ifndef DWord
#define DWord unsigned long
#endif

typedef struct {
	DWord magic;
	Word ver;
	Word type;
	DWord seq;
} KiNET_Hdr;

typedef struct {
	KiNET_Hdr hdr;
	Byte port;
	Byte flags;
	Word timerVal;
	DWord uni;
} KiNET_DMXout;

/*
 * update_light - updates the state of a single light in 23
 *
 * @params: 	lightnum:	which light to update (currently 0-79 are valid)
 * 		intensity: 	the intensity value for lightnum (0=off 255=completely on)
 *
 * @returns:	0  upon a successful update
 * 		-1 if the update fails
 */
int update_light(Byte lightnum, Byte intensity) {

	/*** --- BEGIN SECTION: prepare a buffer of data --- ***/
	Byte data[26];
	// put header info at the beginning of the buffer
	KiNET_DMXout *kdmxout = (KiNET_DMXout*) data;
	kdmxout->hdr.magic=KINET_MAGIC;
	kdmxout->hdr.ver=KINET_VERSION;
	kdmxout->hdr.type=KTYPE_DMXOUT;
	kdmxout->hdr.seq=0;
	kdmxout->port=0;
	kdmxout->timerVal=0;
	kdmxout->uni=-1;

	// move the buffer position to the end of the header
	int dlen = sizeof(KiNET_DMXout);	
	
	// determine the board number from the light number
	Byte boardnum = ((lightnum / 20) + 0x88);
	if (boardnum > 0x8b) 
		return -1;
	
	// add an escape character and the board number to the buffer
	data[dlen++]=0xf0;
	data[dlen++]=boardnum;

	// add an escape character and the light number to the buffer
	data[dlen++]=0xf0;
	data[dlen++]=0x40+(lightnum%20);

	// add the intensity to the buffer (twice if it is the escape character)
	if (intensity == 0xf0) data[dlen++]=0xf0;
	data[dlen++]=intensity;
	/*** --- END SECTION --- ***/
	
	/*** --- BEGIN SECTION: send data to the light controller --- ***/
	int sock;
	struct sockaddr_in destsa;

	if ((sock=socket(PF_INET,SOCK_DGRAM,0))==-1) 
		return -1;

	destsa.sin_family=AF_INET;
	destsa.sin_addr.s_addr=inet_addr("18.224.1.147");
	destsa.sin_port=htons(KINET_UDP_PORT);

	if (sendto(sock,data,dlen,0,(struct sockaddr *)&destsa,sizeof(destsa))==-1) 
		return -1;
	close(sock);
	/*** --- END SECTION --- ***/

	return 0;
}
