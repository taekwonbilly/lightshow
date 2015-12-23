#ifndef __HARDWARE_H__
#define __HARDWARE_H__

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

int update_light(Byte lightnum, Byte intensity); 

#endif
