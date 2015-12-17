#include "moviemaker.h"

int finished = 0;

void myabort(void) {
  abort();
} 

void plotPixel(color_t *screen, int x, int y, float r, float g, float b, float a)
{
	screen[y*WINDOW_WIDTH + x].r = r;
	screen[y*WINDOW_WIDTH + x].g = g;
	screen[y*WINDOW_WIDTH + x].b = b;
	screen[y*WINDOW_WIDTH + x].a = a;
}

//linear algebra functions
void normalize(GLfloat *p) { 
  double d=0.0;
  int i;
  for(i=0; i<3; i++) d+=p[i]*p[i];
    d=sqrt(d);
  if(d > 0.0) for(i=0; i<3; i++) p[i]/=d;
}

GLfloat dotprod(Vector v1, Vector v2, int size) {
  float tot = 0;
  int i;
  for (i=0; i<size; i++)
    tot += v1[i]*v2[i];
  return tot;
}

void matrix_multiply(Matrix m, Vector v, Vector p) {
  Vector tempv;
  memcpy (tempv, v, 16);
	for(int i=0; i<4; i++){
    Vector tempm = {m[0][i],m[1][i],m[2][i],m[3][i]};
		p[i] = dotprod(tempv, tempm, 4);
	}
}

void matrix_multiply_four(Matrix m, Matrix n, Matrix p) {
  Matrix tempm, tempn;
  memcpy (tempm, m, 64);
  memcpy (tempn, n, 64);
	for(int i=0; i<4; i++){
		for(int j=0; j<4;j++){
			Vector tempv = {tempm[0][i], tempm[1][i], tempm[2][i], tempm[3][i]};
      p[j][i] = dotprod(tempn[j],tempv, 4);
		}
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  num_objects = num_lights = 0;
	my_assert(argc > 1, "need to supply a spec file");
	read_spec(argv[1]);

	my_assert(argc > 2, "need to supply an mcf file");
	parse_mcf(argv[2]);

	glutCreateWindow("Ray Tracer");

	initGL();

	glutDisplayFunc(displayScene);
	glutReshapeFunc(resizeWindow);
	glutIdleFunc(idle);

	glutMainLoop();

	return 0;
}

//code for reading spec files
void parse_floats(char *buffer, GLfloat nums[]) {
  int i;
  char *ps;

  ps = strtok(buffer, " ");
  for (i=0; ps; i++) {
    nums[i] = atof(ps);
    ps = strtok(NULL, " ");
  }
}

void parse_mcf(char * file)
{
	char buffer[300];
	FILE *fp;

	//read file
	fp = fopen(file, "r");
	my_assert(fp, "can't open mcf");
	while(!feof(fp))
	{
		fgets(buffer, 300, fp);
		if(buffer[0] != '(')
			break;
		char *pos, *at, *up, *frames;

		pos = strtok(buffer, "()"); strtok(NULL, "()");
		at = strtok(NULL, "()"); strtok(NULL, "()");
		up = strtok(NULL, "()"); strtok(NULL, "()");
		frames = strtok(NULL, "()"); strtok(NULL, "()");

		parse_floats(pos, fd[cur_index].pos);
		parse_floats(at, fd[cur_index].at);
		parse_floats(up, fd[cur_index].up);
		fd[cur_index].frames = atoi(frames);
		
		frame_count += fd[cur_index].frames;
		cur_index++;
	}
	data_count = cur_index;
	cur_index = 0;

	//load camera
	my_cam.pos[0] = fd[cur_index].pos[0];
	my_cam.pos[1] = fd[cur_index].pos[1];
	my_cam.pos[2] = fd[cur_index].pos[2];
	my_cam.at[0] = fd[cur_index].at[0];
	my_cam.at[1] = fd[cur_index].at[1];
	my_cam.at[2] = fd[cur_index].at[2];
	my_cam.up[0] = fd[cur_index].up[0];
	my_cam.up[1] = fd[cur_index].up[1];
	my_cam.up[2] = fd[cur_index].up[2];
	
}

void parse_obj(char *buffer){
  OBJECT *po;
  char *pshape, *pshine, *pemi, *pamb, *pdiff, *pspec, *ptranslate, *pscale, *protate;

  my_assert ((num_objects < NUM_OBJECTS), "too many objects");
  po = &my_objects[num_objects++];

  pshape  = strtok(buffer, " ");

  ptranslate    = strtok(NULL, "()");  strtok(NULL, "()");
  pscale        = strtok(NULL, "()");  strtok(NULL, "()"); 
  protate       = strtok(NULL, "()");  strtok(NULL, "()");  

  pshine  = strtok(NULL, "()");strtok(NULL, "()");
 
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

  Vector origin = {0,0,0,1};
  memcpy(po->center,origin,16);
  memcpy(po->ctm,identity,64);
  real_scaling(po->ctm, po->scale[0], po->scale[1], po->scale[2]);  
  real_rotation(po->ctm, po->rotate[0], 1, 0, 0);
  real_rotation(po->ctm, po->rotate[1], 0, 1, 0);
  real_rotation(po->ctm, po->rotate[2], 0, 0, 1);
  real_translation(po->ctm, po->translate[0], po->translate[1], po->translate[2]);
  matrix_multiply(po->ctm,po->center,po->center);

}

/*
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
}*/

void parse_light(char *buffer){
  LITE *pl;
  char *pamb, *pdiff, *pspec, *ppos, *pdir, *pang;
  my_assert ((num_lights < MAX_LIGHTS), "too many lights");
  pl = &my_lights[num_lights++];

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
  }
  else
    pl->dir[0]= pl->dir[1]= pl->dir[2] =0;
}

void read_spec(char *fname) {
  char buffer[300];
  FILE *fp;

  fp = fopen(fname, "r");
  my_assert(fp, "can't open spec");
  while(!feof(fp)){
    fgets(buffer, 300, fp);
    switch (buffer[0]) {
    case '#':
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
	    parse_obj(buffer);
 	    break;

    case 'l':
      parse_light(buffer);
      break;

    /*case 'c':
      parse_camera(buffer);
      break;*/

    default:
      break;
    }
  }
}

void initGL()
{
	int i, j;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);

	resizeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	// erase texture
	for (i=0;i<sizeof(screen)/sizeof(color_t);i++)
	{
		screen[i].r = 0.0f;
		screen[i].g = 0.0f;
		screen[i].b = 0.0f;
		screen[i].a = 1.0f;
	}

	// put 3 red squares for "loading"
	for (i=0; i < 32; i++)
	{
		for (j=0; j<32;j++)
		{
			plotPixel(screen, i+176, j+240, 1.0f, 0.0f, 0.0f, 1.0f);
			plotPixel(screen, i+240, j+240, 1.0f, 0.0f, 0.0f, 1.0f);
			plotPixel(screen, i+306, j+240, 1.0f, 0.0f, 0.0f, 1.0f);
		}
	}


	// create texture for drawing
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, (void*)screen);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void displayScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId);

	// load orthographic projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(1.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
	glEnd();

	// restore projection
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glutSwapBuffers();
}

//transformation functions
void real_translation(Matrix m, GLfloat x, GLfloat y, GLfloat z) {
  Matrix t = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{x,y,z,1}};
  matrix_multiply_four(t,m,m);
}

void real_scaling(Matrix m, GLfloat sx, GLfloat sy, GLfloat sz) {
	Matrix t = {{sx,0,0,0},{0,sy,0,0},{0,0,sz,0},{0,0,0,1}};
	matrix_multiply_four(t,m,m);
}

void real_rotation(Matrix m, GLfloat deg, GLfloat x, GLfloat y, GLfloat z) {
  Vector u = {x,y,z,1};
  normalize(u);
	GLfloat ux = u[0];
	GLfloat uy = u[1];
	GLfloat uz = u[2];
	GLfloat x2 = (ux*ux);
	GLfloat y2 = (uy*uy);
	GLfloat z2 = (uz*uz);
	GLfloat c = cos(deg * M_PI / 180.0);
	GLfloat s = sin(deg * M_PI / 180.0);
	Matrix t = {{x2 + c*(1-x2),ux*uy*(1-c)+(uz*s),uz*ux*(1-c)-uy*s,0},
	            {ux*uy*(1-c)-uz*s,y2+c*(1-y2),uy*uz*(1-c)+ux*s,0},
	            {uz*ux*(1-c)+uy*s,uy*uz*(1-c) - ux*s,z2+c*(1-z2),0},
              {0,0,0,1}};
	matrix_multiply_four(t,m,m);
}

//generate direction vector for ray at screen position x, y
void generateRay(GLfloat x, GLfloat y, Vector d){
    GLdouble nearx, neary, nearz, farx, fary, farz = 0;
    GLdouble modelViewMatrix[16], projectionMatrix[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluUnProject(my_cam.pos[0],my_cam.pos[1],my_cam.pos[2],modelViewMatrix,projectionMatrix,viewport,&nearx,&neary,&nearz);
    gluUnProject(x,y,1.0,modelViewMatrix,projectionMatrix,viewport,&farx,&fary,&farz);
    d[0] = farx-nearx;
    d[1] = fary-neary;
    d[2] = farz-nearz;
    d[3] = 0;
    normalize(d);
}

//individual shape intersection algorithms
int intersect_sphere(OBJECT* po, Vector p, Vector d, GLfloat* points){
    int ret = 0;
    GLfloat a = dotprod(d,d,3);
    GLfloat b = 2*dotprod(p,d,3);
    GLfloat c = dotprod(p,p,3) -1;
    GLfloat det = ((b*b) - (4*a*c));
    GLfloat t1 = (-b)/(2*a) + sqrt(det)/(2*a);
    GLfloat t2 = (-b)/(2*a) - sqrt(det)/(2*a);
    
    if (det < 0){
        ret = 0;
    }
    else if (det == 0){
        points[0] = t1;
        ret = 1;
    }
    else if (det > 0){
        points[0] = t1;
        points[1] = t2;
        ret = 2;
    }
    return ret;
}

int intersect_cube(OBJECT* po, Vector p, Vector d, GLfloat* points){
    int ret = 0;
    GLfloat t,x,y,z;
    if(d[0] != 0){
        //left
        t = (-0.707106769 - p[0])/d[0];
        y = p[1] + d[1]*t;
        z = p[2] + d[2]*t;
        if(y<=0.707106769 && y>=-0.707106769 && z<=0.707106769 && z>=-0.707106769){
            points[ret] = t;
            ret++;
        }
        //right
        t = (0.707106769 - p[0])/d[0];
        y = p[1] + d[1]*t;
        z = p[2] + d[2]*t;
        if(y<=0.707106769 && y>=-0.707106769 && z<=0.707106769 && z>=-0.707106769){
            points[ret] = t;
            ret++;
        }
    }
    if(d[1] != 0){
        //bottom
        t = (-0.707106769 - p[1])/d[1];
        x = p[0] + d[0]*t;
        z = p[2] + d[2]*t;
        if(x<=0.707106769 && x>=-0.707106769 && z<=0.707106769 && z>=-0.707106769){
            points[ret] = t;
            ret++;
        }
        //top
        t = (0.707106769 - p[1])/d[1];
        x = p[0] + d[0]*t;
        z = p[2] + d[2]*t;
      if(x<=0.707106769 && x>=-0.707106769 && z<=0.707106769 && z>=-0.707106769){
            points[ret] = t;
            ret++;
        }
    }
    if(d[2] != 0){
        //back
        t = (-0.707106769 - p[2])/d[2];
        x = p[0] + d[0]*t;
        y = p[1] + d[1]*t;
        if(x<=0.707106769 && x>=-0.707106769 && y<=0.707106769 && y>=-0.707106769){
            points[ret] = t;
            ret++;
        }
        //front
        t = (0.707106769 - p[2])/d[2];
        x = p[0] + d[0]*t;
        y = p[1] + d[1]*t;
        if(x<=0.707106769 && x>=-0.707106769 && y<=0.707106769 && y>=-0.707106769){
            points[ret] = t;
            ret++;
        }
    }
    return ret;
}

int intersect_cylinder(OBJECT* po, Vector p, Vector d, GLfloat* points){
    int ret = 0;
    GLfloat a = (d[0] * d[0]) + (d[2]*d[2]);
    GLfloat b = 2*(p[0]*d[0] + p[2]*d[2]);
    GLfloat c = p[0]*p[0] + p[2]*p[2] -1;
    GLfloat det = ((b*b) - (4*a*c));
    GLfloat t1 = (-b)/(2*a) + sqrt(det)/(2*a);
    GLfloat t2 = (-b)/(2*a) - sqrt(det)/(2*a);
    GLfloat y1 = p[1] + t1*d[1];
    GLfloat y2 = p[1] + t2*d[1];
    
   if (det == 0 && y1 <= 0.5 && y1 >= -0.5){
        points[ret] = t1;
        ret++;
    }
    else if (det > 0){
      if(y1 <= 0.5 && y1 >= -0.5){
        points[ret] = t1;
        ret++;
      }
      if(y2 <= 0.5 && y2 >= -0.5){
        points[ret] = t2;
        ret++;
      }
    }

    GLfloat t3 = (0.5-p[1])/d[1];
    GLfloat t4 = (-0.5-p[1])/d[1];
    GLfloat x3 = p[0] + t3*d[0];
    GLfloat x4 = p[0] + t4*d[0];
    GLfloat z3 = p[2] + t3*d[2];
    GLfloat z4 = p[2] + t4*d[2];

    if(x3*x3 + z3*z3 <= 1){
        points[ret] = t3;
        ret++;
    }

    if(x4*x4 + z4*z4 <= 1){
        points[ret] = t4;
        ret++;
    }
    return ret;
}

//find intersection points
int find_intersects(Vector pv, Vector dv, Vector* v, int* obj){
    int num_intersects = 0;
    Vector all_intersects[NUM_OBJECTS];
    int intersect_object[NUM_OBJECTS];
    for(int i=0; i<num_objects; i++){

        OBJECT *cur;
        cur = &my_objects[i];

        //transform ray to object coordinate space
        Vector p,d;
        memcpy(p,pv,16);
        memcpy(d,dv,16);
        Matrix obj_local, dir_local;
        memcpy(obj_local,identity,64);
        memcpy(dir_local,identity,64);
        real_translation(obj_local,-cur->translate[0],-cur->translate[1],-cur->translate[2]);
        real_rotation(obj_local,-cur->rotate[2],0,0,1);
        real_rotation(obj_local,-cur->rotate[1],0,1,0);
        real_rotation(obj_local,-cur->rotate[0],1,0,0);
        real_scaling(obj_local,1/cur->scale[0],1/cur->scale[1],1/cur->scale[2]);
        matrix_multiply(obj_local,p,p);
        real_rotation(dir_local,-cur->rotate[2],0,0,1);
        real_rotation(dir_local,-cur->rotate[1],0,1,0);
        real_rotation(dir_local,-cur->rotate[0],1,0,0);
        real_scaling(dir_local,1/cur->scale[0],1/cur->scale[1],1/cur->scale[2]);
        matrix_multiply(dir_local,d,d);

        GLfloat point[8];
        int num = 0;
        //compute intersection per shape
        switch(cur->sid){
  	    case CUBE:
    	      num = intersect_cube(cur,p,d,point);
    	      break;
  	    case SPHERE:
    	      num = intersect_sphere(cur,p,d,point);
    	      break;
        case CYLINDER:
            num = intersect_cylinder(cur,p,d,point);
            break;
  	    default: break;
        }

        if(num != 0){
            for(int j=0; j<num; j++){
                Vector retPoint;

                //calculate intersection point
                retPoint[0] = p[0] + point[j]*d[0];
                retPoint[1] = p[1] + point[j]*d[1];
                retPoint[2] = p[2] + point[j]*d[2];
                retPoint[3] = 1;

                //transform intersection point to world coordinates
                memcpy(obj_local,identity,64);
                real_scaling(obj_local,cur->scale[0],cur->scale[1],cur->scale[2]);
                real_rotation(obj_local,cur->rotate[0],1,0,0);
                real_rotation(obj_local,cur->rotate[1],0,1,0);
                real_rotation(obj_local,cur->rotate[2],0,0,1);
                real_translation(obj_local,cur->translate[0],cur->translate[1],cur->translate[2]);
                matrix_multiply(obj_local,retPoint,retPoint);

                memcpy(all_intersects[num_intersects],retPoint,16);
                intersect_object[num_intersects] = i;
                num_intersects++;
            }
        }
  }

  if(num_intersects <= 0){
    return 0;
  }
  else{
      memcpy(v,all_intersects,16 * num_intersects);
      memcpy(obj,intersect_object,4 * num_intersects);
      return num_intersects;
  }
}

void trace(float rx, float ry, Vector rgba){

    //declare variables
    float r, g, b, a;
    Vector *intersects;
    int *obj;
    int inters;
    Vector d,v = {0,0,0,0};
    Vector p = {my_cam.pos[0], my_cam.pos[1], my_cam.pos[2], 1};

    //initialize variables
    r = 0;
    g = 0;
    b = 0;
    a = 0;
    intersects = new Vector[num_objects * 2];
    obj = new int[num_objects * 2];

    generateRay(rx,ry,d);
    
    inters = find_intersects(p,d,intersects,obj);

		if (inters != 0) {
      //find closest intersection
      GLfloat dist = FLT_MAX;
      int index = -1;
      for(int k=0; k<inters; k++){
          GLfloat x = intersects[k][0] - p[0];
          GLfloat y = intersects[k][1] - p[1];
          GLfloat z = intersects[k][2] - p[2];
          GLfloat newDist = sqrt(x*x + y*y + z*z);
          if(newDist<dist){
              dist = newDist;
              index = k;
          }
      }  

        //set v = intersection point
        memcpy(v,intersects[index],16);

        //set po = to that object
        OBJECT* po = &my_objects[obj[index]];
        Vector n,l,e,f = {0,0,0,0};

        // surface normal n
        n[0] = v[0] - po->center[0];
        n[1] = v[1] - po->center[1];
        n[2] = v[2] - po->center[2];
        n[3] = 0;
        normalize(n);

        // vector to eye e
        e[0] = my_cam.pos[0] - v[0];
        e[1] = my_cam.pos[1] - v[1];
        e[2] = my_cam.pos[2] - v[2];
        e[3] = 0;
        normalize(e);

        //calculate input of each light
        for(int i = 0; i<num_lights; i++){
            LITE *cur = &my_lights[i];
            Vector temp_p,temp_d,temp_v;
            Vector *temp_i;
            temp_i = new Vector[num_objects*2];
            int *temp_o;
            temp_o = new int[num_objects*2];

            //vector to light source l
            l[0] = (cur->pos[0] - v[0]);
            l[1] = (cur->pos[1] - v[1]);
            l[2] = (cur->pos[2] - v[2]);
            l[3] = 0;
            normalize(l);

            //direction of ray from light to intersection
            temp_d[0] = -l[0];
            temp_d[1] = -l[1];
            temp_d[2] = -l[2];
            temp_d[3] = 0;

            //position of ray from light
            temp_p[0] = cur->pos[0];
            temp_p[1] = cur->pos[1];
            temp_p[2] = cur->pos[2];
            temp_p[3] = 1;
            
            int obs = find_intersects(temp_p,temp_d,temp_i,temp_o);

            //set boudning box for shadow-casting objects
            GLfloat max_x, max_y, max_z, min_x, min_y, min_z;
            if(temp_p[0] < v[0]){
                max_x = v[0];
                min_x = temp_p[0];
            }
            else {
                min_x = v[0];
                max_x = temp_p[0];
            }
            if(temp_p[1] < v[1]){
                max_y = v[1];
                min_y = temp_p[1];
            }
            else {
                min_y = v[1];
                max_y = temp_p[1];
            }
            if(temp_p[2] < v[2]){
                max_z = v[2];
                min_z = temp_p[2];
            }
            else {
                min_z = v[2];
                max_z = temp_p[2];
            }

            //check each intersection to see if it is within bounding box
            int interfere = 0;
            int j = 0;
            while(j<obs){
                if((temp_i[j][0] >= min_x && temp_i[j][0] <= max_x) &&
                   (temp_i[j][1] >= min_y && temp_i[j][1] <= max_y) &&
                   (temp_i[j][2] >= min_z && temp_i[j][2] <= max_z)){
                       if(obj[index] != temp_o[j]){
                         interfere = 1;
                       }
                }
                j++;
            }

            //if no shadows
            if(interfere == 0 || !SHADOW){

                if(dotprod(n,l,3) > 0){
                    //diffuse light
                    float rdiff = (kDiff * cur->diff[0] * po->diff[0] * dotprod(n,l,3));
                    float gdiff = (kDiff * cur->diff[1] * po->diff[1] * dotprod(n,l,3));
                    float bdiff = (kDiff * cur->diff[2] * po->diff[2] * dotprod(n,l,3));

                    // toon shading offsets
                    if(TOON){
                        if(rdiff > 0 && rdiff <= 0.2){
                            rdiff = 0.1;
                        }
                        if(rdiff > 0.2 && rdiff <= 0.4){
                            rdiff = 0.3;
                        }
                        if(rdiff > 0.4 && rdiff <= 0.6){
                            rdiff = 0.5;
                        }
                        if(rdiff > 0.6 && rdiff <= 0.8){
                            rdiff = 0.7;
                        }
                        if(rdiff > 0.8 && rdiff <= 1){
                            rdiff = 0.9;
                        }
                        if(gdiff > 0 && gdiff <= 0.2){
                            gdiff = 0.1;
                        }
                        if(gdiff > 0.2 && gdiff <= 0.4){
                            gdiff = 0.3;
                        }
                        if(gdiff > 0.4 && gdiff <= 0.6){
                            gdiff = 0.5;
                        }
                        if(gdiff > 0.6 && gdiff <= 0.8){
                            gdiff = 0.7;
                        }
                        if(gdiff > 0.8 && gdiff <= 1){
                            gdiff = 0.9;
                        }
                        if(bdiff > 0 && bdiff <= 0.2){
                            bdiff = 0.1;
                        }
                        if(bdiff > 0.2 && bdiff <= 0.4){
                            bdiff = 0.3;
                        }
                        if(bdiff > 0.4 && bdiff <= 0.6){
                            bdiff = 0.5;
                        }
                        if(bdiff > 0.6 && bdiff <= 0.8){
                            bdiff = 0.7;
                        }
                        if(bdiff > 0.8 && bdiff <= 1){
                            bdiff = 0.9;
                        }
                    }

                    r += rdiff;
                    g += gdiff;
                    b += bdiff;

                    //reflection vector f
                    f[0] = 2*n[0]*dotprod(n,l,3) - l[0];
                    f[1] = 2*n[1]*dotprod(n,l,3) - l[1];
                    f[2] = 2*n[2]*dotprod(n,l,3) - l[2];
                    f[3] = 1;
                    normalize(f);

                    //specular light
                    GLfloat shine = pow(dotprod(e,f,3), po->shine * 20);
                    if(shine > 0 && !TOON){
                        r += (kSpec * cur->spec[0] * po->spec[0] * shine);
                        g += (kSpec * cur->spec[1] * po->spec[1] * shine);
                        b += (kSpec * cur->spec[2] * po->spec[2] * shine);
                    }
                }
           }
           //garbage collection
           delete [] temp_i;
           delete [] temp_o;
        }
        //ambient light
        r += (po->amb[0]*kAmb);
			  g += (po->amb[1]*kAmb);
			  b += (po->amb[2]*kAmb);
			  a = 1.0f;
		}
		else
		{
      //base color
			r = 0.115f;
			g = 0.400f;
			b = 0.951f;
			a = 1.0f;
		}

    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;

    //garbage collection
    delete [] intersects;
    delete [] obj;

}

void idle()
{

	if(frame_traced)
	{
		int f_count = fd[cur_index].frames;
		//save current GL buffer as a frame (image file)
		for(int i=0; i<f_count; i++)
		{
			char filename[50];
			sprintf(filename, "frame%d.tga", cur_frame++);
			FILE   *out = fopen(filename,"wb");
			char   *pixel_data = new char[3*WINDOW_WIDTH*WINDOW_HEIGHT];
			short  TGAhead[] = { 0, 2, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 24 };
 
			glReadBuffer(GL_FRONT);
			glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_data);
 
			fwrite(TGAhead,sizeof(TGAhead),1,out);
			fwrite(pixel_data, 3*WINDOW_WIDTH*WINDOW_HEIGHT, 1, out);
			fclose(out);

			printf("Wrote frame %d of %d as %s\n", cur_frame, frame_count, filename);
      delete [] pixel_data;
		}

		if(cur_index + 1 < data_count)
		{
			cur_index++;
			my_cam.pos[0] = fd[cur_index].pos[0];
			my_cam.pos[1] = fd[cur_index].pos[1];
			my_cam.pos[2] = fd[cur_index].pos[2];
			my_cam.at[0] = fd[cur_index].at[0];
			my_cam.at[1] = fd[cur_index].at[1];
			my_cam.at[2] = fd[cur_index].at[2];
			my_cam.up[0] = fd[cur_index].up[0];
			my_cam.up[1] = fd[cur_index].up[1];
			my_cam.up[2] = fd[cur_index].up[2];
			raster_x = 0;
			raster_y = 0;

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			gluLookAt(my_cam.pos[0],my_cam.pos[1], my_cam.pos[2],
			my_cam.at[0],my_cam.at[1],my_cam.at[2],
			my_cam.up[0], my_cam.up[1], my_cam.up[2]);
		}
		else
		{
			finished = 1;
			printf("Rendering finished\n");
		}
		frame_traced = 0;
	}

	if(finished)
		return;


  float r, g, b, a;

    r = 0;
    g = 0;
    b = 0;
    a = 0;

	if (raster_x < WINDOW_WIDTH && raster_y < WINDOW_HEIGHT)
	{

    float rx = raster_x;
    float ry = WINDOW_HEIGHT - raster_y;
    Vector rgba;

    //super sample
    if(SUPER){
		    trace (rx,ry,rgba);
        r += rgba[0] * (0.4);
        g += rgba[1] * (0.4);
        b += rgba[2] * (0.4);
        a += rgba[3] * (0.4);
		    trace (rx-0.5,ry-0.5,rgba);
        r += rgba[0] * (0.15);
        g += rgba[1] * (0.15);
        b += rgba[2] * (0.15);
        a += rgba[3] * (0.15);
		    trace (rx-0.5,ry+0.5,rgba);
        r += rgba[0] * (0.15);
        g += rgba[1] * (0.15);
        b += rgba[2] * (0.15);
        a += rgba[3] * (0.15);
		    trace (rx+0.5,ry-0.5,rgba);
        r += rgba[0] * (0.15);
        g += rgba[1] * (0.15);
        b += rgba[2] * (0.15);
        a += rgba[3] * (0.15);
		    trace (rx+0.5,ry+0.5,rgba);
        r += rgba[0] * (0.15);
        g += rgba[1] * (0.15);
        b += rgba[2] * (0.15);
        a += rgba[3] * (0.15);
    }

    //single sample
    else{
		    trace (rx,ry,rgba);
        r += rgba[0];
        g += rgba[1];
        b += rgba[2];
        a += rgba[3];
    }

		plotPixel(screen, raster_x, raster_y, r, g, b, a);

    glBindTexture(GL_TEXTURE_2D, texId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, raster_x, raster_y, 1, 1, GL_RGBA, GL_FLOAT, (void*)(screen + (raster_y*WINDOW_WIDTH+raster_x)));
	}

	if (raster_x < WINDOW_WIDTH)
		raster_x++;

	if (raster_x == WINDOW_WIDTH) // finished line, increment Y raster
	{
		raster_x = 0;
		raster_y++;
	}
  if (raster_y == WINDOW_HEIGHT){
      	glutPostRedisplay();
        frame_traced = 1;
  }
}

void resizeWindow(int width, int height)
{
	if (height == 0) // prevent div/0
		height = 1;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)width/(double)height, 0.1, 200.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

  gluLookAt(my_cam.pos[0],my_cam.pos[1], my_cam.pos[2],
    my_cam.at[0],my_cam.at[1],my_cam.at[2],
    my_cam.up[0], my_cam.up[1], my_cam.up[2]);
}