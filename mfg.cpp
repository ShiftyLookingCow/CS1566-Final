/**************************************************************************
File: glmain.c
Does: basic lighting and modeling for cs1566 hw4 Modeler
Author: Steven Lauck, based on some hwa
Date: 01/08/09
**************************************************************************/

#include "mfg.h"
#define my_assert(X,Y) ((X)?(void) 0:(printf("error:%s in %s at %d", Y, __FILE__, __LINE__), myabort()))

#define min(a,b) ((a) < (b)? a:b)
#define FALSE 0 
#define TRUE  1
#define MAX_LIGHTS  8
#define NUM_OBJECTS 8

//Set FPS
short fps = 30;

const float movement_speed = 0.4;
const float rotation_speed = 0.4;
const float flight_speed = 0.4;

ofstream output ( "test.mcf" );

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

double start_time, current_time;

short recording = 0;
short show_axes = 0;
short w_pressed = 0, a_pressed = 0, s_pressed = 0, d_pressed = 0, lm_pressed = 0, rm_pressed = 0;

double cyl_height=2;
double cyl_ray=1;
double sph_ray=1;
double cone_height = 2;
double cone_ray = 1;
double torus_r1 = 1;
double torus_r2 = 0.2;

void make_cube_smart(OBJECT *po, double size );

void real_translation(OBJECT *po, GLfloat x, GLfloat y, GLfloat z);
void real_scaling(OBJECT *po, GLfloat sx, GLfloat sy, GLfloat sz);
void real_rotation(OBJECT *po, GLfloat deg, GLfloat x, GLfloat y, GLfloat z);

OBJECT my_objects[NUM_OBJECTS];
LITE my_lights[MAX_LIGHTS];
int num_objects;
int  num_lights;

// camera variables
CAM my_cam;
GLfloat cam_y_vel = 0;

int crt_render_mode;
int crt_shape, crt_rs, crt_vs;
int crt_transform;

void myabort(void) {
  abort();
  exit(1); /* exit so g++ knows we don't return. */
} 

int main(int argc, char** argv)
{ 
  setbuf(stdout, NULL);   /* for writing to stdout asap */
  glutInit(&argc, argv);

  my_setup(argc, argv);  
  glut_setup();
  gl_setup();

  glutMainLoop();
  return(0);
}


void glut_setup (){

  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
  
  glutInitWindowSize(WINDOW_WIDTH,WINDOW_HEIGHT);
  glutInitWindowPosition(20,20);
  glutCreateWindow("Movie File Generator");

  //hide mouse
  glutSetCursor(GLUT_CURSOR_NONE); 

  /* set up callback functions */
  glutDisplayFunc(my_display);
  glutReshapeFunc(my_reshape);
  glutMouseFunc(my_mouse);
  glutMotionFunc(my_mouse_drag);
  glutPassiveMotionFunc(my_mouse_drag);
  glutKeyboardFunc(my_keyboard);
  glutKeyboardUpFunc(my_keyboard_up);
  glutIdleFunc( my_idle );

  glutTimerFunc(1.0/fps*1000, update_and_display, 0); //set a timer to update positions

  return;
}

void gl_setup(void) {

  // enable depth handling (z-buffer)
  glEnable(GL_DEPTH_TEST);

  // enable auto normalize
  glEnable(GL_NORMALIZE);

  // define the background color 
  glClearColor(0,0,0,1);

  glMatrixMode(GL_PROJECTION) ;
  glLoadIdentity() ;
  gluPerspective( 40, 1.0, 1, 200.0);
  glMatrixMode(GL_MODELVIEW) ;
  glLoadIdentity() ;  // init modelview to identity

  // toggle to smooth shading (instead of flat)
  glShadeModel(GL_SMOOTH); 
  lighting_setup();


  return ;
}

void my_setup(int argc, char **argv){
  num_objects = num_lights = 0;

  // initialize global shape defaults and mode for drawing
  crt_render_mode = GL_POLYGON;

  crt_rs = 30;
  crt_vs = 30;
 
  //If you want to allow the user to type in the spec file
  //then modify the following code.
  //Otherwise, the program will attempt to load the file as specified
  //on the command line:
  //EX: ./glmain spec3
  my_assert(argc >1, "need to supply a spec file");
  read_spec(argv[1]);
  return;
}

void parse_floats(char *buffer, GLfloat nums[]) {
  int i;
  char *ps;

  ps = strtok(buffer, " ");
  for (i=0; ps; i++)
  {
    nums[i] = atof(ps);
    ps = strtok(NULL, " ");
  }
}

void parse_obj(char *buffer){
  OBJECT *po;
  char *pshape, *pshine, *pemi, *pamb, *pdiff, *pspec, *ptranslate, *pscale, *protate;

  my_assert ((num_objects < NUM_OBJECTS), "too many objects");
  po = &my_objects[num_objects++];

  pshape  = strtok(buffer, " ");
  //printf("pshape is %s\n",pshape);

  ptranslate    = strtok(NULL, "()");  strtok(NULL, "()");
  pscale        = strtok(NULL, "()");  strtok(NULL, "()"); 
  protate       = strtok(NULL, "()");  strtok(NULL, "()");  

  pshine  = strtok(NULL, "()");strtok(NULL, "()");
  //printf("pshine is %s\n",pshine);
 
  pemi    = strtok(NULL, "()");  strtok(NULL, "()"); 
  pamb    = strtok(NULL, "()");  strtok(NULL, "()"); 
  pdiff   = strtok(NULL, "()");  strtok(NULL, "()"); 
  pspec   = strtok(NULL, "()");  strtok(NULL, "()"); 

  po->sid  = atoi(pshape);
  po->shine = atof(pshine);

  parse_floats(ptranslate, po->translate);
  parse_floats(pscale, po->scale);
  parse_floats(protate, po->rotate);

  parse_floats(pemi, po->emi);
  parse_floats(pamb, po->amb);
  parse_floats(pdiff, po->diff);
  parse_floats(pspec, po->spec);

  // use switch to create your objects, cube given as example
  switch (po->sid){
	  case 1: //cube
		make_cube_smart(po, 1);
		break;
	  case 2: //house
		  make_house(po);
		  break;
	  case 3: //sphere
		  make_sphere(po, sph_ray);
		  break;
	  case 4: //cylinder
		  make_cylinder(po, cyl_height, cyl_ray);
		  break;
	  case 5: //mesh
		   
		  break;
  }
  
  // scale, rotate, translate using your real tranformations from assignment 3 depending on input from spec file
  real_scaling(po, po->scale[0], po->scale[1], po->scale[2]);  
  real_rotation(po, po->rotate[0], 1, 0, 0);
  real_rotation(po, po->rotate[1], 0, 1, 0);
  real_rotation(po, po->rotate[2], 0, 0, 1);
  real_translation(po, po->translate[0], po->translate[1], po->translate[2]);

  calculate_normals(po);
  
  printf("read object\n");
}

void parse_camera(char *buffer){
  CAM *pc;
  char *ppos, *plook, *pup;

  pc = &my_cam;

  strtok(buffer, "()");
  ppos  = strtok(NULL, "()");  strtok(NULL, "()"); 
  plook  = strtok(NULL, "()");  strtok(NULL, "()"); 
  pup  = strtok(NULL, "()");  strtok(NULL, "()"); 

  parse_floats(ppos, pc->pos);
  parse_floats(plook, pc->at);
  parse_floats(pup, pc->up);

  //pc->at[0] += pc->pos[0];
  //pc->at[1] += pc->pos[1];
  //pc->at[2] += pc->pos[2];

  pc->dir[0] = pc->at[0] - pc->pos[0];
  pc->dir[1] = pc->at[1] - pc->pos[1];
  pc->dir[2] = pc->at[2] - pc->pos[2];
  normalize(pc->dir);

  printf("read camera\n");
}

void parse_light(char *buffer){
  LITE *pl;
  char *pamb, *pdiff, *pspec, *ppos, *pdir, *pang;
  my_assert ((num_lights < MAX_LIGHTS), "too many lights");
  pl = &my_lights[++num_lights];

  strtok(buffer, "()");
  pamb  = strtok(NULL, "()");  strtok(NULL, "()"); 
  pdiff = strtok(NULL, "()");  strtok(NULL, "()"); 
  pspec = strtok(NULL, "()");  strtok(NULL, "()"); 
  ppos  = strtok(NULL, "()");  strtok(NULL, "()"); 
  pdir  = strtok(NULL, "()");  strtok(NULL, "()"); 
  pang  = strtok(NULL, "()");

  parse_floats(pamb, pl->amb);
  parse_floats(pdiff, pl->diff);
  parse_floats(pspec, pl->spec);
  parse_floats(ppos, pl->pos);
  if (pdir) {
    parse_floats(pdir, pl->dir);
    pl->angle = atof(pang);
    //printf("angle %f\n", pl->angle);
  }
  else
    pl->dir[0]= pl->dir[1]= pl->dir[2] =0;
  printf("read light\n");

}

/* assuming the spec is going to be properly written
   not error-checking here */
void read_spec(char *fname) {
  char buffer[300];
  FILE *fp;

  fp = fopen(fname, "r");
  my_assert(fp, "can't open spec");
  while(!feof(fp)){
    fgets(buffer, 300, fp);
    //printf("read line: %s\n", buffer);
    switch (buffer[0]) {
    case '#':
      break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
   	  //read in the cube
	  parse_obj(buffer);
 	  break;

    case 'l':
      parse_light(buffer);
      break;

    case 'c':
      parse_camera(buffer);
      break;

    default:
      break;
    }
  }
}

void lighting_setup () {
  int i;
  GLfloat globalAmb[]     = {.1, .1, .1, .1};

  // create flashlight
  GLfloat amb[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat dif[] = {0.8, 0.8, 0.8, 1.0};
  GLfloat spec[] = {5.0, 5.0, 5.0, 1.0};

  //enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);

  // reflective propoerites -- global ambiant light
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

  // setup properties of lighting
  for (i=1; i<=num_lights; i++) {
    glEnable(GL_LIGHT0+i);
    glLightfv(GL_LIGHT0+i, GL_AMBIENT, my_lights[i].amb);
    glLightfv(GL_LIGHT0+i, GL_DIFFUSE, my_lights[i].diff);
    glLightfv(GL_LIGHT0+i, GL_SPECULAR, my_lights[i].spec);
    glLightfv(GL_LIGHT0+i, GL_POSITION, my_lights[i].pos);
    if ((my_lights[i].dir[0] > 0) ||  (my_lights[i].dir[1] > 0) ||  (my_lights[i].dir[2] > 0)) {
      glLightf(GL_LIGHT0+i, GL_SPOT_CUTOFF, my_lights[i].angle);
      glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, my_lights[i].dir);
    }
  }

}

void my_reshape(int w, int h) {
  // ensure a square view port
  glViewport(0,0,min(w,h),min(w,h)) ;
  return ;
}

void real_translation(OBJECT *po, GLfloat x, GLfloat y, GLfloat z) {
	//translation matrix
	GLfloat transformation[4][4] = {
		{1, 0, 0, x},
		{0, 1, 0, y},
		{0, 0, 1, z},
		{0, 0, 0, 1}
	};

	//po->pos[0] += x;
	//po->pos[1] += y;
	//po->pos[2] += z;
	transform_object(po, transformation);
}

void real_scaling(OBJECT *po, GLfloat sx, GLfloat sy, GLfloat sz) {
	//scaling matrix
	GLfloat transformation[4][4] = {
		{sx, 0, 0, 0},
		{0, sy, 0, 0},
		{0, 0, sz, 0},
		{0, 0, 0, 1}
	};

	//po->pos = matrix_multiply(transformation, po->pos);
	transform_object(po, transformation);
}

void real_rotation(OBJECT *po, GLfloat deg, GLfloat x, GLfloat y, GLfloat z) {
	deg *= M_PI / 180;
	//rotation matrix
	GLfloat transformation[4][4] = {0};
	memcpy(transformation, identity_matrix, sizeof(identity_matrix));
	if(x == 1)
	{
		//set rotation about x-axis
		transformation[1][1] = cos(deg);
		transformation[1][2] = -1*sin(deg);
		transformation[2][1] = sin(deg);
		transformation[2][2] = cos(deg);
	}
	else if (y == 1)
	{
		//set rotation about y-axis
		transformation[0][0] = cos(deg);
		transformation[0][2] = sin(deg);
		transformation[2][0] = -1*sin(deg);
		transformation[2][2] = cos(deg);
	}
	else if (z == 1)
	{
		//set rotation about z-axis
		transformation[0][0] = cos(deg);
		transformation[0][1] = -1*sin(deg);
		transformation[1][0] = sin(deg);
		transformation[1][1] = cos(deg);
	}

	transform_object(po, transformation);
}

//applies transformation matrix to each vertex of object
void transform_object(OBJECT* po, GLfloat transformation[4][4])
{
	GLfloat* transformed_vertices;
	//transform vertices
	switch(po->sid)
	{
		case 1: 
			//cube
			for(int i=0; i<8; i++)
			{
				transformed_vertices = matrix_multiply(transformation, po->vertices_cube_smart[i]);
				po->vertices_cube_smart[i][0] = transformed_vertices[0];
				po->vertices_cube_smart[i][1] = transformed_vertices[1];
				po->vertices_cube_smart[i][2] = transformed_vertices[2];
				po->vertices_cube_smart[i][3] = transformed_vertices[3];
			}
			break;

		case 2: 
			//house
			for(int i=0; i<9; i++)
			{
				transformed_vertices = matrix_multiply(transformation, po->vertices_house[i]);
				po->vertices_house[i][0] = transformed_vertices[0];
				po->vertices_house[i][1] = transformed_vertices[1];
				po->vertices_house[i][2] = transformed_vertices[2];
				po->vertices_house[i][3] = transformed_vertices[3];
			}
			break;

		default: 
			//other objects
			for(int i=0; i<crt_rs; i++)
			{
				for(int j=0; j<crt_vs; j++)
				{
					transformed_vertices = matrix_multiply(transformation, po->vertices_obj[i][j]);
					po->vertices_obj[i][j][0] = transformed_vertices[0];
					po->vertices_obj[i][j][1] = transformed_vertices[1];
					po->vertices_obj[i][j][2] = transformed_vertices[2];
					po->vertices_obj[i][j][3] = transformed_vertices[3];
				}
			}
			break;
	}
}

GLfloat* rotate_arbitrary_axis(GLfloat deg, GLfloat ax, GLfloat ay, GLfloat az, GLfloat cx, GLfloat cy, GLfloat cz, GLfloat pt[4])
{
	GLfloat* result = new GLfloat[4];

	pt[0] -= cx;
	pt[1] -= cy;
	pt[2] -= cz;

	result = real_rot_axis_point(deg, ax, ay, az, pt);
	
	result[0] += cx;
	result[1] += cy;
	result[2] += cz;

	return result;
}

GLfloat* real_rot_axis_point(GLfloat deg, GLfloat ax, GLfloat ay, GLfloat az, GLfloat pt[4])
{
		deg *= M_PI / 180;

		//normalize axis vector
		GLfloat m = sqrt( pow(ax, 2) + pow(ay, 2) + pow(az, 2));
		ax /= m;
		ay /= m;
		az /= m;

		//rotation matrix
		GLfloat transformation[4][4] = {
			{cos(deg)+(pow(ax, 2)*(1-cos(deg))), (ax*ay*(1-cos(deg)))-(az*sin(deg)), (az*ax*(1-cos(deg)))+(ay*sin(deg)), 0},
			{(ax*ay*(1-cos(deg)))+(az*sin(deg)), cos(deg)+(pow(ay, 2)*(1-cos(deg))), (ay*az*(1-cos(deg)))-(ax*sin(deg)), 0},
			{(az*ax*(1-cos(deg)))-(ay*sin(deg)), (ay*az*(1-cos(deg)))+(ax*sin(deg)), cos(deg)+(pow(az, 2)*(1-cos(deg))), 0},
			{0, 0, 0, 1}
		};

		return matrix_multiply(transformation, pt);
}

void rotate_camera(GLfloat deg)
{
	//rotate lookAt
	GLfloat * newAt = new GLfloat[4];
	newAt = rotate_arbitrary_axis(deg, my_cam.up[0], my_cam.up[1], my_cam.up[2], my_cam.pos[0], my_cam.pos[1], my_cam.pos[2], my_cam.at);
	my_cam.at[0] = newAt[0];
	my_cam.at[1] = newAt[1];
	my_cam.at[2] = newAt[2];
	my_cam.at[3] = 1;

	//update camera's dir
	my_cam.dir[0] = my_cam.at[0] - my_cam.pos[0];
	my_cam.dir[1] = my_cam.at[1] - my_cam.pos[1];
	my_cam.dir[2] = my_cam.at[2] - my_cam.pos[2];
	my_cam.dir[3] = 1;
	normalize(my_cam.dir);
}

void pitch_camera(GLfloat deg)
{
	//calculate axis to rotate LookAt about
	GLfloat pitch_axis[4];
	cross(my_cam.pos, my_cam.up, my_cam.dir, pitch_axis);
	normalize(pitch_axis);
	
	//rotate lookAt
	GLfloat * newVector = new GLfloat[4];
	newVector = rotate_arbitrary_axis(deg, pitch_axis[0], pitch_axis[1], pitch_axis[2], my_cam.pos[0], my_cam.pos[1], my_cam.pos[2], my_cam.at);
	my_cam.at[0] = newVector[0];
	my_cam.at[1] = newVector[1];
	my_cam.at[2] = newVector[2];
	my_cam.at[3] = 1;

	/*
	//rotate up vector
	newVector = rotate_arbitrary_axis(deg, pitch_axis[0], pitch_axis[1], pitch_axis[2], my_cam.pos[0], my_cam.pos[1], my_cam.pos[2], my_cam.up);
	my_cam.up[0] = newVector[0];
	my_cam.up[1] = newVector[1];
	my_cam.up[2] = newVector[2];
	my_cam.up[3] = 1;
	normalize(my_cam.up);
	*/

	//update camera's dir
	my_cam.dir[0] = my_cam.at[0] - my_cam.pos[0];
	my_cam.dir[1] = my_cam.at[1] - my_cam.pos[1];
	my_cam.dir[2] = my_cam.at[2] - my_cam.pos[2];
	my_cam.dir[3] = 1;
	normalize(my_cam.dir);
}

//transforms v by transform parameter and returns the resulting 4d vector
GLfloat* matrix_multiply(GLfloat transform[4][4], GLfloat v[4])
{
	GLfloat* result = new GLfloat[4];
	for(int i=0; i<4; i++)
		result[i] = (transform[i][0]*v[0]) + (transform[i][1]*v[1]) + (transform[i][2]*v[2]) + (transform[i][3]*v[3]);

	return result;
}

//multiples b by a and stores the result in c
void matrix_multiply(GLfloat a[4][4], GLfloat b[4][4], GLfloat c[4][4])
{
	GLfloat temp[4][4] = {0};

	for(int i=0; i<4; i++)  
			for(int j=0; j<4; j++)
			{
				temp[i][j] = 0;
				for(int k=0; k<4; k++)  
					temp[i][j] += a[i][k]*b[k][j];		
			}

	memcpy(c, temp, sizeof(temp));
}

void transpose_matrix(GLfloat m[4][4])
{
	for(int i=0; i<4; i++)
	{
		for(int j=i; j<4; j++)
		{
			if(i == j)
				continue;
			GLfloat temp = m[i][j];
			m[i][j] = m[j][i];
			m[j][i] = temp;
		}
	}
}

void my_keyboard( unsigned char key, int x, int y )
{
  switch( key )
  {
	  case 't':
		  //toggle axes
		  show_axes = (show_axes == 1) ? 0 : 1;
		  break;
	  case 'm':
		  break;
	  case 'd':
		  d_pressed = 1;
		  break;
	  case 'a':
		  a_pressed = 1;
		  break;
	  case 'w':
		  w_pressed = 1;
		break;
	  case 's':
		  s_pressed = 1;
		  break;
	  case ' ':
		crt_render_mode = crt_render_mode == GL_POLYGON ? GL_LINE_LOOP : GL_POLYGON;
		glutPostRedisplay();
		break;
	  case 'r':
		  if(recording)
		  {
			output.close();
			recording = 0;
		  }
		  else
			  recording = 1;
		  start_time = GetTickCount();
		  break;
	  case 'q': 
	  case 'Q':
		exit(0) ;
		break ;	
	  default: break;
  }
  
  return ;
}

void my_keyboard_up( unsigned char key, int x, int y )
{
	switch( key )
	{
		case 'w':
			w_pressed = 0;
			break;
		case 's':
			s_pressed = 0;
			break;
		case 'a':
			a_pressed = 0;
			break;
		case 'd':
			d_pressed = 0;
			break;
	}
}

//function updates the scene based on current keyboard state
void update_and_display(int id)
{
	//calculate movement factor based on how many keys are pressed + movement_speed
	GLfloat mweight = 0;
	if((a_pressed - d_pressed) != 0)
		mweight++;
	if((w_pressed - s_pressed) != 0)
		mweight++;
	if((lm_pressed - rm_pressed) != 0)
		mweight++;

	if(mweight != 0)
		mweight = (1/mweight);
	mweight *= movement_speed;

	if(w_pressed)
	{
		//move camera forward
		my_cam.pos[0]+= my_cam.dir[0] * mweight;
		my_cam.pos[2]+= my_cam.dir[2] * mweight;
		my_cam.at[0]+= my_cam.dir[0] * mweight;
		my_cam.at[2]+= my_cam.dir[2] * mweight;
	}

	if(s_pressed)
	{
		//move camera back
		my_cam.pos[0]-= my_cam.dir[0] * mweight;
		my_cam.pos[2]-= my_cam.dir[2] * mweight;
		my_cam.at[0]-= my_cam.dir[0] * mweight;
		my_cam.at[2]-= my_cam.dir[2] * mweight;
	}

	if(a_pressed)
	{
		//calculate axis to move along left
		GLfloat pitch_axis[4];
		cross(my_cam.pos, my_cam.up, my_cam.dir, pitch_axis);
		normalize(pitch_axis);

		//move camera left
		my_cam.pos[0]+= pitch_axis[0] * mweight;
		my_cam.pos[2]+= pitch_axis[2] * mweight;
		my_cam.at[0]+= pitch_axis[0] * mweight;
		my_cam.at[2]+= pitch_axis[2] * mweight;
	}

	if(d_pressed)
	{
		//calculate axis to move along right
		GLfloat pitch_axis[4];
		cross(my_cam.pos, my_cam.dir, my_cam.up, pitch_axis);
		normalize(pitch_axis);

		//move camera left
		my_cam.pos[0]+= pitch_axis[0] * mweight;
		my_cam.pos[2]+= pitch_axis[2] * mweight;
		my_cam.at[0]+= pitch_axis[0] * mweight;
		my_cam.at[2]+= pitch_axis[2] * mweight;
	}

	if(lm_pressed)
	{
		my_cam.pos[1] += my_cam.up[1] * mweight;
		my_cam.at[1] += my_cam.up[1] * mweight;
	}

	if(rm_pressed)
	{
		my_cam.pos[1] -= my_cam.up[1] * mweight;
		my_cam.at[1] -= my_cam.up[1] * mweight;
	}

	//if recording and camera has just moved
	if(recording && (w_pressed || s_pressed || a_pressed || d_pressed || lm_pressed || rm_pressed))
	{
		current_time = GetTickCount();
		double seconds = (current_time - start_time) / 1000;
		start_time = current_time;

		output << "(" << my_cam.pos[0] << " " << my_cam.pos[1] << " " << my_cam.pos [2] << ") " 
			<< "(" << my_cam.at[0] << " " << my_cam.at[1] << " " << my_cam.at[2] << ") "
			<< "(" << my_cam.up[0] << " " << my_cam.up[1] << " " << my_cam.up[2] << ") "
			<< "(" << max(1, floor(seconds*fps)) << ")\n";
	}

	//set another timer function to update and display player, camera, and scene
	glutTimerFunc(1.0/fps*1000, update_and_display, 0);
}

void my_mouse_drag(int x, int y) {
    static bool just_warped = false;
	if(just_warped)
	{
		just_warped = false;
		return;
	}

    int dx = WINDOW_WIDTH/2 - x;
    int dy = y - WINDOW_HEIGHT/2;

    if(dx) 
	{
		rotate_camera(rotation_speed * ((dx > 0) - (dx < 0)));
		glutPostRedisplay();
    }

    if(dy)
	{
		pitch_camera(rotation_speed * ((dy > 0) - (dy < 0)));
		glutPostRedisplay();
    }

	//if recording and camera has just moved
	if(recording && (dx || dy))
	{
		current_time = GetTickCount();
		double seconds = (current_time - start_time) / 1000;
		start_time = current_time;
		
		output << "(" << my_cam.pos[0] << " " << my_cam.pos[1] << " " << my_cam.pos [2] << ") " 
			<< "(" << my_cam.at[0] << " " << my_cam.at[1] << " " << my_cam.at[2] << ") "
			<< "(" << my_cam.up[0] << " " << my_cam.up[1] << " " << my_cam.up[2] << ") "
			<< "(" << max(1, floor(seconds*fps)) << ")\n";
	}
	
    glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

    just_warped = true;
}


void my_mouse(int button, int state, int mousex, int mousey) {

  switch( button ) {
	
  case GLUT_LEFT_BUTTON:
    if( state == GLUT_DOWN ) {
		lm_pressed = 1;
	}
    
    if( state == GLUT_UP ) {
		lm_pressed = 0;
    }
    break ;

  case GLUT_RIGHT_BUTTON:
    if ( state == GLUT_DOWN ) {
		rm_pressed = 1;
    }
    
    if( state == GLUT_UP ) {
		rm_pressed = 0;
    }
    break ;
  }
  

  
  return ;
}

float dotprod(float v1[], float v2[]) {
	float dot_product = 0;
	for(int i=0; i<3; i++)
		dot_product += v1[i] * v2[i];

	return dot_product;
}


void normalize(GLfloat *p) { 
  double d=0.0;
  int i;
  for(i=0; i<3; i++) d+=p[i]*p[i];
  d=sqrt(d);
  if(d > 0.0) for(i=0; i<3; i++) p[i]/=d;
}


void cross(GLfloat *a, GLfloat *b, GLfloat *c, GLfloat *d) { 
  d[0]=(b[1]-a[1])*(c[2]-a[2])-(b[2]-a[2])*(c[1]-a[1]);
  d[1]=(b[2]-a[2])*(c[0]-a[0])-(b[0]-a[0])*(c[2]-a[2]);
  d[2]=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);
  normalize(d);
}

void print_matrix(float my_matrix[])
{ 
  int i, j;

  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      printf ("%f ", my_matrix[i+j*4]);
    }
    printf ("\n");
  }
  printf ("\n");
}

void make_quad(GLfloat vertices[][3]) {
  glBegin(GL_POLYGON); 
  {
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);
  }
  glEnd();
}

void make_cube_smart(OBJECT *po, double size){
  po->sid = 1;

  int i;  
  // compute verts on PI/4 angels for x y z, then -x y z  
  for(i = 0; i < 4; i++){
    po->vertices_cube_smart[i][0] = size*cos((M_PI/4));
    po->vertices_cube_smart[i][1] = -size*sin(i*(M_PI/2)+(M_PI/4));
    po->vertices_cube_smart[i][2] = size*cos(i*(M_PI/2)+(M_PI/4));
    po->vertices_cube_smart[i][3] = 1;
    // mirror on x axis
    po->vertices_cube_smart[i+4][0] = -size*cos((M_PI/4));
    po->vertices_cube_smart[i+4][1] = -size*sin(i*(M_PI/2)+(M_PI/4));
    po->vertices_cube_smart[i+4][2] = size*cos(i*(M_PI/2)+(M_PI/4));
    po->vertices_cube_smart[i+4][3] = 1;
  }

  //compute normals
    cross(po->vertices_cube_smart[0], po->vertices_cube_smart[1], po->vertices_cube_smart[2], po->normals_cube_smart[0]);
    cross(po->vertices_cube_smart[1], po->vertices_cube_smart[5], po->vertices_cube_smart[6], po->normals_cube_smart[1]);
    cross(po->vertices_cube_smart[5], po->vertices_cube_smart[4], po->vertices_cube_smart[7], po->normals_cube_smart[2]);
    cross(po->vertices_cube_smart[4], po->vertices_cube_smart[0], po->vertices_cube_smart[3], po->normals_cube_smart[3]);
}

void make_house(OBJECT *po)
{
	po->sid = 2;
	memcpy(po->vertices_house, vertices_house, sizeof(vertices_house));
}

void make_sphere(OBJECT *po, double ray)
{
	po->sid = 3;

	for(int i=0; i<crt_rs; i++)
	{
		for(int j=0; j<crt_vs; j++)
		{
			double theta = i*2*M_PI/crt_rs;
			double phi = (-1*M_PI/2) + (j*M_PI/(crt_vs-1));

			po->vertices_obj[i][j][0] = ray * sin(theta) * cos(phi);
			po->vertices_obj[i][j][1] = ray * sin(phi);
			po->vertices_obj[i][j][2] = ray * cos(theta) * cos(phi);
			po->vertices_obj[i][j][3] = 1;
		}
	}
}

void make_cylinder(OBJECT *po, double height, double ray)
{
	po->sid = 4;

	for(int i=0; i<crt_rs; i++)
	{
		for(int j=0; j<crt_vs; j++)
		{
			po->vertices_obj[i][j][0] = ray * cos(i*2*M_PI/crt_rs);
			po->vertices_obj[i][j][1] = (height/2) - j*height/(crt_vs-1);
			po->vertices_obj[i][j][2] = ray * sin(i*2*M_PI/crt_rs);
			po->vertices_obj[i][j][3] = 1;
		}
	}
}

void make_cone(OBJECT *po, double r, double height)
{
	po->sid = 5;

	for(int i=0; i<crt_rs; i++)
	{
		for(int j=0; j<crt_vs; j++)
		{
			double theta = i*2*M_PI/crt_rs;

			po->vertices_obj[i][j][0] = (j*r/(crt_vs-1)) * cos(theta);
			po->vertices_obj[i][j][1] = (-1*height/2) + (j*height/(crt_vs-1));
			po->vertices_obj[i][j][2] = (j*r/(crt_vs-1)) * sin(theta);
			po->vertices_obj[i][j][3] = 1;
		}
	}
}

void make_torus(OBJECT *po, double r1, double r2)
{
	po->sid = 6;

	for(int i=0; i<crt_rs; i++)
	{
		for(int j=0; j<crt_vs; j++)
		{
			double theta = i*2*M_PI/crt_rs;
			double phi = j*2*M_PI/crt_vs;

			po->vertices_obj[i][j][0] = (r1 + r2*cos(phi)) * cos(theta);
			po->vertices_obj[i][j][1] = r2 * sin(phi);
			po->vertices_obj[i][j][2] = (r1 + r2*cos(phi)) * sin(theta);
			po->vertices_obj[i][j][3] = 1;
		}
	}
}

void calculate_normals(OBJECT *po)
{
	GLfloat count[50][50];

	for(int i=0; i<crt_rs; i++)
		for(int j=0; j<crt_vs; j++)
		{
			po->normals_obj[i][j][0] = 0;
			po->normals_obj[i][j][1] = 0;
			po->normals_obj[i][j][2] = 0;

			count[i][j] = 0;
		}

	for(int i=0; i<crt_rs; i++)
	{
		for(int j=0; j<crt_vs; j++)
		{
			int iplus = i+1 == crt_rs ? 0 : i+1;
			int jplus = j+1 == crt_vs ? 0 : j+1;
			
			Vector normal1 = {0,0,0,1};
			cross(po->vertices_obj[i][j], po->vertices_obj[iplus][j], po->vertices_obj[i][jplus], normal1);
			po->normals_obj[i][j][0] += normal1[0]; 
			po->normals_obj[i][j][1] += normal1[1];
			po->normals_obj[i][j][2] += normal1[2];
			count[i][j]++;
			
			po->normals_obj[iplus][i][0] += normal1[0]; 
			po->normals_obj[iplus][i][1] += normal1[1];
			po->normals_obj[iplus][i][2] += normal1[2]; 
			count[iplus][j]++;

			po->normals_obj[iplus][jplus][0] += normal1[0];
			po->normals_obj[iplus][jplus][1] += normal1[1];
			po->normals_obj[iplus][jplus][2] += normal1[2];
			count[iplus][jplus]++;
			
			po->normals_obj[i][jplus][0] += normal1[0]; 
			po->normals_obj[i][jplus][1] += normal1[1];
			po->normals_obj[i][jplus][2] += normal1[2]; 
			count[i][jplus]++;
		}
	}
	for(int i = 0; i < crt_rs; i++)
		for(int j = 0; j<crt_vs; j++)
		{
			po->normals_obj[i][j][0] /= count[i][j];
			po->normals_obj[i][j][1] /= count[i][j];
			po->normals_obj[i][j][2] /= count[i][j];
			normalize(po->normals_obj[i][j]);
		}
}

double get_length(GLfloat v[4])
{
	return sqrt((v[0]*v[0]) + (v[1]*v[1]) + (v[2]*v[2]));
}

void draw_triangle(GLfloat vertices[][4], int iv1, int iv2, int iv3, int ic) {
  glBegin(crt_render_mode); 
  {
    glColor3fv(colors[ic]);
    /*note the explicit use of homogeneous coords below: glVertex4f*/
    glVertex4fv(vertices[iv1]);
    glVertex4fv(vertices[iv2]);
    glVertex4fv(vertices[iv3]);
  }
  glEnd();
}

void draw_quad(GLfloat vertices[][4], GLfloat *normals, int iv1, int iv2, int iv3, int iv4, int ic) {
  glBegin(crt_render_mode); 
  {
    glColor3fv(colors[ic]);
    glNormal3fv(normals);
    /*note the explicit use of homogeneous coords below: glVertex4f*/
    glVertex4fv(vertices[iv1]);
    glVertex4fv(vertices[iv2]);
    glVertex4fv(vertices[iv3]);
    glVertex4fv(vertices[iv4]);
  }
  glEnd();
}

void draw_quad(GLfloat vertices[][4], GLfloat *normals, int iv1, int iv2, int iv3, int iv4, const GLfloat* colors) {
  glBegin(crt_render_mode); 
  {
    glColor3fv(colors);
    glNormal3fv(normals);
    /*note the explicit use of homogeneous coords below: glVertex4f*/
    glVertex4fv(vertices[iv1]);
    glVertex4fv(vertices[iv2]);
    glVertex4fv(vertices[iv3]);
    glVertex4fv(vertices[iv4]);
  }
  glEnd();
}

/***********************************
 FUNCTION: draw_cube_smart() 
 ARGS: none
 RETURN: none
 DOES: draws a cube from quads
************************************/
void draw_cube_smart(OBJECT *po)
{
  /* sides */
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[0],0,1,2,3, po->amb);
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[1],1,5,6,2, po->amb);
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[2],5,4,7,6, po->amb);
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[3],4,0,3,7, po->amb);
    
  /* top and bottom */
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[3],3,2,6,7, po->amb);
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[3],0,1,5,4, po->amb);
}

/***********************************
 FUNCTION: draw_axes
 ARGS: none
 RETURN: none
 DOES: draws main X, Y, Z axes
************************************/
void draw_axes( void ) {
  glLineWidth( 5.0 );

  glDisable(GL_LIGHTING);

  glBegin(GL_LINES); 
  {
    glColor3fv(colors[1]);
    glVertex4fv(vertices_axes[0]);
    glVertex4fv(vertices_axes[1]);
		
    glColor3fv(colors[4]);
    glVertex4fv(vertices_axes[0]);
    glVertex4fv(vertices_axes[2]);
		
    glColor3fv(colors[6]);
    glVertex4fv(vertices_axes[0]);
    glVertex4fv(vertices_axes[3]);
  }
  glEnd();
  glLineWidth( 1.0 );

  glEnable(GL_LIGHTING);
	
}

void draw_objects() {
  int i;
  for(i=0; i<num_objects; i++){
    OBJECT *cur;
    cur = &my_objects[i];

    glMaterialfv(GL_FRONT, GL_AMBIENT, cur->amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, cur->diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, cur->spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, &cur->shine);
    //glMaterialfv(GL_FRONT, GL_EMISSION, cur->emi);

    switch(cur->sid){
		case 1: //cube
			draw_cube_smart(cur);
			break;
		case 2: //house
			draw_house(cur);
			break;
		case 3: //sphere
		case 4: //cylinder
		case 5: //cones
		case 6: //torus
			draw_shape(cur);
			break;
		default: break;
    }
  }
}

void draw_house(OBJECT *po)
{
	draw_triangle(po->vertices_house, 0,1,2,RED);
	draw_triangle(po->vertices_house, 0,2,3,GREEN);
	draw_triangle(po->vertices_house, 0,3,4,WHITE);
	draw_triangle(po->vertices_house, 0,4,1,GREY);
  
	draw_triangle(po->vertices_house, 2,1,5, BLUE);
	draw_triangle(po->vertices_house, 5,6,2, BLUE);
	draw_triangle(po->vertices_house, 2,6,3, CYAN);
	draw_triangle(po->vertices_house, 3,6,7, CYAN);
	draw_triangle(po->vertices_house, 3,7,8, YELLOW);
	draw_triangle(po->vertices_house, 8,3,4, YELLOW);
	draw_triangle(po->vertices_house, 4,8,1, MAGENTA);
	draw_triangle(po->vertices_house, 1,8,5, MAGENTA);
}

void draw_shape(OBJECT *po)
{
	for(int i=0; i<crt_rs; i++)
	{
		for(int j=0; j<crt_vs; j++)
		{
			int iplus = i+1 == crt_rs ? 0 : i+1;
			int jplus = j+1 == crt_vs ? 0 : j+1;
			
			//draw
			glBegin(crt_render_mode);
			glColor3fv(po->amb);
			glNormal3fv(po->normals_obj[i][j]);
			glVertex4fv(po->vertices_obj[i][j]);

			glNormal3fv(po->normals_obj[iplus][j]);
			glVertex4fv(po->vertices_obj[iplus][j]);

			glNormal3fv(po->normals_obj[iplus][jplus]);
			glVertex4fv(po->vertices_obj[iplus][jplus]);

			glNormal3fv(po->normals_obj[i][jplus]);
			glVertex4fv(po->vertices_obj[i][jplus]);
			glEnd();
		}
	}
}

void my_display() {
  // clear all pixels, reset depth 
  glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT );
  
  glLoadIdentity();

  gluLookAt(my_cam.pos[0],my_cam.pos[1], my_cam.pos[2],
	    my_cam.at[0],my_cam.at[1],my_cam.at[2],
	    my_cam.up[0], my_cam.up[1], my_cam.up[2]);
  
  //draw the axes and objects
  if(show_axes)
	draw_axes();
  draw_objects();

  // this buffer is ready
  glutSwapBuffers();
}

void my_idle(void) {
  //EC idea: Make the flashlight flicker a bit (random flicker strength) when the user is idle.
	
	glutPostRedisplay();
  return ;
}