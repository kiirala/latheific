#include <stdlib.h>
#include <math.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#ifndef NDEBUG
#include <stdio.h>
#endif

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

int audio_pos = 0;
int16_t noise_sample[1024];
int noise_pos = 0;
int noise_step = 0;
int noise_counter = 0;
int noises[] = {10, 14, 10, 20, 14, 10, 20, 10};
void audio_callback(void *userdata, uint8_t *stream, int len) {
  int16_t *out = (int16_t*)stream;

  for (int i = 0 ; i < len / 2 ; ++i) {
    noise_step = noises[(int)floor(audio_pos / 44100.0f / 4.0f)];
    noise_counter += noise_step;
    if (noise_counter > 0x1000) {
      noise_counter -= 0x1000;
      noise_pos++;
    }
    if (noise_pos >= 1024) {
      noise_pos = 0;
    }
    float volume = sin(audio_pos / 44100.0f * 2 * M_PI + M_PI / 2.0f) + 1.0f / 2.0f;
    out[i] = noise_sample[noise_pos] * volume;
    audio_pos++;
  }
}

static void colour_hsl(float *colour, const int h, const int s, const int l) {
  const int q = ((l < 128) ? (l * 256 + l * s) : ((l + s) * 256 - (l * s))) / 256;
  const int p = 2 * l - q;
  for (int col_n = 0 ; col_n < 3 ; col_n++) {
    int tmp_col;
    int t = h - col_n * 85 + 85;
    //if (t < 0) t += 256;
    //if (t >= 256) t -= 256;
    t &= 255;
    if (t < 43) {
      tmp_col = p + ((q - p) * 6 * t) / 256;
    } else if (t < 128) {
      tmp_col = q;
    } else if (t < 171) {
      tmp_col = p + ((q - p) * (171 - t) * 6) / 256;
    } else {
      tmp_col = p;
    }
    if (tmp_col > 255) tmp_col = 255;
    if (tmp_col < 0) tmp_col = 0;
    colour[col_n] = tmp_col / 255.0f;
  }
  colour[3] = 1.0f;
}

int use_lights = 0;
int s;
struct sockaddr_in sockaddr;

static void setLights(int r, int g, int b) {
  if (use_lights) {
    const int lightCount = 39;
    char cmd[10 + 6 * 39] = {1, 0, 'k', 'i', 'i', 'r', 'a', 'l', 'a', 0};
    for (int i = 0 ; i < lightCount ; ++i) {
      cmd[i * 6 + 10] = 1;
      cmd[i * 6 + 1 + 10] = i;
      cmd[i * 6 + 2 + 10] = 0;
      cmd[i * 6 + 3 + 10] = r;
      cmd[i * 6 + 4 + 10] = g;
      cmd[i * 6 + 5 + 10] = b;
    }
    sendto(s, cmd, sizeof(cmd), 0, &sockaddr, sizeof(sockaddr));
  }
}

static int const aspectratio_w = 16;
static int const aspectratio_h = 9;
static int viewport[4];

typedef struct {
  int count;
  GLfloat *loc;
  GLfloat *normal;
  GLfloat *colour;
} Mesh;

typedef struct {
  Mesh *mesh;
  GLuint vao, *vbo;
} Object;

#ifndef NDEBUG
#define logGL() logGL_impl(__FILE__, __LINE__)
static void logGL_impl(const char *file, int line) {
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL error %d in %s:%d", err, file, line);
  }
}
#else
#define logGL() { }
#endif

static void setupProjection() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-aspectratio_w, aspectratio_w,
	    -aspectratio_h, aspectratio_h,
	    1, 100);
  
  glMatrixMode(GL_MODELVIEW);    
  glLoadIdentity();
}

static void drawAxis() {
  /*
  glShadeModel(GL_FLAT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  //glUseProgram(0);

  glBegin (GL_LINES);
  glColor3f (1., 0., 0.);
  glVertex3f (0., 0., 0.);
  glVertex3f (1., 0., 0.);
  glEnd ();
	
  glBegin (GL_LINES);
  glColor3f (0., 1., 0.);
  glVertex3f (0., 0., 0.);
  glVertex3f (0., 1., 0.);
  glEnd ();
	
  glBegin (GL_LINES);
  glColor3f (0., 0., 1.);
  glVertex3f (0., 0., 0.);
  glVertex3f (0., 0., 1.);
  glEnd ();
  */
}

static Mesh* lathe(float *points, int pointCount, int steps) {
  int vertexCount = (pointCount * 2 + 2) * steps - 2;
  Mesh* mesh = malloc(sizeof(Mesh));
  mesh->count = vertexCount;
  mesh->loc = malloc(sizeof(GLfloat) * vertexCount * 3);
  mesh->normal = malloc(sizeof(GLfloat) * vertexCount * 3);
  mesh->colour = malloc(sizeof(GLfloat) * vertexCount * 3);
  int outpos = 0;

  for (int step = 0 ; step < steps ; ++step) {
    for (int point = 0 ; point < pointCount ; ++point) {
      for (int foo = 0 ; foo < 2 ; ++foo) {
	float rot = (step + foo) * 2 * M_PI / steps;
	float z = points[point * 2 + 1];
	float distance = points[point * 2];
	float x = cos(rot) * distance;
	float y = sin(rot) * distance;
	int duplicateCount = 1;
	if ((step < steps - 1 && point == pointCount - 1 && foo == 1) ||
	    (step > 0 && point == 0 && foo == 0)) {
	  duplicateCount = 2;
	}
	for (int i = 0 ; i < duplicateCount ; ++i) {
	  mesh->loc[outpos * 3] = x;
	  mesh->normal[outpos * 3] = 0;
	  mesh->colour[outpos * 3] = 0;
	  mesh->loc[outpos * 3 + 1] = y;
	  mesh->normal[outpos * 3 + 1] = 0;
	  mesh->colour[outpos * 3 + 1] = 1;
	  mesh->loc[outpos * 3 + 2] = z;
	  mesh->normal[outpos * 3 + 2] = 0;
	  mesh->colour[outpos * 3 + 2] = 1;
	  outpos++;
	}
      }
    }
  }
  return mesh;
}

static void ballVertexMap(Mesh *mesh) {
  for (int i = 0 ; i < mesh->count ; ++i) {
    float x = mesh->loc[i * 3];
    float y = mesh->loc[i * 3 + 1];
    float z = mesh->loc[i * 3 + 2];
    float len = sqrt(x*x + y*y + z*z);
    mesh->normal[i * 3] = x / len;
    mesh->normal[i * 3 + 1] = y / len;
    mesh->normal[i * 3 + 2] = y / len;
  }
}

Mesh* someMesh;
Object* someObject;

void blinkenlichts() {
  use_lights = 1;
  s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  memset(&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(9909);
  inet_aton("192.168.10.1", &sockaddr.sin_addr);
}

void load() {
  float points[] = {0, -1, 1, -.2, .8, .2, 0, 1};
  someMesh = lathe(points, 4, 20);
  ballVertexMap(someMesh);

  someObject = malloc(sizeof(Object));
  someObject->mesh = someMesh;

  glGenVertexArrays(1, &someObject->vao);
  glBindVertexArray(someObject->vao);
  someObject->vbo = malloc(sizeof(GLuint) * 2);

  glGenBuffers(3, someObject->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, someObject->vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, someMesh->count * 3 * sizeof(GLfloat),
	       someMesh->loc, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, someObject->vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, someMesh->count * 3 * sizeof(GLfloat),
	       someMesh->colour, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, someObject->vbo[2]);
  glBufferData(GL_ARRAY_BUFFER, someMesh->count * 3 * sizeof(GLfloat),
	       someMesh->normal, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  logGL();

  for (int i = 0 ; i < 1024 ; ++i) {
    noise_sample[i] = (int16_t)(rand() / (RAND_MAX / 65536));
  }
}

void unload() {
}

int last_lights = 0;
void render(int time) {
  setupProjection();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
      
  int xcount = 16;
  int ycount = 12;
  for (int x = 0 ; x < xcount ; ++x) {
    for (int y = 0 ; y < ycount ; ++y) {
      glLoadIdentity();
      glRotatef(sin(time / 1000.0f) * 360.0, 0, 0, 1);
      glTranslatef((x - xcount / 2 + sin(time / 1000.0f * 39.0f)) * 3.5,
		   (y - ycount / 2 + sin(time / 1000.0f * 239.0f)) * 3.5,
		   -2);
      glRotatef ((time / 1000.0f + (x ^ y) / 16.0f) * 360.0f, 1, 0, 1);

      float colour[4];
      const int hues[] = {128, 86, 128, 0, 86, 128, 0, 128};
      colour_hsl(colour,
		 (int)(sin(time / 1000.0f) * 20 + hues[time / 1000 / 4] * 15) % 256,
		 100,
		 sin(time / 1000.0f * 2 * M_PI) * 128 + 128);
      float white[4] = {1, 1, 1, 1};

      glLightfv(GL_LIGHT0, GL_DIFFUSE, colour);
      glLightfv(GL_LIGHT0, GL_SPECULAR, white);

      if (time >= last_lights + 25) {
	setLights(colour[0] * 255, colour[1] * 255, colour[2] * 255);
	last_lights = time;
      }

      //glScalef(0.3, 0.3, 0.3);
      // glRotatef (ang, 0, 1, 0);
      // glRotatef (ang, 0, 0, 1);
      
      //glScalef(2, 2, 2);
      //glBindVertexArray(someObject->vao);
      logGL();
      glBindVertexArray(0);
      glEnableClientState(GL_VERTEX_ARRAY);
      glBindBuffer(GL_ARRAY_BUFFER, someObject->vbo[0]);
      glVertexPointer(3, GL_FLOAT, 0, 0);
      glBindBuffer(GL_ARRAY_BUFFER, someObject->vbo[1]);
      glColorPointer(3, GL_FLOAT, 0, 0);
      glBindBuffer(GL_ARRAY_BUFFER, someObject->vbo[2]);
      glNormalPointer(GL_FLOAT, 0, 0);
      logGL();
      glDrawArrays(GL_TRIANGLE_STRIP, 0, someObject->mesh->count);
      glDisableClientState(GL_VERTEX_ARRAY);
      logGL();
    }
  }
  //drawAxis();
}

void resize(int width, int height) {
  //scene->screen_width = width;
  //scene->screen_height = height;

  int used_w, used_h;
  if (width * aspectratio_h >= height * aspectratio_w) {
    used_w = height * aspectratio_w / aspectratio_h;
    used_h = height;
  }
  else {
    used_w = width;
    used_h = width * aspectratio_h / aspectratio_w;
  }
  
  viewport[0] = (width - used_w) / 2;
  viewport[1] = (height - used_h) / 2;
  viewport[2] = used_w;
  viewport[3] = used_h;
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}
