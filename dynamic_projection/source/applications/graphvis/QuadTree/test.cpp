#include "../../paint/gl_includes.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <list>
#include <set>
#include <utility>
#include <iostream>

#include "../../../../calibration/planar_interpolation_calibration/planar_calibration.h"
#include "../../../../calibration/planar_interpolation_calibration/tracker.h"
#include "../../../../calibration/planar_interpolation_calibration/colors.h"

#include "QuadTree.h"

using namespace std;

QuadTree<int> tree;
vector<float> points;
QuadTree<int>::iterator iter(&tree);


int HandleGLError(std::string foo) {
	GLenum error;
	int i = 0;
	while ((error = glGetError()) != GL_NO_ERROR) {
		printf ("GL ERROR(#%d == 0x%x):  %s\n", i, error, gluErrorString(error));
		std::cout << foo << std::endl;
		if (error != GL_INVALID_OPERATION) i++;
	}
	if (i == 0) return 1;
	return 0;
}

#define CIRCLE_RES 20


void draw() {
	static GLfloat amb[] =  {0.4, 0.4, 0.4, 0.0};
	static GLfloat dif[] =  {1.0, 1.0, 1.0, 0.0};

	float s = 0.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	amb[3] = dif[3] = cos(s) / 2.0 + 0.5;
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_LIGHT2);
	glDisable(GL_LIGHT1);
	amb[3] = dif[3] = 0.5 - cos(s * .95) / 2.0;
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(0, 1920, 0, 1080);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	tree.Draw();
	iter.Draw();

	glPointSize(5);
	glColor3f(1,0,0);
	glBegin(GL_POINTS);
	for(unsigned int i = 0; i < points.size(); i +=2)
	{
		glVertex2f(points[i], points[i+1]);
	}
	glEnd();

	glDisable(GL_LINE_SMOOTH);
	//glDisable(GL_BLEND);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glMatrixMode(GL_MODELVIEW);


	glutSwapBuffers();
}


void display(void) {
	//  HandleGLError("BEFORE DISPLAY");
	draw();
	HandleGLError("AFTER DISPLAY");
}

int count = 0;

void idle(void) {

	/*
	count++;

	int *k = new int(5);

	tree.Add(k, (rand() % 1920 + 1), (rand() % 1200 + 1));
	 
*/


	display();

}

void visible(int vis) {
	if (vis == GLUT_VISIBLE)
		glutIdleFunc(idle);
	else
		glutIdleFunc(NULL);
}


void keyboard(unsigned char key, int x, int y) {

	if(key == 'q')
		exit(0);
	if(key == 'r')
	{
		tree.Clear();
		points.clear();
		iter = tree.Begin();
	}
	if(key == '=')
	{
		iter++;
	}
	if(key == '-')
	{
		iter--;
	}
	if(key == '0')
	{
		iter = tree.End();
	}
	if(key == '9')
	{
		iter = tree.Begin();
	}

	display();
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {

	y = 1200-y-1;

	static int global_last_x;
	static int global_last_y;

	//bool shiftPressed = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
	//bool ctrlPressed = glutGetModifiers() & GLUT_ACTIVE_CTRL;
	//bool altPressed = glutGetModifiers() & GLUT_ACTIVE_ALT;

	if (state == GLUT_DOWN)
	{
		global_last_x = x;
		global_last_y = y;

		int *k = new int;
		*k = 4;

		//AZ removed for testing
/*		if(tree.Add(k, x, y))
		{
			points.push_back(global_last_x);
			points.push_back(global_last_y);
		}*/
	}
}

void reshape(int w, int h) {

}


// ===================================================================

int  main(int argc, char **argv) {

	// initialize things...
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1640, 1080);
	glutInitWindowPosition(20,20);
	glutCreateWindow("multi-surface graph");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutVisibilityFunc(visible);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	GLfloat light0_ambient[] = {0.2, 0.2, 0.2, 1.0};
	GLfloat light0_diffuse[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat light1_diffuse[] = {1.0, 0.0, 0.0, 1.0};
	GLfloat light1_position[] = {1.0, 1.0, 1.0, 0.0};
	GLfloat light2_diffuse[] = {0.0, 1.0, 0.0, 1.0};
	GLfloat light2_position[] = {-1.0, -1.0, 1.0, 0.0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
	glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0);

	iter = tree.Begin();

	/*
		x1 = 160;
		y1 = 1080;
		x2 = 1760;
		y2 = 120;
	 */

	//iter.SetBox(640, 722, 1522, 422);

	glutFullScreen();

	glutMainLoop();

	return 0;             /* ANSI C requires main to return int. */
}
