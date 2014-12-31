/*
 * plugin-main.c - Jesse Lacika (jesse@lacika.org)
 *
 * last modified: Mon Apr 19 2004
 *
 * The purpose of plugin-main.c is to provide the callback functions
 * that xmms needs in order to use a visualization plugin.  Do NOT
 * modify this file.  All scripted events should be set up in the
 * file scripting.h
 *
 */

#include <stdio.h>
#include <xmms/plugin.h>
#include <xmms/xmmsctrl.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include "hardware.h"
#include "scripting.h"

// the number of lights we can control (at present 4 boards with 20 lights each)
#define NUM_LIGHTS 80 
// the number of frequency bands we examine
#define NUM_BANDS 16
// the number of callbacks to wait before resetting trigger information
#define RESET_CALLBACKS 150

// possible modes for the lights
enum light_mode {
	AUTO,
	SCRIPTED
};

// each light is represented by one of these structs
typedef struct {
	// current mode of the the light (AUTO or SCRIPTED)
	int current_mode;
	
	// the current intensity value of the light (0=off 255=completely on)
	unsigned char current_intensity;

	// frequency band the light responds to (0=lowest freq 15=highest freq)
	unsigned char frequency_band;
	
	// threshold value to turn the light on 
	unsigned char threshold;

	// number of times it should fall below the threshold before turned off
	double turn_off_decays;
	
	// number of times it has fallen below the threhold since last trigger
	unsigned char num_decays;
	
	// how many times has the light been triggered since the last reset 
	unsigned char num_triggers;

	// the target number of times for the light to be triggered between resets
	double target_num_triggers;
} light_data;

/* --- BEGIN GLOBAL VARIABLES --- */
// the first four global variables might be useful for scripting

// all the data for the lights
light_data lights[NUM_LIGHTS];
// the FFT difference values
unsigned char differences[NUM_BANDS];
// the current FFT values
unsigned char curr_values[NUM_BANDS];
// the previous FFT values
unsigned char prev_values[NUM_BANDS];


gint16 freq_data[2][256];
int avg_values[NUM_BANDS];
int loop, render_freq_now;
/* --- END GLOBAL VARIABLES --- */

// forward declarations
static void lights_xmms_plugin_init();
static void lights_xmms_plugin_disable(); 
static void lights_xmms_plugin_configure();
static void lights_xmms_plugin_render_freq(gint16 freq_data[2][256]);
void load_config();
void* thread_func(void *params); 
static void actually_render_freq(); 
int freq_weighted_random_band(); 

// the VisPlugin struct sets up some information about the plugin and gives
// xmms the callback functions to call when things happen
VisPlugin lights_xmms_plugin_vtable = {
	NULL, // Handle, filled in by xmms
	NULL, // Filename, filled in by xmms

	0, // Session ID - commands in xmmsctrl.h use this
	"23Lights Plugin 2.0 - Spring 2004", // description

	0, // # of PCM channels for render_pcm()
	1, // # of freq channels wanted for render_freq()

	lights_xmms_plugin_init,           	// called when plugin is enabled
	lights_xmms_plugin_disable,	       	// called when plugin is disabled
	NULL,  			        	// called when 'about' is clicked
	lights_xmms_plugin_configure,		// called when 'configure' is clicked
	NULL,		 			// called to disable plugin, filled in by xmms
	NULL,				 	// called when playback starts
	NULL,				  	// called when playback stops
	NULL,                  			// called periodically with PCM data
	lights_xmms_plugin_render_freq     	// called periodically with freq data
};

// xmms calls this to grab on to all the info in the VisPlugin struct
VisPlugin *get_vplugin_info(void) {
	return &lights_xmms_plugin_vtable;
}

// called by xmms when the plugin is enabled
static void lights_xmms_plugin_init() {
	for (int i = 0; i < NUM_LIGHTS; i++) {
		// set default information for the lights
		lights[i].current_mode = AUTO;
		lights[i].current_intensity = 0;
		lights[i].frequency_band = (i%NUM_BANDS);
		lights[i].threshold = 30;
		lights[i].turn_off_decays = 18;
		lights[i].num_decays = 0;
		lights[i].num_triggers = 0;
		lights[i].target_num_triggers = 3;
		update_light(i, 0);
	}

	// initialize our frequency data buffers
	memset(differences, 0, NUM_BANDS);
	memset(curr_values, 0, NUM_BANDS);
	memset(prev_values, 0, NUM_BANDS);
	memset(avg_values, 0, NUM_BANDS*sizeof(int));
	
	// load the config file
	load_config();

	// let scripting.h update the initial state of things
	initialize_scripting();
	
	// seed the random number generator
	srand(time(0)); 

	// initialize thread control variables
	loop = 1;
	render_freq_now = 0;

	// spawn a thread to actually do the work
	pthread_t *handle;
	handle = malloc(sizeof(pthread_t));
	pthread_create(handle, NULL, thread_func, NULL);
}

// called by xxms when the plugin is disabled
static void lights_xmms_plugin_disable() {
	// signal the processing thread to terminate
	loop = 0;
}

// called by xmms when the configure button is clicked
static void lights_xmms_plugin_configure() {
	load_config();
}

// called by xmms periodically during playback - passed_freq_data gives us a 256 point
// FFT of the music that xmms is currently playing
static void lights_xmms_plugin_render_freq(gint16 passed_freq_data[2][256]) {
	// when the thread isn't busy copy the data over and signal the thread to process it 
	if (render_freq_now == 0) {
		memcpy(freq_data, passed_freq_data, sizeof(gint16)*512);
		render_freq_now = 1;
	}
}

// the processing thread runs in this function
void* thread_func(void *params) {
	// keep processing until signaled to stop
	while (loop == 1) {
		// actually process frequency data when signaled to do so
		if (render_freq_now == 1) {
			actually_render_freq();
			render_freq_now = 0;
		}
	}

	return NULL;
}

// called from the thread function - actually processes the 256 point
// FFT of the music that xmms is currently playing
static void actually_render_freq() {

	/* The following section of code (roughly) converts the 256 point FFT
	 * data into 16 frequency bands that are the same width in terms of musical
	 * significance.  It was adapted from the old 23 lights plugin and it looks
	 * like that was stolen directly from the spectrum analyzer code distributed
	 * with the xmms source. */
	int y, c;
	gint xscale[] = {0, 1, 2, 3, 5, 7, 10, 14, 20, 28, 40, 54, 
		74, 101, 137, 187, 255};

	// a running average of the difference values - gives some measure of how
	// spastic a song is
	static int running_avg_difference = 0;
	// the partial difference sum before it is added into the running average
	int difference_sum = 0;

	for (int i = 0; i < NUM_BANDS; i++) {
		for (c = xscale[i], y = 0; c < xscale[i + 1]; c++) {
			if (freq_data[0][c] > y)
				y = freq_data[0][c];
	    	}

		y >>= 7;
		if (y != 0) {
			y = (gint)(log(y) * (256 / log(256)));
			if (y > 255)
				y = 255;
		}
		
		// put the frequency value (y) into our buffer 
		curr_values[i] = (unsigned char) y;
		// calculate the difference (beats tend to correspond to the difference)
		differences[i] = abs(prev_values[i]-curr_values[i]);
		// save the value in order to calculate the difference next time
		prev_values[i] = curr_values[i];

		// add the difference value to the partial difference sum
		difference_sum += differences[i];

		avg_values[i] += (1.0/RESET_CALLBACKS) * y;
	}
	/* END frequency band conversion code */
	
	// incorporate the partial difference sum into the running average
	running_avg_difference = (0.99 * running_avg_difference) + (0.01 * difference_sum);

	// keep track of the number of times this function has been called
	static int callback_count = 0;
	callback_count = (callback_count + 1) % RESET_CALLBACKS;

	// go through all the lights
	for (int i = 0; i < NUM_LIGHTS; i++) {

		// check if the light should be controlled automaticallay
		if (lights[i].current_mode == AUTO) {
		
			// if the light is off and its threhold has been exceeded turn it on
			if ((differences[lights[i].frequency_band] > lights[i].threshold) &&
			    (lights[i].current_intensity == 0)) {
				lights[i].current_intensity = 255;
				update_light(i, 255);
				lights[i].num_triggers++;
				lights[i].num_decays = 0;
			}

			// check if the light is on and the music is below its threshold
			if ((differences[lights[i].frequency_band] < lights[i].threshold) &&
			    (lights[i].current_intensity > 0)) { 
				// if the light has been below the threshold enough times turn it off
				if (lights[i].num_decays < 
					(lights[i].turn_off_decays * ((400.0 - running_avg_difference) / 300.0)))
					lights[i].num_decays++;
				else {
					lights[i].num_decays = 0;
					lights[i].current_intensity = 0;
					update_light(i, 0);
				}
			}
		
			// adjust thresholds when the callback function has been called enough times 
			if (callback_count == (RESET_CALLBACKS-1)) {
				// lower the threshold if the light wasn't active enough
				if (lights[i].num_triggers < 
					(lights[i].target_num_triggers * (running_avg_difference / 300.0))) {
					if (lights[i].threshold > 5)
						lights[i].threshold -= 5;
					else lights[i].threshold = 1;
				} 
				// raise the threshold if the light was too active (or active enough)
				else {
					if (lights[i].threshold < 251)
						lights[i].threshold += 5;
					else lights[i].threshold = 255;
				}
			
				// reset the counter of the light activity
				lights[i].num_triggers = 0;
			}
		}
	}

	// reassign a light to a different frequency band every so often
	if (callback_count == (RESET_CALLBACKS-1)) {
		// pick a light to reassign at random
		int reassign_light = (((double)rand()/(double)RAND_MAX) * NUM_LIGHTS);
		// pick a frequency band semi-randomly, weighted by actual freq content of the song
		int reassign_freq_band = freq_weighted_random_band();
	
		// reassign the light and reset the frequency band data
		lights[reassign_light].frequency_band = reassign_freq_band;
		memset(avg_values, 0, sizeof(int)*NUM_BANDS);
	}

	// get the track number and current time from xmms
	gint tracknum = xmms_remote_get_playlist_pos(0);
	gint time_msec = xmms_remote_get_output_time(0);

	// convert the current time to minutes/seconds/msecs
	int min = (time_msec/60000.0);
	int sec = (((int)(time_msec/1000.0)) % 60);
	int msec = (time_msec % 1000);

	// trigger events that are set up in scripting.h
	trigger_timed_events(tracknum, min, sec, msec);	
}

// picks a semi-random frequency band with a greater probability of picking a band
// that has recently been active
int freq_weighted_random_band() {
	// sort the frequency bands from most active to least active
	int sorted_bands[NUM_BANDS];
	for (int i = 0; i < NUM_BANDS; i++) {
		int max_loc = 0;
		for (int j = 0; j < NUM_BANDS; j++) 
			if (avg_values[j] >= avg_values[max_loc]) max_loc = j;
		avg_values[max_loc] = -1;
		sorted_bands[i] = max_loc;
	}

	// randomly pick a band with greater probability of picking an active band
	double x = ((double)rand() / (double)RAND_MAX) * 256.0;
	int y = pow(2, (x/64.0)) - 1;
	int band_to_pick = sorted_bands[y];

	return band_to_pick;
}

// loads the config file
void load_config() {
	FILE *config_file;
	int light_num, frequency_band, turn_off_decays, target_num_triggers;
	char *home;
	char filename[100]; 

	// create the config file name by appending to the users home directory
	home = getenv("HOME");
	strcpy(filename, home);
	strcat(filename, "/.xmms/Plugins/23lights/23lights.conf");
	
	// try to open the config file, if it isn't there then just use defaults
	config_file = fopen(filename, "r");
	if (config_file == NULL) {
		printf("Unable to open %s\n", filename);
		return;
	}
	
	// read lines from the config file each line should look like this:
	// 	light_num freq_band turn_off_decays target_num_triggers
	// 	ex: 3 15 5 3
	// ***MALFORMED CONFIG FILES WILL CAUSE SEGFAULTS***
	while (fscanf(config_file, "%d %d %d %d", &light_num, &frequency_band,
				&turn_off_decays, &target_num_triggers) != EOF) {
		if ((light_num < NUM_LIGHTS) && (frequency_band < 16)) {
			lights[light_num].frequency_band = frequency_band;
			lights[light_num].turn_off_decays = turn_off_decays;
			lights[light_num].target_num_triggers = target_num_triggers;
		}
	}
}
