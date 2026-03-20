#ifndef AUDIO_H__
#define AUDIO_H__

#ifndef __EMSCRIPTEN__
#include <fluidsynth.h>
typedef struct FluidSynth_s
{
	fluid_settings_t* settings;
	fluid_synth_t* synth;
	fluid_audio_driver_t* adriver;
} FluidSynth_t;

extern FluidSynth_t fluidSynth;
#endif

void DoomRPG_InitAudio(void);
void DoomRPG_CloseAudio(void);

#endif
