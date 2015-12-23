#ifndef LASERCTL_H
#define LASERCTL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "sound.h"
#include "thread.h"
#include "lasercirclemode.h"

#ifdef EXTERN
#  undef EXTERN
#endif
#ifdef LASERCTL_C
#  define EXTERN
#else
#  define EXTERN extern
#endif

typedef unsigned char byte;

EXTERN inline void* smalloc(int size) {
  void* ptr = malloc(size);
  if(ptr == NULL) {
    fprintf(stderr, "smalloc error: not enough memory to allocate %d bytes.\n", size);
    exit(-1);
  }
  return ptr;
}

enum LaserCtlMode {
  LCM_OFF,
  LCM_CIRCLES,
};

typedef struct LaserCtl {
  char*    filename;
  SoundPtr sound;
  int      current_pattern;
  int      sample_num;
  int      buffer_size; // sample_num * 2
  byte*    buffer;
  double   the_time; // thread safe mutable
  int      mode;     // thread safe mutable
  int      volume;   // thread safe mutable
  int      exit;     // thread safe mutable (tells child to exit)
  int      exited;   // signals parent of successful exit
}* LaserCtlPtr;

EXTERN inline void laserctl_init(LaserCtlPtr this, char* filename, int sample_num) {
  this->filename = smalloc(strlen(filename) + 1);
  strcpy(this->filename, filename);
  fprintf(stderr, "laserctl_init: opening %s\n", filename);
  this->sound = (SoundPtr)smalloc(sizeof(struct Sound));
  sound_init(this->sound, filename, 12, 13, 1, AFMT_U8, 22050, O_WRONLY);
  this->sample_num = sample_num;
  this->buffer_size = sample_num * 2;
  this->buffer = (byte*)smalloc(sizeof(byte) * this->buffer_size);
  this->the_time = 0;
  this->mode = LCM_OFF;
  this->volume = 0;
  this->exit = 0;
  this->exited = 0;
}

EXTERN inline void laserctl_destroy(LaserCtlPtr this) {
  fprintf(stderr, "laserctl_destroy: closing %s\n", this->filename);
  sound_destroy(this->sound);
  free(this->sound);
  free(this->filename);
}

EXTERN inline void laserctl_run(LaserCtlPtr this) {
  int   i;
  byte* p;
  double t;
  double vol;
  LaserCircleModePtr circle_mode = smalloc(sizeof(struct LaserCircleMode));
  lasercirclemode_init(circle_mode);
  while(this->exit == 0) { // there should be a way to exit this loop
    //fprintf(stderr, "running.\n");
    t = this->the_time;
    vol = this->volume * 127.0;
    switch(this->mode) {
    case LCM_OFF:
      p = this->buffer;
      for(i = 0; i < this->sample_num; i++) {
	p[0] = 0x7f;
	p[1] = 0x7f;
	p += 2;
	t += 1 / 22050.0;
      }
      break;
    case LCM_CIRCLES:
      
      p = this->buffer;
      for(i = 0; i < this->sample_num; i++) {
	p[0] = 0x7f + (int)(vol * sin(502 * t));
	p[1] = 0x7f + (int)(vol * cos(502 * t));
	p += 2;
	t += 1 / 22050.0;
      }
      break;
    }
    this->the_time = t;
    sound_write(this->sound, this->buffer, this->buffer_size);
  }
  lasercirclemode_destroy(circle_mode);
  free(circle_mode);
  this->exited = -1;
}

#ifdef LASERCTL_C
void* laserctl_thread_start_run(void* ptr) {
  laserctl_run(ptr);
  thread_exit();
  return NULL;
}
#endif

EXTERN inline void laserctl_run_in_new_thread(LaserCtlPtr this) {
  ThreadPtr thread = smalloc(sizeof(struct Thread));
  thread_init(thread, laserctl_thread_start_run, this);
  thread_run(thread);
}

EXTERN inline void laserctl_exit(LaserCtlPtr this) {
  this->exit = -1;
}

EXTERN inline void laserctl_stop_thread(LaserCtlPtr this) {
  this->exit = -1;
  thread_wait_int(&(this->exited), 1);
  int i;
  for(i = 0; i < 4 && this->exited == 0; i++) {
    fprintf(stderr, "waiting for child thread to exit.\n");
    thread_sleep(1);
  }
  if(this->exited == 0) {
    fprintf(stderr, "forcing child thread to exit.\n");
  }
}

EXTERN inline void wait_for_enter() {
  char buf[1];
  read(STDIN_FILENO, buf, 1);
}

#ifdef LASERCTL_C
int main(int argc, char** argv) {
  if(argc != 2) {
    printf("\n"
	   "usage: laserctl <sound device>\n"
	   "\n");
    exit(-1);
  }
  LaserCtlPtr laser_ctl = smalloc(sizeof(struct LaserCtl));
  laserctl_init(laser_ctl, argv[1], 128);
  laserctl_run_in_new_thread(laser_ctl);

  fprintf(stderr, "default is silence, press enter to play circles.\n");
  wait_for_enter();
  
  laser_ctl->mode = LCM_CIRCLES;
  laser_ctl->volume = 1;
  fprintf(stderr, "circles...\n");
  wait_for_enter();
  
  laserctl_stop_thread(laser_ctl);
  laserctl_destroy(laser_ctl);
  return 0;
}
#endif

#endif // LASERCTL_H
