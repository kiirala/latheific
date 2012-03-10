#include <stdint.h>
#include <SDL.h>

#include <GL/gl.h>
/*
#include <dlfcn.h>

int (*mSDL_Init)(Uint32 flags);
SDL_Surface * (*mSDL_SetVideoMode)(int width, int height, int bpp, Uint32 flags);
void (*mSDL_Quit)();
void (*mSDL_WM_SetCaption)(const char *title, const char *icon);
int (*mSDL_OpenAudio)(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
int (*mSDL_ShowCursor)(int toggle);
void (*mSDL_PauseAudio)(int pause_on);
int (*mSDL_PollEvent)(SDL_Event *event);
Uint32 (*mSDL_GetTicks)(void);
void (*mSDL_GL_SwapBuffers)(void);

const char *sdlNames = "SDL_Init\0"
  "SDL_SetVideoMode\0"
  "SDL_Quit\0"
  "SDL_WM_SetCaption\0"
  "SDL_OpenAudio\0"
  "SDL_ShowCursor\0"
  "SDL_PauseAudio\0"
  "SDL_PollEvent\0"
  "SDL_GetTicks\0"
  "SDL_GL_SwapBuffers\0";

void** sdlPointers[] = {(void*)&mSDL_Init,
			(void*)&mSDL_SetVideoMode,
			(void*)&mSDL_Quit,
			(void*)&mSDL_WM_SetCaption,
			(void*)&mSDL_OpenAudio,
			(void*)&mSDL_ShowCursor,
			(void*)&mSDL_PauseAudio,
			(void*)&mSDL_PollEvent,
			(void*)&mSDL_GetTicks,
			(void*)&mSDL_GL_SwapBuffers};

void link() {
  void *sdl = dlopen("libSDL.so", RTLD_NOW);
  
  const char *pos = sdlNames;
  int i = 0;
  while (*pos) {
    *sdlPointers[i] = dlsym(sdl, pos);
    while (*pos) ++pos;
    ++pos;
  }
}
*/

typedef char bool;
#define false 0
#define true 1

extern void load();
extern void unload();
extern void render(int time);
extern void resize(int width, int height);
extern void audio_callback(void *userdata, uint8_t *stream, int len);
extern void blinkenlichts();

bool fullscreen = true;
int scrWidth = 1920;
int scrHeight = 1080;
const int scrBPP = 32;

static void init_sdl() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
    exit(11);
  }

  //SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  //SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

  /* Set the video mode */
  int videoFlags = SDL_OPENGL;
  if (fullscreen)
    videoFlags |= SDL_FULLSCREEN;

  SDL_Surface *surface = SDL_SetVideoMode(scrWidth, scrHeight, scrBPP,
					  videoFlags);
  if (!surface) {
    SDL_Quit();
    exit(12);
  }

  SDL_WM_SetCaption("Latheific!", "Latheific!");

  SDL_AudioSpec aspec;
  aspec.freq = 44100;
  aspec.format = AUDIO_S16;
  aspec.channels = 1;
  aspec.samples = 2048;
  aspec.callback = audio_callback;
  aspec.userdata = 0;
  if (SDL_OpenAudio(&aspec, NULL) != 0) {
    SDL_Quit();
    exit(13);
  }
}

//void cleanup(int state, void *foo) {
  //Sound_FreeSample(data.sample);  /* clean up SDL_Sound resources... */
//}
/*
int main(int argc, char **argv) {
  for (int i = 1 ; i < argc ; ++i) {
    if (argv[i][0] == 'w') {
      fullscreen = false;
      scrWidth = 800;
      scrHeight = 450;
    }
    else if (argv[i][0] == 'l') {
      blinkenlichts();
    }
  }
*/
void _start() {
#ifdef WINDOWED
  fullscreen = false;
  scrWidth = 800;
  scrHeight = 450;
#endif

#ifdef BLINKENLICHTS
  blinkenlichts();
#endif

  //  link();

  init_sdl();
  //on_exit(cleanup, 0);
  load();
  resize(scrWidth, scrHeight);
  SDL_ShowCursor(SDL_DISABLE);

  SDL_Event event;
  bool running = true;
  SDL_PauseAudio(0);

  while (running) {
    while (SDL_PollEvent(&event)) {
      switch( event.type ) {
      case SDL_KEYDOWN:
	if (event.key.keysym.sym == SDLK_ESCAPE)
	  running = false;
	break;
      case SDL_QUIT:
        running = false;
        break;

      default:
        break;
      }
    }
    
    uint32_t time = SDL_GetTicks();
    render(time);
    SDL_GL_SwapBuffers();
    if (time > 32 * 1000) running = false;
  }

  unload();
  SDL_Quit();

  __asm__ (
  "movl $1,%eax\n"
  "xor %ebx,%ebx\n"
  "int $128\n"
  );
}
/*
  return 0;
}
*/
