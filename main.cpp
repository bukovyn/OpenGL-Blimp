/*******************************************************************
Multi-Part Model Construction and Manipulation
By: Nazar Bukovynsky
********************************************************************/

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "CubeMesh.h"
#include "QuadMesh.h"

void screenText(const char *text, int len, int x, int y);
void timer(int value);
void initializeSub();
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void functionKeys(int key, int x, int y);
void drawSub();
void circle(float a, float b, float rad);
int subMenu, mainMenu;
bool textHelp = false;

const float DEG2RAD = 3.14159 / 180;

GLfloat light_position0[] = { -6.0,  12.0, 0.0,1.0 };
GLfloat light_position1[] = { 6.0,  12.0, 0.0,1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };

// Set up lighting/shading and material properties for submarine - upcoming lecture
GLfloat submarine_mat_ambient[] = { 0.4, 0.2, 0.0, 1.0 };
GLfloat submarine_mat_specular[] = { 0.1, 0.1, 0.0, 1.0 };
GLfloat submarine_mat_diffuse[] = { 0.9, 0.5, 0.0, 1.0 };
GLfloat submarine_mat_shininess[] = { 0.0 };

QuadMesh *groundMesh = NULL;

struct BoundingBox
{
	VECTOR3D min;
	VECTOR3D max;
} BBox;

typedef struct Submarine
{
	float xPos, yPos, zPos;
	float orientation;
	float propSpeed;
	float speed;
	float bodyWidth;
	float bodyLength;
	float bodyHeight;
	float propOrientation;
} Submarine; Submarine sub;

// Default Mesh Size
int meshSize = 16;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Scene Modeller");
	initOpenGL(500, 500);

	glutTimerFunc(5, timer, 0);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(functionKeys);

	glutMainLoop();
	return 0;
}

// Setup openGL 
void initOpenGL(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and modeling transforms
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.2, 80.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.6, 0.6, 0.6, 0.0);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	//renormalize normal vectors 
	glEnable(GL_NORMALIZE);

	//Nice perspective.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-8.0f, 0.0f, 8.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 16.0);
	groundMesh->InitMesh(meshSize, origin, 16.0, 16.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);

	// Set up the bounding box of the scene
	BBox.min.Set(-8.0f, 0.0, -8.0);
	BBox.max.Set(8.0f, 6.0, 8.0);

	initializeSub();
}

void circle(float a, float b, float rad)
{
	int num;
	int triangles = 1000;
	float twoPi = 2.0f * 3.14;
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(5.0);
	glBegin(GL_LINES);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	for (num = 0; num < triangles - 1; num++)
	{
		glVertex2f(a, b);
		glVertex2f(a + (rad * cos(num * twoPi/triangles)), b + (rad * sin(num * twoPi/triangles)));
	}
	glEnd();
}

void timer(int value)
{
	sub.xPos -= sub.speed;
	sub.propOrientation += 1.5;
	glutPostRedisplay();
	glutTimerFunc(20, timer, 0);

	if (sub.xPos < -16)
	{
		sub.xPos = 16;
	}
	else if (sub.yPos > 16)
	{
		sub.yPos = -16;
	}
}

void drawSub()
{
	glRotatef(sub.orientation, 0.0, 1.0, 0.0);

	//Sub Base
	glPushMatrix();
	glTranslatef(sub.xPos, sub.yPos, sub.zPos);
	glScalef(sub.bodyLength, sub.bodyHeight, sub.bodyWidth);
	glutSolidSphere(1.0, 100, 100);
	glPopMatrix();

	//Main Propeller
	glPushMatrix();
	glTranslatef(sub.xPos + 3, sub.yPos, sub.zPos);
	glRotatef(sub.propOrientation * sub.propSpeed, 1.0, 0.0, 0.0);
	glScalef(1.0, 3.5, 1.0);
	glutSolidCube(0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos + 3, sub.yPos, sub.zPos);
	glRotatef(sub.propOrientation * sub.propSpeed, 1.0, 0.0, 0.0);
	glScalef(1.0, 1.0, 3.5);
	glutSolidCube(0.5);
	glPopMatrix();

	//Mini Propellors
	glPushMatrix();
	glTranslatef(sub.xPos + 0.5, sub.yPos - 1.5, sub.zPos + 1);
	glRotatef((sub.propOrientation + 45) * sub.propSpeed, 1.0, 0.0, 0.0);
	glScalef(1.0, 3.5, 1.0);
	glutSolidCube(0.25);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos + 0.5, sub.yPos - 1.5, sub.zPos + 1);
	glRotatef((sub.propOrientation + 45) * sub.propSpeed, 1.0, 0.0, 0.0);
	glScalef(1.0, 1.0, 3.5);
	glutSolidCube(0.25);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos + 0.5, sub.yPos - 1.5, sub.zPos - 1);
	glRotatef((sub.propOrientation + 45) * sub.propSpeed, 1.0, 0.0, 0.0);
	glScalef(1.0, 3.5, 1.0);
	glutSolidCube(0.25);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos + 0.5, sub.yPos - 1.5, sub.zPos - 1);
	glRotatef((sub.propOrientation + 45) * sub.propSpeed, 1.0, 0.0, 0.0);
	glScalef(1.0, 1.0, 3.5);
	glutSolidCube(0.25);
	glPopMatrix();

	//Propellor Holders
	glPushMatrix();
	glTranslatef(sub.xPos + 0.25, sub.yPos -1.5, sub.zPos + 1);
	glScalef(2.0, 1.0, 1.0);
	glutSolidCube(0.25);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos, sub.yPos -1.25, sub.zPos + 1);
	glScalef(1.0, 3.0, 1.0);
	glutSolidCube(0.25);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos + 0.25, sub.yPos - 1.5, sub.zPos - 1);
	glScalef(2.0, 1.0, 1.0);
	glutSolidCube(0.25);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos, sub.yPos - 1.25, sub.zPos - 1);
	glScalef(1.0, 3.0, 1.0);
	glutSolidCube(0.25);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos + 2.5, sub.yPos, sub.zPos);
	glScalef(2.0, 1.0, 1.0);
	glutSolidCube(0.5);
	glPopMatrix();

	//Side Mods
	glPushMatrix();
	glTranslatef(sub.xPos,sub.yPos - 0.5, sub.zPos + 1);
	glScalef(2.5, 1.25, 1.0);
	GLUquadricObj *mySphere;
	mySphere = gluNewQuadric();
	gluQuadricDrawStyle(mySphere, GLU_LINE);
	gluSphere(mySphere, 0.4, 100, 100);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sub.xPos, sub.yPos - 0.5, sub.zPos - 1);
	glScalef(2.5, 1.25, 1.0);
	GLUquadricObj *mySphereTwo;
	mySphereTwo = gluNewQuadric();
	gluQuadricDrawStyle(mySphereTwo, GLU_LINE);
	gluSphere(mySphere, 0.4, 100, 100);
	glPopMatrix();

}

void initializeSub()
{
	sub.xPos = sub.zPos = 0;
	sub.yPos = 5.0;
	sub.orientation = 0;
	sub.propSpeed = 1.0;
	sub.speed = 0.05;
	sub.bodyLength = 2.25;
	sub.bodyHeight = 1.0;
	sub.bodyWidth = 1;
	sub.propOrientation = 90;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	// Set up the camera
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	groundMesh->DrawMesh(meshSize);
	// Set submarine material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);
	drawSub();
	if (textHelp == true) {
		glLoadIdentity();
		std::string text;
		text = "Page Up: Speed Increase";
		std::string textTwo;
		textTwo = "Page Down: Speed Decrease";
		std::string textThree;
		textThree = "Arrow Keys: Direction";
		std::string textFour;
		textFour = "F1 toggles the hotkey menu";
		screenText(text.data(), text.size(), 50, 200);
		screenText(textTwo.data(), textTwo.size(), 50, 180);
		screenText(textThree.data(), textThree.size(), 50, 160);
		screenText(textFour.data(), textFour.size(), 50, 140);
	}
	glutSwapBuffers();
}


//Called at initialization and whenever user resizes the window
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.2, 40.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

VECTOR3D pos = VECTOR3D(0, 0, 0);

//Keyboard Inputs
void functionKeys(int key, int x, int y)
{
	VECTOR3D min, max;

	switch (key)
	{
	case GLUT_KEY_PAGE_UP:
		if (sub.speed < 0.25)
		{
			sub.speed += 0.025;
			sub.propSpeed += 0.25;
		}
		break;
	case GLUT_KEY_PAGE_DOWN:
		if (sub.speed > 0.1)
		{
			sub.speed -= 0.025;
			sub.propSpeed -= 0.25;
		}
		break;
	case GLUT_KEY_DOWN:
		sub.yPos -= 0.25;
		break;
	case GLUT_KEY_UP:
		sub.yPos += 0.25;
		break;
	case GLUT_KEY_RIGHT:
		sub.orientation -= 5;
		break;
	case GLUT_KEY_LEFT:
		sub.orientation += 5;
		break;
	case GLUT_KEY_F1:
		if (textHelp == false) textHelp = true;
		else textHelp = false;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void screenText(const char *text, int len, int x, int y)
{
	glMatrixMode(GL_PROJECTION);
	double *mat = new double[16];
	glGetDoublev(GL_PROJECTION_MATRIX, mat);
	glLoadIdentity();
	glOrtho(0, 800, 0, 600, -5, 5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glLoadIdentity();
	glRasterPos2i(x, y);
	for (int i = 0; i < len; i++) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)text[i]);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(mat);
	glMatrixMode(GL_MODELVIEW);
}
