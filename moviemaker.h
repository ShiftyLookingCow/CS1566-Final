#ifndef _RAY_TRACE_H_
#define _RAY_TRACE_H_

// platform specific includes
#include <stdlib.h>
#ifdef __APPLE__
 #include <GLUT/glut.h>
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
#else
 #include <GL/glut.h>
 #include <GL/gl.h>
 #include <GL/glu.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <process.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define MAX_LIGHTS  8
#define NUM_OBJECTS 50

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

#define CUBE     1
#define HOUSE    2
#define SPHERE   3
#define CYLINDER 4
#define CONE	   5
#define TORUS    6
#define PLAYER   7
#define INTERSECT   8
#define MESH 9

#define my_assert(X,Y) ((X)?(void) 0:(printf("error:%s in %s at %d", Y, __FILE__, __LINE__), myabort()))

/*
 * Type Definitions
 */

typedef GLfloat Vector[4];
typedef Vector Matrix[4];

typedef struct color_s
{
	float r, g, b, a;
} color_t;

typedef struct _Object {
  int sid;
 
  GLfloat shine;
  GLfloat emi[4];
  GLfloat amb[4];
  GLfloat diff[4];
  GLfloat spec[4];

  GLfloat translate[4];
  GLfloat scale[4];
  GLfloat rotate[4];

  Vector center;
  Matrix ctm;

}OBJECT;

typedef struct FrameData
{
	GLfloat pos[4];
	GLfloat at[4];
	GLfloat up[4];
	int frames;
}FD;

typedef struct _CAM{
  GLfloat pos[4];
  GLfloat at[4];
  GLfloat up[4];

  GLfloat dir[4];
}CAM;

typedef struct _LITE{
  GLfloat amb[4];
  GLfloat diff[4];
  GLfloat spec[4];
  GLfloat pos[4];
  GLfloat dir[3];
  GLfloat angle;
}LITE;

short frame_traced = 0;

//can read up to 5000 frames
FrameData fd[500];
int data_count = 0, cur_index = 0;
int frame_count = 0, cur_frame = 0;

extern GLuint textureId;
extern color_t screen[WINDOW_WIDTH * WINDOW_HEIGHT];

void initGL(void);
void displayScene(void);
void idle(void);
void resizeWindow(int width, int height);

void plotPixel(color_t *screen, int x, int y, float r, float g, float b, float a);

void read_spec(char *fname);
void parse_mcf(char * file);

void generateRay(GLfloat x, GLfloat y, Vector d);

int find_intersects(Vector p, Vector d, OBJECT* po, Vector v);

void real_translation(Matrix m, GLfloat x, GLfloat y, GLfloat z);
void real_scaling(Matrix m, GLfloat sx, GLfloat sy, GLfloat sz);
void real_rotation(Matrix m, GLfloat deg, GLfloat x, GLfloat y, GLfloat z);

/*
 * Global Variables
 */
OBJECT my_objects[NUM_OBJECTS];
LITE my_lights[MAX_LIGHTS];
CAM my_cam;
int num_objects;
int  num_lights;
Matrix identity = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
GLuint texId;
color_t screen[WINDOW_WIDTH * WINDOW_HEIGHT];
static int raster_x=0;
static int raster_y=0;

//Settings
GLfloat kDiff = 0.9f;
GLfloat kAmb = 0.7f;
GLfloat kSpec = 0.7f;
int SUPER = TRUE;
int SHADOW = TRUE;
int TOON = FALSE;

#endif