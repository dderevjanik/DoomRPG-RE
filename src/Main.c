//Using SDL and standard IO
#include <SDL.h>
#ifndef __EMSCRIPTEN__
#include <SDL_mixer.h>
#endif
#include <stdio.h>
#include <zlib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "Z_Zone.h"
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Player.h"
#include "Hud.h"
#include "MenuSystem.h"
#include "SDL_Video.h"
#include "Z_Zip.h"

extern DoomRPG_t* doomRpg;

// Global state for main loop (needed for Emscripten callback)
static int g_UpTime = 0;
static int g_mouseTime = 0;
static int g_key = 0;
static int g_oldKey = -1;
static const Uint8* g_keyboardState = NULL;

// Main loop iteration - called repeatedly
// For native: called in while loop
// For Emscripten: called via emscripten_set_main_loop
static void mainLoopIteration(void)
{
	SDL_Event ev;
	int mouse_Button;
	int currentTimeMillis = DoomRPG_GetUpTimeMS();

	mouse_Button = MOUSE_BUTTON_INVALID;

	while (SDL_PollEvent(&ev))
	{
		// check event type
		switch (ev.type) {

			// Mouse Event
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				Uint32 buttons = SDL_GetMouseState(NULL, NULL);

				if ((buttons & SDL_BUTTON_LMASK) != 0) {
					mouse_Button = MOUSE_BUTTON_LEFT;
				}
				else if ((buttons & SDL_BUTTON_MMASK) != 0) {
					mouse_Button = MOUSE_BUTTON_MIDDLE;
				}
				else if ((buttons & SDL_BUTTON_RMASK) != 0) {
					mouse_Button = MOUSE_BUTTON_RIGHT;
				}
				else if ((buttons & SDL_BUTTON_X1MASK) != 0) {
					mouse_Button = MOUSE_BUTTON_X1;
				}
				else if ((buttons & SDL_BUTTON_X2MASK) != 0) {
					mouse_Button = MOUSE_BUTTON_X2;
				}
				break;
			}

			case SDL_MOUSEWHEEL:
			{
				if (currentTimeMillis > g_mouseTime) {
					g_mouseTime = currentTimeMillis + 128;
					if (ev.wheel.y > 0) {
						mouse_Button = MOUSE_BUTTON_WHELL_UP;
					}
					else if (ev.wheel.y < 0) {
						mouse_Button = MOUSE_BUTTON_WHELL_DOWN;
					}
				}
				break;
			}

			case SDL_MOUSEMOTION:
			{
				if (!doomRpg->menuSystem->setBind) {
					if (currentTimeMillis > g_mouseTime) {
						g_mouseTime = currentTimeMillis + 128;
						int x = 0, y = 0;
						SDL_GetRelativeMouseState(&x, &y);

						int sensivity = (doomRpg->doomCanvas->mouseSensitivity * 1000) / 100;

						if (x <= -sensivity) {
							mouse_Button = MOUSE_BUTTON_MOTION_LEFT;
						}
						else if (x >= sensivity) {
							mouse_Button = MOUSE_BUTTON_MOTION_RIGHT;
						}

						if (doomRpg->doomCanvas->mouseYMove) {
							if (y <= -sensivity) {
								mouse_Button = MOUSE_BUTTON_MOTION_UP;
							}
							else if (y >= sensivity) {
								mouse_Button = MOUSE_BUTTON_MOTION_DOWN;
							}
						}
					}
				}
				break;
			}

			case SDL_WINDOWEVENT:
			{
				if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
					//printf("MESSAGE:Resizing window...\n");
					//SDL_Rect rect = { 0,0,640,480};
					//SDL_RenderSetViewport(sdlVideo.renderer, &rect);
					//resizeWindow();
				}

				if (ev.window.event == SDL_WINDOWEVENT_CLOSE) {
					SDL_Log("Window %d closed", ev.window.windowID);
					closeZipFile(&zipFile);
					DoomRPG_FreeAppData(doomRpg);
					DoomRPG_CloseAudio();
					SDL_Close();
#ifdef __EMSCRIPTEN__
					emscripten_cancel_main_loop();
#else
					exit(0);
#endif
					break;
				}

				if (ev.window.event != SDL_WINDOWEVENT_CLOSE)
				{
					int w, h;
					SDL_GetWindowSize(sdlVideo.window, &w, &h);
					SDL_WarpMouseInWindow(sdlVideo.window, w / 2, h / 2);
					SDL_GetRelativeMouseState(NULL, NULL);
				}
				break;
			}

			case SDL_QUIT:
			{
				// shut down
#ifdef __EMSCRIPTEN__
				emscripten_cancel_main_loop();
#else
				exit(0);
#endif
				break;
			}
		}

		g_key = DoomRPG_getEventKey(mouse_Button, g_keyboardState);
		if (g_key != g_oldKey) {
			//printf("oldKey %d\n", g_oldKey);
			//printf("key %d\n", g_key);

			g_oldKey = g_key;
			if (!doomRpg->menuSystem->setBind) {
				DoomCanvas_keyPressed(doomRpg->doomCanvas, g_key);
			}
			else {
				goto setBind;
			}
		}
		else if (g_key == 0) {
		setBind:
			if (doomRpg->menuSystem->setBind) {
				DoomRPG_setBind(doomRpg, mouse_Button, g_keyboardState);
			}
		}
	}

	if (currentTimeMillis > g_UpTime) {
		g_UpTime = currentTimeMillis + 15;
		DoomRPG_loopGame(doomRpg);
	}

#ifndef __EMSCRIPTEN__
	// For native builds, check if we should exit
	if (doomRpg->closeApplet == true) {
		closeZipFile(&zipFile);
		DoomRPG_FreeAppData(doomRpg);
		DoomRPG_CloseAudio();
		SDL_Close();
		exit(0);
	}
#endif
}

int main(int argc, char* args[])
{
	Z_Init();
	SDL_InitVideo();
	DoomRPG_InitAudio();

#ifdef __EMSCRIPTEN__
	// For Emscripten, the zip file is written to virtual filesystem by JavaScript before main() runs
	openZipFile("/DoomRPG.zip", &zipFile);
#else
	openZipFile("DoomRPG.zip", &zipFile);
#endif

	/*int size;
	byte* data;
	data = readZipFileEntry("c.bmp", &zipFile, &size);

	SDL_RWops* rw;
	rw = SDL_RWFromFile("c.bmp", "w");
	SDL_RWwrite(rw, data, sizeof(byte), size);
	SDL_RWclose(rw);

	SDL_free(data);

	closeZipFile(&zipFile);*/

	if (DoomRPG_Init() == 0) {
		DoomRPG_Error("Failed to initialize Doom Rpg\n");
	}

	//Hud_addMessage(doomRpg->hud, "Bienvenido a Doom RPG por GEC...");

	g_keyboardState = SDL_GetKeyboardState(NULL);
	g_key = 0;
	g_oldKey = -1;

#ifdef __EMSCRIPTEN__
	// For Emscripten, use the browser's requestAnimationFrame loop
	// 0 = use browser's refresh rate, 1 = simulate infinite loop
	emscripten_set_main_loop(mainLoopIteration, 0, 1);
#else
	// Native build - use traditional while loop
	while (doomRpg->closeApplet != true)
	{
		mainLoopIteration();
	}

	closeZipFile(&zipFile);
	DoomRPG_FreeAppData(doomRpg);
	DoomRPG_CloseAudio();
	SDL_Close();
#endif

	return 0;
}