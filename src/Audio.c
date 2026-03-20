
#include <SDL.h>
#include <SDL_mixer.h>
#include <fluidsynth.h>
#include <stdio.h>

#include "DoomRPG.h"
#include "Audio.h"

FluidSynth_t fluidSynth;

void DoomRPG_InitAudio(void)
{
	printf("DoomRPG_InitAudio\n");

	fluidSynth.settings = NULL;
	fluidSynth.synth = NULL;
	fluidSynth.adriver = NULL;

	// create the settings
	fluidSynth.settings = new_fluid_settings();
	if (fluidSynth.settings == NULL) {
		DoomRPG_Error("Failed to create the settings");
	}

	// create the synthesizer
	fluidSynth.synth = new_fluid_synth(fluidSynth.settings);
	if (fluidSynth.synth == NULL) {
		DoomRPG_Error("Failed to create the synthesizer");
	}

	// create the audio driver
	fluidSynth.adriver = new_fluid_audio_driver(fluidSynth.settings, fluidSynth.synth);
	if (fluidSynth.synth == NULL) {
		DoomRPG_Error("Failed to create the audio driver");
	}

	if (fluid_is_soundfont("gm.sf2")) {
		fluid_synth_sfload(fluidSynth.synth, "gm.sf2", 1);
	}
	else {
		DoomRPG_Error("Cannot find the soundfont %s file", "gm.sf2");
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		DoomRPG_Error("Could not initialize SDL Mixer: %s", Mix_GetError());
	}
}

void DoomRPG_CloseAudio(void) {
	delete_fluid_audio_driver(fluidSynth.adriver);
	delete_fluid_synth(fluidSynth.synth);
	delete_fluid_settings(fluidSynth.settings);

	Mix_Quit();
}
