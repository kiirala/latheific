#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

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

#define logGL() logGL_impl(__FILE__, __LINE__)
static void logGL_impl(const char *file, int line) {
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL error %d in %s:%d", err, file, line);
  }
}

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
}

void unload() {
}

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
      glRotatef(sin(time / 1000.0f), 0, 0, 1);
      glTranslatef((x - xcount / 2 + sin(time / 1000.0f * 39.0f)) * 3.5,
		   (y - ycount / 2 + sin(time / 1000.0f * 239.0f)) * 3.5,
		   -2);
      glRotatef ((time / 1000.0f + (x ^ y) / 16.0f) * 360.0f, 1, 0, 1);
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
