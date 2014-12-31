#ifndef SOUND_H
#define SOUND_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
//#include <iostream.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <linux/soundcard.h>

#ifdef EXTERN
#undef EXTERN
#endif
#ifdef SOUND_C
#define EXTERN
#else
#define EXTERN extern
#endif

typedef struct Sound {
  int fd;
}* SoundPtr;

EXTERN inline void sound_init(SoundPtr this,
			      char *device,     // device filename
			      int fragments,    // 12 => 12 fragments
			      int pow_two_size, // 13 => size 8kb
			      int channels,     // 0=mono 1=stereo
			      int format,       // AFMT_U8
			      int rate,         // 22050
			      int perms) {      // O_WRONLY, O_RDONLY
  int setting = (fragments << 16) + pow_two_size;
  
  if ( (this->fd = open(device, perms)) == -1 ) {
    perror("open");
    fprintf(stderr, "Sound ERROR: opening %s\n", device);
    exit(-1);
  }
  if ( ioctl(this->fd, SNDCTL_DSP_SETFRAGMENT, &setting) == -1 ) {
    perror("ioctl set fragment");
    exit(-1);
  }
  if ( ioctl(this->fd, SNDCTL_DSP_STEREO, &channels) == -1 ) {
    perror("ioctl stereo");
    exit(-1);
  }
  if ( ioctl(this->fd, SNDCTL_DSP_SETFMT, &format) == -1 ) {
    perror("ioctl format");
    exit(-1);
  }
  if ( ioctl(this->fd, SNDCTL_DSP_SPEED, &rate) == -1 ) {
    perror("ioctl sample rate");
    exit(-1);
  }    
}

EXTERN inline void sound_destroy(SoundPtr this) {
  close(this->fd);
}

EXTERN inline void sound_sync(SoundPtr this) {
  if (ioctl(this->fd, SNDCTL_DSP_SYNC) == -1) {
    perror("ioctl sync");
    exit(-1);
  }
}

EXTERN inline void sound_write(SoundPtr this, void* buffer, unsigned long length) {
  write(this->fd, buffer, length);
}

EXTERN inline void sound_read(SoundPtr this, void* buffer, unsigned long length) {
  read(this->fd, buffer, length);
}

#endif // SOUND_H
