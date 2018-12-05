/*******************************************************************
		   Multi-Part Model Construction and Manipulation
********************************************************************/

/**
	Lauren Marsillo
	500689959
	CPS511
**/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "QuadMesh.h"

#define PI 3.14159265
#define val PI / 180

const int meshSize = 16;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

static int currentButton;
static unsigned char currentKey;

// Lighting/shading and material properties for drone - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat drone_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat drone_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat drone_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat drone_mat_shininess[] = { 0.0F };

// A quad mesh representing the ground
static QuadMesh groundMesh;

// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
Vector3D ScreenToWorld(int x, int y);

static GLfloat spin = 0.0;

// drone values
Vector3D droneOrigin;
Vector3D front;
Vector3D forwardV;
float droneRad = 0.0;
float droneAngle = 0.0;

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 1");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	// Set up ground quad mesh
	Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
	Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh, meshSize, origin, 16.0, 16.0, dir1v, dir2v);

	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	//Set(&BBox.min, -8.0f, 0.0, -8.0);
	//Set(&BBox.max, 8.0f, 6.0,  8.0);

	droneOrigin = NewVector3D(0.0f, 4.0f, 0.0f);
	front = NewVector3D(-2.3f, 4.0f, 0.0f);
	Subtract(&front, &droneOrigin, &forwardV);
}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw Drone

	// Set drone material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);

	glPushMatrix();

	// Translates drone according to the droneOrigin vector, set when the user selects the f, b , up or down key
	glTranslatef(droneOrigin.x, droneOrigin.y, droneOrigin.z);

	// Rotates drone around its local y axis when user selects the right or left arrow keys
	glRotatef(droneAngle, 0.0, 1.0, 0.0);

	// body of drone
	glPushMatrix();
	glScalef(3.0, 1.0, 3.0);
	glutSolidCube(1.0);
	glPopMatrix();

	//front cylinder
	glPushMatrix();
	glTranslatef(-1.3, 1.0, 0.0);
	glScalef(0.5, 5.0, 0.5);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glutSolidTorus(0.1, 0.3, 20, 20);
	glPopMatrix();

	//left cylinder
	glPushMatrix();
	glTranslatef(1.3, 1.0, 1.3);
	glScalef(0.5, 5.0, 0.5);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glutSolidTorus(0.1, 0.3, 20, 20);
	glPopMatrix();

	//right cylinder
	glPushMatrix();
	glTranslatef(1.3, 1.0, -1.3);
	glScalef(0.5, 5.0, 0.5);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glutSolidTorus(0.1, 0.3, 20, 20);
	glPopMatrix();

	//front propellors
	glPushMatrix();
	glTranslatef(-1.3, 1.4, 0.0);
	glRotatef(spin, 0.0, 1.0, 0.0);

	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(1.5, 1.5, 10.0);
	glutSolidCone(0.1, 0.1, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glScalef(1.5, 1.5, 10.0);
	glutSolidCone(0.1, 0.1, 20, 20);
	glPopMatrix();

	glPopMatrix();

	//left propellors
	glPushMatrix();
	glTranslatef(1.3, 1.4, 1.3);
	glRotatef(spin, 0.0, 1.0, 0.0);

	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(1.5, 1.5, 10.0);
	glutSolidCone(0.1, 0.1, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glScalef(1.5, 1.5, 10.0);
	glutSolidCone(0.1, 0.1, 20, 20);
	glPopMatrix();

	glPopMatrix();

	//right propellors
	glPushMatrix();
	glTranslatef(1.3, 1.4, -1.3);
	glRotatef(spin, 0.0, 1.0, 0.0);

	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(1.5, 1.5, 10.0);
	glutSolidCone(0.1, 0.1, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glScalef(1.5, 1.5, 10.0);
	glutSolidCone(0.1, 0.1, 20, 20);
	glPopMatrix();

	glPopMatrix();

	glPopMatrix();

	// random building
	glPushMatrix();
	glTranslatef(4.0, 1.0, 4.0);
	glutSolidCube(2.0);
	glPopMatrix();
	
	// Draw ground mesh
	DrawMeshQM(&groundMesh, meshSize);

	glutSwapBuffers();   // Double buffering, swap buffers
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void spinDisplay(void)
{
	spin += 2.0;
	if (spin > 360.0)
		spin -= 360.0;

	glutPostRedisplay();
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	Vector3D unit = forwardV;
	Vector3D unitB;
	Negate(&forwardV, &unitB);
	switch (key)
	{
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 's':
		glutIdleFunc(spinDisplay);
		break;
	case 'S':
		glutIdleFunc(NULL);
		break;
	case 'f':
		Normalize(&unit);
		Add(&droneOrigin, &unit, &droneOrigin);
		Add(&front, &unit, &front);

		glutPostRedisplay();
		break;
	case 'b':
		Normalize(&unitB);
		Add(&droneOrigin, &unitB, &droneOrigin);
		Add(&front, &unitB, &front);

		glutPostRedisplay();
		break;

	case 't':
			break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	Vector3D temp = front;

	// Help key
	if (key == GLUT_KEY_F1)
	{
		/* Displays vector values for testing purposes
		printf("Drone Origin Vector: %.3f, %.3f, %.3f\n", droneOrigin.x, droneOrigin.y, droneOrigin.z);
		printf("Front Vector: %.3f, %.3f, %.3f\n", front.x, front.y, front.z);
		printf("Forward Vector: %.3f, %.3f, %.3f\n", forwardV.x, forwardV.y, forwardV.z);
		printf("Drone Angle: %.3f\n", droneAngle);
		putchar('\n');
		*/

		printf("Controls for the drone are as follows: \n");
		printf("\ts - spins all the propellors \n");
		printf("\tS - stops propellors from spinning \n");
		printf("\tf - move forward \n");
		printf("\tb - move backward \n");
		printf("\tup arrow - move upward \n");
		printf("\tdown arrow - move downward \n");
		printf("\tleft arrow - rotate left \n");
		printf("\tright arrow - rotate right \n");
	}

	// Do transformations with arrow keys

	switch (key)
	{
	case GLUT_KEY_DOWN:
		Set(&droneOrigin, droneOrigin.x, droneOrigin.y - 1, droneOrigin.z);
		glutPostRedisplay();
		break;

	case GLUT_KEY_UP:
		Set(&droneOrigin, droneOrigin.x, droneOrigin.y + 1, droneOrigin.z);
		glutPostRedisplay();
		break;

	case GLUT_KEY_RIGHT:
		droneAngle -= 5.0;
		droneRad = (-5) * val;
		front.x = (temp.z * sin(droneRad)) + (temp.x * cos(droneRad));
		front.z = (temp.z * cos(droneRad)) - (temp.x * sin(droneRad));
		Set(&forwardV, front.x, 0, front.z);
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		droneAngle += 5.0;
		droneRad = 5 * val;
		front.x = (temp.z * sin(droneRad)) + (temp.x * cos(droneRad));
		front.z = (temp.z * cos(droneRad)) - (temp.x * sin(droneRad));
		Set(&forwardV, front.x, 0, front.z);
		glutPostRedisplay();
		break;
	default:
		break;
	}
	
	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


Vector3D ScreenToWorld(int x, int y)
{
	// you will need to finish this if you use the mouse
	return NewVector3D(0, 0, 0);
}



