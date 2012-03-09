#include <GL/gl.h>

static int const aspectratio_w = 16;
static int const aspectratio_h = 9;
static int viewport[4];

typedef struct {
  GLfloat *loc;
  GLfloat *normal;
  GLfloat *colour;
} Mesh;

typedef struct {
  Mesh *mesh;
  GLuint vao, *vbo;
} Object;

static void setupProjection() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-aspectratio_w, aspectratio_w,
	    -aspectratio_h, aspectratio_h,
	    1, 100);
  
  glMatrixMode(GL_MODELVIEW);    
  glLoadIdentity();
}

void load() {
}

void unload() {
}

void render(int time) {
  setupProjection();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glRotatef (time / 1000.0f * 3.14f, 1, 0, 1);
  glTranslatef(0, 0, -5);
  //glScalef(0.3, 0.3, 0.3);
  // glRotatef (ang, 0, 1, 0);
  // glRotatef (ang, 0, 0, 1);

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
