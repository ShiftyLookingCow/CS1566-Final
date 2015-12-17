/***************************************************
FILE: glmain.h
AUTHOR: gem, loosely based on hwa random skel
DATE: 02/28/08
DOES: header file for cs1566 Assignment 4 -- Modeler
***************************************************/

#ifndef __CS1566_GLOBALS
#define __CS1566_GLOBALS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <windows.h>
#include <fstream>
#include <iostream>
// may need to change GLUT/glut.h to GL/glut.h on PC or Linux
#include <GL/glut.h>

using namespace std;


#ifndef min //In VC++ 2008, Ryan got a warning about min redefinition, so let's not redefine it if something else already did; sigh
#define min(a,b) ((a) < (b)? a:b)
#endif

#ifndef M_PI //In VS 2010 M_PI seems to be missing, so we will put it here
#define M_PI 3.14159265
#endif

#define FALSE 0 
#define TRUE  1

/* define index constants into the colors array */
#define BLACK   0
#define RED     1
#define YELLOW  2
#define MAGENTA 3
#define GREEN   4
#define CYAN    5
#define BLUE    6
#define GREY    7
#define WHITE   8

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

typedef GLfloat Vector[4];

typedef struct _Object {
  int sid;

  GLfloat color[3];

  GLfloat vertices_cube_smart[8][4];
  GLfloat normals_cube_smart[8][3];
  
  GLfloat vertices_house[9][4];
  GLfloat normals_house[9][3];

  GLfloat vertices_obj[50][50][4];
  GLfloat normals_obj[50][50][3];

  GLfloat* pos;
  GLfloat* at;
  GLfloat* dir;
 
  GLfloat shine;
  GLfloat emi[4];
  GLfloat amb[4];
  GLfloat diff[4];
  GLfloat spec[4];

  GLfloat translate[4];
  GLfloat scale[4];
  GLfloat rotate[4];

}OBJECT;

GLfloat identity_matrix[4][4] = {
	{1, 0, 0, 0},
	{0, 1, 0, 0},
	{0, 0, 1, 0},
	{0, 0, 0, 1}
};

GLfloat vertices_house[][4] = {
  {0,2,0,1}, 
  {-1,1,1,1}, {1,1,1,1}, {1,1,-1,1}, {-1,1,-1,1},
  {-1,-1,1,1}, {1,-1,1,1}, {1,-1,-1,1}, {-1,-1,-1,1}
};

GLfloat colors [][3] = {
  {0.0, 0.0, 0.0},  /* black   */
  {1.0, 0.0, 0.0},  /* red     */
  {1.0, 1.0, 0.0},  /* yellow  */
  {1.0, 0.0, 1.0},  /* magenta */
  {0.0, 1.0, 0.0},  /* green   */
  {0.0, 1.0, 1.0},  /* cyan    */
  {0.0, 0.0, 1.0},  /* blue    */
  {0.5, 0.5, 0.5},  /* 50%grey */
  {1.0, 1.0, 1.0}   /* white   */
};

GLfloat vertices_axes[][4] = {
	{0.0, 0.0, 0.0, 1.0},  /* origin */ 
	{5.0, 0.0, 0.0, 1.0},  /* maxx */ 
	{0.0, 5.0, 0.0, 1.0}, /* maxy */ 
	{0.0, 0.0, 5.0, 1.0}  /* maxz */ 
};

void glut_setup(void) ;
void gl_setup(void) ;
void my_init(int argc, char **argv);
void my_setup(int argc, char **argv);
void myabort(void);
void my_display(void) ;
void my_mouse(int button, int state, int mousex, int mousey) ;
void my_mouse_drag (int mousex, int mousey);
void my_reshape(int w, int h) ;
void my_keyboard_up( unsigned char key, int x, int y ) ;
void my_keyboard( unsigned char key, int x, int y ) ;
void my_idle(void) ;
void my_TimeOut(int id) ;

void make_house(OBJECT *po);
void make_sphere(OBJECT *po, double ray);
void make_cylinder(OBJECT *po, double height, double ray);
void make_cone(OBJECT *po, double r, double height);
void make_torus(OBJECT *po, double r1, double r2);
void calculate_normals(OBJECT *po);

void transform_object(OBJECT* po, GLfloat transformation[4][4]);
void matrix_multiply(GLfloat a[4][4], GLfloat b[4][4], GLfloat c[4][4]);
GLfloat* matrix_multiply(GLfloat transform[4][4], GLfloat v[4]);
void transpose_matrix(GLfloat m[4][4]);

GLfloat* rotate_arbitrary_axis(GLfloat deg, GLfloat ax, GLfloat ay, GLfloat az, GLfloat cx, GLfloat cy, GLfloat cz, GLfloat pt[4]);
GLfloat* real_rot_axis_point(GLfloat deg, GLfloat ax, GLfloat ay, GLfloat az, GLfloat pt[4]);
void rotate_camera(int deg);
void pitch_camera(int deg);

void update_and_display(int id);
void draw_quad(GLfloat vertices[][4], GLfloat *normals, int iv1, int iv2, int iv3, int iv4, const GLfloat* colors);
void draw_shape(OBJECT *po);
void draw_house(OBJECT *po);

void normalize(GLfloat *p);
void cross(GLfloat *a, GLfloat *b, GLfloat *c, GLfloat *d);

void lighting_setup();
void print_matrix(float my_matrix[]);

void read_spec(char *fname) ;
void parse_nums(char *buffer, GLfloat nums[]) ;
void parse_bldg(char *buffer);
void parse_light(char *buffer);

void gen_vertices(void);
void my_mult_pt(GLfloat *p);

void draw_axes( void );

int unit_cube_intersect(GLfloat *rayStart, GLfloat *rayDirection, GLfloat result[2][4]);
int unit_sphere_intersect(GLfloat *rayStart, GLfloat *rayDirection, GLfloat results[2][4]);

float dotprod(float v1[], float v2[]);
#endif