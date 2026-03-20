
#include <SDL.h>
#include <stdio.h>

#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Sound.h"
#include "Menu.h"
#include "MenuSystem.h"

// ============================================================================
// Emscripten stub implementation - audio disabled for web builds
// ============================================================================

Sound_t* Sound_init(Sound_t* sound, DoomRPG_t* doomRpg)
{
	int i;
	SoundChannel_t* chan;

	printf("Sound_init\n");

	if (sound == NULL) {
		sound = SDL_malloc(sizeof(Sound_t));
		if (sound == NULL) {
			return NULL;
		}
	}
	SDL_memset(sound, 0, sizeof(Sound_t));

	sound->soundEnabled = 0;
	sound->priority = 3;
	sound->channel = 0;
	sound->volume = 100;

	for (i = 0; i < (MAX_SOUNDCHANNELS + 1); i++) {
		chan = &sound->soundChannel[i];
		chan->mediaAudioSound = NULL;
		chan->mediaAudioMusic = NULL;
		chan->size = 0;
	}

	sound->doomRpg = doomRpg;
	printf("Audio disabled for web build\n");

	return sound;
}

void Sound_free(Sound_t* sound, boolean freePtr)
{
	if (freePtr) {
		SDL_free(sound);
	}
}

void Sound_stopSounds(Sound_t* sound)
{
	sound->priority = 0;
}

void Sound_freeSound(Sound_t* sound, int chan)
{
	SoundChannel_t* sChannel = &sound->soundChannel[chan];
	sChannel->flags = 0;
	sChannel->mediaAudioSound = NULL;
	sChannel->mediaAudioMusic = NULL;
}

int Sound_getState(Sound_t* sound, int resourceID)
{
	(void)resourceID;
	return sound->nextplay;
}

int Sound_getFreeChanel(Sound_t* sound)
{
	(void)sound;
	return -1;
}

void Sound_loadSound(Sound_t* sound, int chan, short resourceID)
{
	(void)sound;
	(void)chan;
	(void)resourceID;
}

void Sound_readySound(Sound_t* sound, int chan)
{
	(void)sound;
	(void)chan;
}

void Sound_playSound(Sound_t* sound, int resourceID, byte flags, int priority)
{
	(void)sound;
	(void)resourceID;
	(void)flags;
	(void)priority;
}

void Sound_freeSounds(Sound_t* sound)
{
	int chan = 0;
	do {
		Sound_freeSound(sound, chan);
	} while (++chan < (MAX_SOUNDCHANNELS + 1));
}

int Sound_getFromResourceID(int resourceID)
{
	(void)resourceID;
	return -1;
}

void Sound_updateVolume(Sound_t* sound)
{
	int menu = sound->doomRpg->menuSystem->menu;
	if (menu == MENU_SOUND || menu == MENU_INGAME_SOUND) {
		Menu_textVolume(sound->doomRpg->menu, sound->volume);
	}
}

int Sound_minusVolume(Sound_t* sound, int volume)
{
	sound->volume -= volume;
	if (sound->volume < 0) {
		sound->volume = 0;
	}
	Sound_updateVolume(sound);
	return sound->volume;
}

int Sound_addVolume(Sound_t* sound, int volume)
{
	sound->volume += volume;
	if (sound->volume > 100) {
		sound->volume = 100;
	}
	Sound_updateVolume(sound);
	return sound->volume;
}
