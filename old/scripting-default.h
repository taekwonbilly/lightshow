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




// this function gets called when the plugin first starts
void initialize_scripting() {
	/* this is an example of something you might want to do when initializing
	 * -- here we put light number 2 in SCRIPTED mode.  while in SCRIPTED mode
	 * -- the light won't do anything unless we explicitly tell it to. */
	//lights[2].current_mode = SCRIPTED;
	
}

// this function gets called regularly by the plugin
void trigger_timed_events(int current_track, int min, int sec, int msec) {
	/* this is an example of something you might want to trigger
	 * -- here we put light number 2 into AUTO mode only if we are playing
	 * -- track 3.  you can do absolutely anything you want here. */
	//if (current_track == 3) lights[2].current_mode = AUTO;
	//else lights[2].current_mode = SCRIPTED;
}
	
#endif
