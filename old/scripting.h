/* 
 * scripting.h - Jesse Lacika (jesse@lacika.org)
 *
 * last modified: Mon Apr 19 2004
 *
 * This file should be modified every time you want to make a light show
 * that is carefully choreographed to particular songs.  
 * 
 */

#ifndef __SCRIPTING_H__
#define __SCRIPTING_H__

#define NUM_LOFT  5
#define NUM_CENTER  5
#define NUM_SOFA  3



/* --- BEGIN REPRODUCED CODE --- 

	READ THIS: The code in this block comment is defined exactly
	as you see it elsewhere.  DO NOT uncomment it here.  It is reproduced
	here as a reference.  A few additional comments are added here to 
	help with scripting for specific songs (marked with ***).  If you 
	are already familiar with the code here you probably want to skip 
	down below.
   
// possible modes for the lights
enum light_mode {
	AUTO,
	SCRIPTED
};

// each light is represented by one of these structs
typedef struct {
	// current mode of the the light (AUTO or SCRIPTED)
	// *** scripts should change this when they want to control lights
	// *** or let the lights be controlled automatically
	int current_mode;
	
	// the current intensity value of the light (0=off 255=completely on)
	unsigned char current_intensity;

	// frequency band the light responds to (0=lowest freq 15=highest freq)
	// *** if the light is in AUTO mode this will change automatically
	// *** so don't rely on it unless you explicity set it for any time
	// *** that you care about
	unsigned char frequency_band;
	
	// threshold value to turn the light on 
	// *** if the light is in AUTO mode this will change automatically
	// *** so don't rely on it unless you explicity set it for any time
	// *** that you care about
	unsigned char threshold;

	// number of times it should fall below the threshold before turned off
	// *** this will not change automatically, you MUST explicitly change it
	// *** when you care for it to change
	double turn_off_decays;
	
	// number of times it has fallen below the threhold since last trigger
	unsigned char num_decays;
	
	// how many times has the light been triggered since the last reset 
	unsigned char num_triggers;

	// the target number of times for the light to be triggered between resets
	// *** this will not change automatically, you MUST explicitly change it
	// *** when you care for it to change
	double target_num_triggers;
} light_data;

// all the data for the lights
light_data lights[NUM_LIGHTS];
// the FFT difference values
unsigned char differences[NUM_BANDS];
// the current FFT values
unsigned char curr_values[NUM_BANDS];
// the previous FFT values
unsigned char prev_values[NUM_BANDS];

 *
 * update_light - updates the state of a single light in 23
 *
 * @params: 	lightnum:	which light to update (currently 0-79 are valid)
 * 		intensity: 	the intensity value for lightnum (0=off 255=completely on)
 *
 * @returns:	0  upon a successful update
 * 		-1 if the update fails
 *
int update_light(Byte lightnum, Byte intensity) 


--- END REPRODUCED CODE --- */

void update_light_batch(unsigned char lightnum, unsigned char intensity) {
	update_light(lightnum, intensity);
	usleep(100);
}

int group_loft[] = {20, 25, 26, 27, 31};
int group_center[] = {0, 5, 21, 28, 30};
int group_sofa[] = {1, 4, 7};

#include "laser.h"

laser *l;
// this function gets called when the plugin first starts
void initialize_scripting() {
	/* this is an example of something you might want to do when initializing
	 * -- here we put light number 2 in SCRIPTED mode.  while in SCRIPTED mode
	 * -- the light won't do anything unless we explicitly tell it to. */
	//lights[2].current_mode = SCRIPTED;
	l =  malloc(sizeof(laser));
	init_laser(l,"/dev/dsp1");
	l->mode = LASER_STAR;
}


// this function returns RGB values for a particular position
// on the color wheel
void color_wheel(int hue, unsigned char *red, unsigned char *green,
		 unsigned char *blue) {

  hue %= 1536;
  // choose colors based on position around wheel
  if(hue <= 255) {
    *red = 255;           // red on
    *green = hue;         // green up
    *blue = 0;            // blue off
  }
  if((hue >= 256) && (hue <= 511)) {
    *green = 255;         // green on
    *red = 511 - hue;     // red down
    *blue = 0;            // blue off
  }
  if((hue >= 512) && (hue <= 767)) {
    *green = 255;         // green on
    *red = 0;             // red off
    *blue = hue - 512;    // blue up
  }
  if((hue >= 768) && (hue <= 1023)) {
    *green = 1023 - hue;  // green down
    *red = 0;             // red off
    *blue = 255;          // blue on
  }
  if((hue >= 1024) && (hue <= 1279)) {
    *green = 0;           // green off
    *red = hue - 1024;    // red up
    *blue = 255;          // blue on
  }
  if((hue >= 1280) && (hue <= 1536)) {
    *green = 0;           // green off
    *red = 255;           // red on
    *blue = 1536 - hue;   // blue down
  }
  
}

// this function wraps color_wheel()
void color_wheel_time(long ticks, int period, unsigned char *red,
		      unsigned char *green, unsigned char *blue) {
  int hue = 0;

  hue = (int)((ticks % period) * (float)(1536.0 / period));
  color_wheel(hue, red, green, blue);
}


// this function turns a luxeon's logical address into a physical one
int map_luxeon(int cluster, int color) {
  return 60 + (cluster * 3) + color;
}

// this function gets called regularly by the plugin
void trigger_timed_events(int current_track, int min, int sec, int msec) {
	/* this is an example of something you might want to trigger
	 * -- here we put light number 2 into AUTO mode only if we are playing
	 * -- track 3.  you can do absolutely anything you want here. */
	//if (current_track == 3) lights[2].current_mode = AUTO;
	//else lights[2].current_mode = SCRIPTED;
  int i = 0;
  unsigned char red, green, blue;
  printf("%d:%d:%d\n",min,sec,msec);

  //cycle the modes uncomment below
  l->mode = (sec/8)%(LASER_ASTEROID+1);

  switch (current_track+1) {
  case 1:    //Dark Side of the Moon, track 1
     if((min == 0) && (sec == 0) && (msec < 100)) {
       for(i = 0; i < NUM_LIGHTS; i++) {
	 lights[i].current_mode = SCRIPTED;
	 update_light_batch(i,0);
       }
       lights[25].current_mode = AUTO;
       lights[25].current_intensity = 255;
       lights[25].frequency_band = 0;
       lights[25].threshold = 20;
     }

     if((min == 0) && (sec == 30) && (msec < 100)) {
       for(i = 0 ; i < NUM_LIGHTS, i++){
	 lights[i].current_mode = AUTO;
       }
     }
  }
}

     /*     
     if((min == 0) && (sec == 18) && (msec < 100)) {
       for(i = 0; i < NUM_LOFT; i++ ) {    // loft lights on auto
	 lights[group_loft[i]].current_mode = AUTO;
       }
       for(i = 0; i < NUM_SOFA; i++) {    // sofa lights off
	 lights[group_sofa[i]].current_mode = SCRIPTED;
	 update_light_batch(group_sofa[i],0);
       }
       for(i = 0; i < NUM_CENTER; i++) {  // center lights off
	 lights[group_center[i]].current_mode = SCRIPTED;
	 update_light_batch(group_center[i],0);
       }
       for(i = 60; i < NUM_LIGHTS; i++) {  // luxeons off
	 lights[i].current_mode = SCRIPTED;
	 update_light_batch(i,0);
       }
     }
     if((min == 0) && (sec == 33) && (msec < 100)) {
       for(i = 0; i < NUM_SOFA; i++) {
	 lights[group_sofa[i]].current_mode = AUTO;
       }
       for(i = 0; i < NUM_CENTER; i++) {  // center lights off
	 lights[group_center[i]].current_mode = SCRIPTED;
	 update_light_batch(group_center[i],0);
       }
       for(i = 60; i < NUM_LIGHTS; i++) {  // luxeons off
	 lights[i].current_mode = SCRIPTED;
	 update_light_batch(i,0);
       }
     }
    break;
  case 2:    // George Harrison - While My Guitar Gently Weeps
    if((min == 0) && (sec == 0)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = SCRIPTED;
	update_light_batch(group_center[i],0);
      }
    }
    if((sec + 60*min) < 34) {
      if(msec < 50) {
	for(i = 60; i < NUM_LIGHTS; i++) {   // color-wheel luxeons
	  lights[i].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_CENTER; i++) {     // freeze the center
	  lights[group_center[i]].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_SOFA; i++) {      // freeze the sofa
	  lights[group_sofa[i]].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_LOFT; i++) {     // freeze the loft
	  lights[group_loft[i]].current_mode = SCRIPTED;
	}
      }
      color_wheel_time(msec + 1000 * sec + 60000 * min, 4000,
		       &red,&green,&blue);
      for(i = 0; i < 6; i++) {
	update_light(map_luxeon(i,0),red);
	update_light(map_luxeon(i,1),green);
	update_light(map_luxeon(i,2),blue);
      }
    }
    if((sec + 60*min) >= 34) {
      if(msec < 50) {
	for(i = 60; i < NUM_LIGHTS; i++) {   // color-wheel luxeons
	  lights[i].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_CENTER; i++) {     
	  lights[group_center[i]].current_mode = AUTO;
	}
	for(i = 0; i < NUM_SOFA; i++) {      
	  lights[group_sofa[i]].current_mode = AUTO;
	}
	for(i = 0; i < NUM_LOFT; i++) {     
	  lights[group_loft[i]].current_mode = AUTO;
	}
      }
      color_wheel_time(msec + 1000 * sec + 60000 * min, 4000,
		       &red,&green,&blue);
      for(i = 0; i < 6; i++) {
	update_light(map_luxeon(i,0),red);
	update_light(map_luxeon(i,1),green);
	update_light(map_luxeon(i,2),blue);
      }

    }
    break;
  case 3:    // Wendy Carlos - Country Lane (ogg)
    if((min == 0) && (sec == 0) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = SCRIPTED;
	update_light_batch(group_loft[i],0);
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = SCRIPTED;
	update_light_batch(group_sofa[i],0);
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = SCRIPTED;
	update_light_batch(i,0);
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = SCRIPTED;
      update_light_batch(21,0);
    }
    if((min == 0) && (sec == 18) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = SCRIPTED;
	update_light_batch(group_sofa[i],0);
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = SCRIPTED;
	update_light_batch(i,0);
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = SCRIPTED;
      update_light_batch(21,0);
    }
    if((min == 0) && (sec == 18) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = SCRIPTED;
	update_light_batch(group_sofa[i],0);
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = AUTO;
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = SCRIPTED;
      update_light_batch(21,0);

    }
    if((min == 0) && (sec == 50) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = SCRIPTED;
	update_light_batch(group_center[i],0);
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = SCRIPTED;
	update_light_batch(group_sofa[i],0);
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = AUTO;
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = AUTO;
    }
    if((min == 1) && (sec == 50) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = SCRIPTED;
	update_light_batch(group_center[i],0);
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = AUTO;
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = AUTO;
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = SCRIPTED;
      update_light_batch(21,0);
    }
    if((min == 2) && (sec == 24) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = SCRIPTED;
	update_light_batch(group_center[i],0);
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = SCRIPTED;
	update_light_batch(group_loft[i],0);
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = SCRIPTED;
	update_light_batch(group_sofa[i],0);
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = AUTO;
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = SCRIPTED;
      update_light_batch(21,0);
    }
    if((min == 2) && (sec == 42) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = SCRIPTED;
	update_light_batch(group_sofa[i],0);
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = SCRIPTED;
	update_light_batch(i,0);
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = SCRIPTED;
      update_light_batch(21,0);
    }
    if((min == 3) && (sec == 16) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = SCRIPTED;
	update_light_batch(group_sofa[i],0);
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = SCRIPTED;
	update_light_batch(i,0);
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = AUTO;
    }

    if((min == 3) && (sec == 40) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = SCRIPTED;
	update_light_batch(group_center[i],0);
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = SCRIPTED;
	update_light_batch(group_loft[i],0);
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = AUTO;
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = SCRIPTED;
	update_light_batch(i,0);
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = SCRIPTED;
      update_light_batch(21,0);
    }
    if((min == 4) && (sec == 11) & (msec < 100)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = SCRIPTED;
	update_light_batch(group_center[i],0);
      }
      for(i = 0; i < NUM_LOFT; i++) {
	lights[group_loft[i]].current_mode = AUTO;
      }
      for(i = 0; i < NUM_SOFA; i++) {
	lights[group_sofa[i]].current_mode = AUTO;
      }
      for(i = 60; i < NUM_LIGHTS; i++) {
	lights[i].current_mode = SCRIPTED;
	update_light_batch(i,0);
      }
      lights[2].current_mode = SCRIPTED;         // lord off
      update_light_batch(2,0);
      lights[21].current_mode = AUTO;
    }

    break;

  case 4:    // Jefferson Airplane - White Rabbit (mp3)
    if((min == 0) && (sec == 0)) {
      for(i = 0; i < NUM_CENTER; i++) {
	lights[group_center[i]].current_mode = SCRIPTED;
	update_light_batch(group_center[i],0);
      }
    }
    if((sec + 60*min) < 28) {
      if(msec < 50) {
	for(i = 60; i < NUM_LIGHTS; i++) {   // freeze the luxeons
	  lights[i].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_CENTER; i++) {     // freeze the center
	  lights[group_center[i]].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_SOFA; i++) {      // activate the sofa
	  lights[group_sofa[i]].current_mode = AUTO;
	}
	for(i = 0; i < NUM_LOFT; i++) {     // freeze the loft
	  lights[group_loft[i]].current_mode = SCRIPTED;
	}
      }
    }
    if((sec + 60*min) < 55 && (sec + 60*min) >= 28) {
      if(msec < 50) {
	for(i = 60; i < 69; i++) {    // freeze half the luxeons
	  lights[i].current_mode = AUTO;
	}
	for(i = 69; i < NUM_LIGHTS; i++) {
	  lights[i].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_CENTER; i++) {     // freeze the center
	  lights[group_center[i]].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_SOFA; i++) {      // activate the sofa
	  lights[group_sofa[i]].current_mode = AUTO;
	}
	for(i = 0; i < NUM_LOFT; i++) {     // freeze the loft
	  lights[group_loft[i]].current_mode = SCRIPTED;
	}
      }
    }
    if((sec + 60*min) >= 55 && (sec + 60*min) < 84) {
      if(msec < 50) {
	for(i = 60; i < NUM_LIGHTS; i++) {    // activate the luxeons
	  lights[i].current_mode = AUTO;
	}
	for(i = 0; i < NUM_CENTER; i++) {     // freeze the center
	  lights[group_center[i]].current_mode = SCRIPTED;
	}
	for(i = 0; i < NUM_SOFA; i++) {      // activate the sofa
	  lights[group_sofa[i]].current_mode = AUTO;
	}
	for(i = 0; i < NUM_LOFT; i++) {     // activate the loft
	  lights[group_loft[i]].current_mode = AUTO;
	}
      }
    }
    if((sec + 60*min) > 84) {
      if(msec < 50) {
	for(i = 60; i < NUM_LIGHTS; i++) {    // activate the luxeons
	  lights[i].current_mode = AUTO;
	}
	for(i = 0; i < NUM_CENTER; i++) {     // activate the center
	  lights[group_center[i]].current_mode = AUTO;
	}
	for(i = 0; i < NUM_SOFA; i++) {      // activate the sofa
	  lights[group_sofa[i]].current_mode = AUTO;
	}
	for(i = 0; i < NUM_LOFT; i++) {     // activate the loft
	  lights[group_loft[i]].current_mode = AUTO;
	}
      }
    }

    if((sec + 60*min) < 137) {   // the LORD is almost always dead
      lights[2].current_mode = SCRIPTED;
      update_light_batch(2,0);
    } else {
      lights[2].current_mode = AUTO;
    }
    break;
    
  }
}
*\	
#endif

