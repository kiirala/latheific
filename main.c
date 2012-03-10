#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_sound.h>
#include <time.h>
#include <unistd.h>

#ifndef NDEBUG
#include <stdio.h>
#endif

#include <GL/gl.h>

typedef char bool;
#define false 0
#define true 1

extern void load();
extern void unload();
extern void render(int time);
extern void resize(int width, int height);
extern void audio_callback(void *userdata, uint8_t *stream, int len);
extern void blinkenlichts();

bool fullscreen = false;
int scrWidth = 800;
int scrHeight = 450;
const int scrBPP = 32;
bool global_done_flag = false;

static void init_sdl() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
    fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

  /* Set the video mode */
  int videoFlags = SDL_OPENGL;
  if (fullscreen)
    videoFlags |= SDL_FULLSCREEN;

  SDL_Surface *surface = SDL_SetVideoMode(scrWidth, scrHeight, scrBPP,
					  videoFlags);
  if (!surface) {
    fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
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
    fprintf(stderr, "Opening audio failed: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }
}

void cleanup(int state, void *foo) {
  //Sound_FreeSample(data.sample);  /* clean up SDL_Sound resources... */
  unload();
  SDL_Quit();
}

int main(int argc, char **argv) {
  int opt;
  while ((opt = getopt(argc, argv, "fwsl:")) != -1) {
    switch (opt) {
    case 'f':
      fullscreen = true;
      break;
    case 'w':
      fullscreen = false;
      break;
    case 'l':
      blinkenlichts();
      break;
    case 's': {
      int r = sscanf(optarg, "%dx%d", &scrWidth, &scrHeight);
      if (r == 2)
	break;
      // else fall through
    }
    default: /* '?' */
      fprintf(stderr,
	      "Usage: %s [-f] [-w] [-s 1280x720]\n"
	      "    -f: fullscreen\n"
	      "    -w: windowed\n"
	      "    -s: screen resolution\n",
	      argv[0]);
      exit(EXIT_FAILURE);
    }
  }


  init_sdl();
  on_exit(cleanup, 0);
  load();
  resize(scrWidth, scrHeight);
  //load_song();
  SDL_ShowCursor(SDL_DISABLE);

  SDL_Event event;
  uint32_t last_draw = 0;
  uint32_t drawn = 0;
  uint32_t start = SDL_GetTicks();
  bool running = true;
  SDL_PauseAudio(0);

  while (running && !global_done_flag) {
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
    last_draw = time;
    drawn++;
    if (time > 32 * 1000) running = false;
  }

#ifndef NDEBUG
  uint32_t end = SDL_GetTicks();
  double runtime = (end - start) / 1000.0;
  fprintf(stderr, "Drawn %u frames\n", drawn);
  fprintf(stderr, "Runtime %.2f s, %.2f fps drawn\n",
	  runtime, drawn / runtime);
#endif

  return 0;
}
