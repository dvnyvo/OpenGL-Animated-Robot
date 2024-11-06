
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "QuadMesh.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


const int vWidth  = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// Note how everything depends on robot body dimensions so that can scale entire robot proportionately
// just by changing robot body scale
float robotBodyWidth = 8.0;
float robotBodyLength = 10.0;
float robotBodyDepth = 6.0;
float headWidth = 0.4*robotBodyWidth;
float headLength = headWidth;
float headDepth = headWidth;
float upperArmLength = robotBodyLength;
float upperArmWidth = 0.125*robotBodyWidth;
float gunLength = upperArmLength / 4.0;
float gunWidth = upperArmWidth;
float gunDepth = upperArmWidth;
float stanchionLength = robotBodyLength;
float stanchionRadius = 0.1*robotBodyDepth;
float baseWidth = 2 * robotBodyWidth;
float baseLength = 0.25*stanchionLength;

float robotX = 0.0;
float robotZ = 0.0;
float robotWalkSpeed = 1.0;
float robotFwdDistance = 0.0;

// Control Robot body rotation on base
float robotAngle = 0.0;
float shoulderAngle = 0.0;
float gunAngle = -25.0;
float elbowAngle = 0.0;
float leftLegAngle = -7.0;
float rightLegAngle = -7.0;
float leftKneeAngle = 6.0;
float rightKneeAngle = 6.0;

bool isLeftLegStep = true; //always start step on left leg
bool isRightLegStep = false;
float leftLegIncrement = 1.0;
float leftKneeIncrement = 1.5;
float rightLegIncrement = 1.0;
float rightKneeIncrement = 1.5;

// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)
GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 32.0F };


GLfloat robotArm_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotArm_mat_diffuse[] = { 0.5f,0.0f,0.0f,1.0f };
GLfloat robotArm_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotArm_mat_shininess[] = { 32.0F };

GLfloat gun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gun_mat_diffuse[] = { 0.01f,0.0f,0.01f,0.01f };
GLfloat gun_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat gun_mat_shininess[] = { 100.0F };

GLfloat robotLowerBody_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotLowerBody_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotLowerBody_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotLowerBody_mat_shininess[] = { 76.8F };


// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// Mouse button
int currentButton;

// A flat open mesh
QuadMesh *groundMesh = NULL;

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
    VECTOR3D min;
	VECTOR3D max;
} BBox;

// Default Mesh Size
int meshSize = 16;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void animationWalkingHandler(int param);
void animationLHandler(int param);
void animationRHandler(int param);
void animationFwdHandler(int param);
void animationBwdHandler(int param);
void animationGunHandler(int param);
void drawRobot();
void drawBody();
void drawHead();
void drawLowerBody();
void drawLeftArm();
void drawRightArm();

void drawLeftLeg();
void drawRightLeg();


int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("3D Hierarchical Example");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). 
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
	glEnable(GL_LIGHT1);   // This second light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// Other initializatuion
	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-16.0f, 0.0f, 16.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 32.0);
	groundMesh->InitMesh(meshSize, origin, 32.0, 32.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot

	// Apply modelling transformations M to move robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	drawRobot();

	// Draw ground
	glPushMatrix();
	glTranslatef(0.0, -20.0, 0.0);
	groundMesh->DrawMesh(meshSize);
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{
	glPushMatrix();
	 // spin robot on base. 
		glTranslatef(robotX, 1.0, robotZ);
		glRotatef(robotAngle, 0.0, 1.0, 0.0);
		glPushMatrix();
			 drawBody();
			 glPushMatrix();
				drawHead();
				glPushMatrix();
					drawLeftArm();
					drawRightArm();
				glPopMatrix();
				glPushMatrix();
				drawLowerBody();
					glPushMatrix();
						drawLeftLeg();
						drawRightLeg();
					glPopMatrix();		
				glPopMatrix();	 
			 glPopMatrix();	 
		glPopMatrix();
	glPopMatrix();
	
	// don't want to spin fixed base in this example
	

	glPopMatrix();
}


void drawBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	glPushMatrix(); //level 1 hierarchy - torso
		glPushMatrix(); // lvl 2 oblique
			glTranslatef(0.0,2.0,0.0);  
			glRotatef(90.0, 1.0, 0.0, 0.0);  
			glScalef(0.13 * robotBodyWidth, 0.06 * robotBodyWidth, 0.1 * robotBodyWidth);
			gluCylinder(gluNewQuadric(), 4.0, 1.0, 5.0, 20, 10);
		glPopMatrix();

		glPushMatrix(); // level 2 chest
			glPushMatrix(); //lvl 3 inner chest
				glTranslatef(0.0, 3.5, -0.5);
				glScalef(0.9 * robotBodyWidth, 0.32 * robotBodyWidth, 0.4 * robotBodyWidth);
				glutSolidCube(1.0);
			glPopMatrix();

			glPushMatrix(); // lvl 3 left chest
				glTranslatef(-0.3 * robotBodyWidth, 0.3 * robotBodyLength, 0.0 * robotBodyWidth);
				glRotatef(-17.0, 0.0, 1.0, 1.0);
				glScalef(0.28 * robotBodyWidth, 0.18 * robotBodyWidth, 0.33 * robotBodyDepth);
				glutSolidCube(2.0);
			glPopMatrix();

			glPushMatrix(); // lvl 3 right chest
				glTranslatef(0.3 * robotBodyWidth, 0.3 * robotBodyLength, 0.0 * robotBodyWidth);
				glRotatef(17.0, 0.0, 1.0, 1.0);
				glScalef(0.28 * robotBodyWidth, 0.18 * robotBodyWidth, 0.33 * robotBodyDepth);
				glutSolidCube(2.0);
			glPopMatrix();
		glPopMatrix();

		glPushMatrix(); // lvl 2 stomach
			glTranslatef(0.0, -2, 0.0);
			glRotatef(-90.0, 1.0, 0., 0.0);
			glScalef(0.245 * robotBodyWidth, 0.245 * robotBodyWidth, 0.7 * robotBodyWidth);
			gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 20.0, 10.0);
		glPopMatrix();

		glPushMatrix(); // lvl 2 chest cannon
			glTranslatef(0.0, 3, 0.0);
			glPushMatrix(); // lvl 3 outer cone
				glRotatef(-90.0, 0.0, 0.0, 4.0);
				glScalef(0.14 * robotBodyWidth, 0.14 * robotBodyWidth, 0.35 * robotBodyWidth);
				gluCylinder(gluNewQuadric(), 2.2, 1.0, 1.0, 20.0, 10.0);
			glPopMatrix();

			glPushMatrix(); // lvl 3 turning cannon
				glRotatef(gunAngle, 0.0, 0.0, 1.0); //rotate cannon angle
				glPushMatrix(); // lvl 4 cannon
					glTranslatef(0.0, 0.0, 2);
					glRotatef(-90.0, 0.0, 0.0, 4.0);
					glScalef(0.145 * robotBodyWidth, 0.145 * robotBodyWidth, 0.16 * robotBodyWidth);
					gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 20.0, 10.0);
				glPopMatrix();

				glPushMatrix(); // lvl 4 cone inside cannon
					glTranslatef(0.0, 0.0, 2.5);
					glRotatef(-90.0, 0.0, 0.0, 4.0);
					glScalef(0.07 * robotBodyWidth, 0.07 * robotBodyWidth, 0.15 * robotBodyWidth);
					gluCylinder(gluNewQuadric(), 2.2, 0.0, 1.0, 20.0, 10.0);
				glPopMatrix();

				glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
				glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
				glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);

				glPushMatrix(); // lvl 4 cube inside cannon
					glTranslatef(0.0, 0.0, 2.4);
					glRotatef(gunAngle, 0.0, 0.0, 1.0);
					glScalef(0.18 * robotBodyWidth, 0.18 * robotBodyWidth, 0.18 * robotBodyWidth);
					glutSolidCube(1.0);
				glPopMatrix();
			glPopMatrix();
		glPopMatrix();	
	glPopMatrix();
}

void drawHead()
{
	// Set robot material properties per body part. Can have seperate material properties for each part
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	glPushMatrix();
	// Position head with respect to parent (body)
		glTranslatef(0.0, 0.5*robotBodyLength+0.5*headLength, 0.0); // this will be done last
	
		// Build Head
		glPushMatrix();
			glScalef(0.3*robotBodyWidth, 0.25*robotBodyWidth, 0.3*robotBodyWidth);
			glutSolidCube(1.0);
		glPopMatrix();


		glPushMatrix(); // neck
			glTranslatef(0.0, -2, 0.0);
			glRotatef(-90.0, 1.0, 0.0, 0.0);
			glScalef(0.09 * robotBodyWidth, 0.07 * robotBodyWidth, 0.15 * robotBodyWidth);
			gluCylinder(gluNewQuadric(), 2.2, 1.5, 1.0, 20, 10);
		glPopMatrix();
	glPopMatrix();
}

void drawLowerBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);

	glPushMatrix();
		glTranslatef(0, -0.13*robotBodyLength, 0.0); 

		// crotch area
		glPushMatrix();
			glScalef(0.9 * stanchionRadius, 0.35 *stanchionLength, 0.87 * stanchionRadius);
			glRotatef(90.0, 1.0, 0.0, 0.0);
			gluCylinder(gluNewQuadric(), 4.0, 3.0, 1.0, 20, 10);
		glPopMatrix();
	glPopMatrix();
}

void drawLeftLeg() {
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix(); // lvl 1 base leg
		glTranslatef(-0.08 * robotBodyWidth - upperArmWidth, -4.0, -0.2);
		glRotatef(leftLegAngle, 1.0, 0.0, 0.0); //rotate leg angle
		glRotatef(-6.5, 0.0, 0.0, 1.0);

		glPushMatrix(); // lvl 2 upper leg
			glTranslatef(0.0, -2.0, 0.0);

			glPushMatrix(); // lvl 3 main thigh
				glScalef(2.0, 6.0, 3.0);
				glRotatef(-90.0, 1.0, 0.0, 0.0);
				glutSolidCube(1.0);
			glPopMatrix();

			glPushMatrix(); //lvl 3 quads
				glTranslatef(0.0, -1, 0.0);
				glScalef(3.0, 6.5, 2.0);
				glutSolidCube(1.0);
			glPopMatrix();

			glPushMatrix(); // lvl 3 lower leg
				glTranslatef(0.0, -3.5, 0.0);
				glRotatef(leftKneeAngle, 1.0, 0.0, 0.0);

				glPushMatrix(); // lvl 4 lower leg-foot
					glRotatef(90.0, 1.0, 0.0, 0.0);
					glScalef(0.19 * robotBodyWidth, 0.19 * robotBodyWidth, 0.48 * robotBodyWidth);
					gluCylinder(gluNewQuadric(), 1.2, 0.9, 1.5, 20.0, 10.0);
				glPopMatrix();

				glPushMatrix(); // lvl 4 foot
					glTranslatef(0.0, -6.3, 0.2);
					glScalef(2.0, 1.0, 3.3);
					glutSolidCube(1.0);
				glPopMatrix();

			glPopMatrix();

		glPopMatrix();

	glPopMatrix();

}

void drawRightLeg() {
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix(); // lvl 1 base leg
		glTranslatef(0.08 * robotBodyWidth + upperArmWidth, -4, -0.2);
		glRotatef(rightLegAngle, 1.0, 0.0, 0.0); //rotate leg angle
		glRotatef(6.5, 0.0, 0.0, 1.0);

		glPushMatrix(); // lvl 2 upper leg
			glTranslatef(0.0, -2.0, 0.0);

			glPushMatrix(); // lvl 3 main thigh
				glScalef(2.0, 6.0, 3.0);
				glRotatef(-90.0, 1.0, 0.0, 0.0);
				glutSolidCube(1.0);
				glPopMatrix();

				glPushMatrix(); //lvl 3 quads
					glTranslatef(0.0, -1, 0.0);
					glScalef(3.0, 6.5, 2.0);
					glutSolidCube(1.0);
					glPopMatrix();

					glPushMatrix(); // lvl 3 lower leg
						glTranslatef(0.0, -3.5, 0.0);
						glRotatef(rightKneeAngle, 1.0, 0.0, 0.0);

						glPushMatrix(); // lvl 4 lower leg-foot
							glRotatef(90.0, 1.0, 0.0, 0.0);
							glScalef(0.19 * robotBodyWidth, 0.19 * robotBodyWidth, 0.48 * robotBodyWidth);
							gluCylinder(gluNewQuadric(), 1.2, 0.9, 1.5, 20.0, 10.0);
						glPopMatrix();

						glPushMatrix(); // lvl 4 foot
							glTranslatef(0.0, -6.3, 0.2);
							glScalef(2.0, 1.0, 3.3);
							glutSolidCube(1.0);
						glPopMatrix();

					glPopMatrix();

				glPopMatrix();

			glPopMatrix();

		glPopMatrix();

	glPopMatrix();

}

void drawLeftArm()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix(); // lvl 1 Left arm
		glTranslatef(0.5*robotBodyWidth + upperArmWidth, 3.8, -0.5); 
		glRotatef(shoulderAngle, 1.0, 0.0, 0.0); //rotate shoulder 
		glRotatef(5.0, 0.0, 0.0, 1.0);

		glPushMatrix();	// lvl 2 shoulder

			glPushMatrix(); // shoulder
				glScalef(3.0, 3.0, 3.0);
				glutSolidCube(1.0);
			glPopMatrix();
			
			glPushMatrix(); // lvl 3 middle arm
				glTranslatef(0.0,-2.0, 0.0);

				glPushMatrix(); //middle arm
					glScalef(2.0, 6.0, 2.0);
					glutSolidCube(1.0);
				glPopMatrix();

				glPushMatrix(); // lvl 4 forearm
					glTranslatef(0.0, -3.0, 0.0);

					glPushMatrix();
						glRotatef(elbowAngle, 1.0, 0.0, 0.0); //rotate elbow

						glPushMatrix(); //forearm
							glRotatef(90.0, 1.0, 0.0,0.0);
							glScalef(0.145 * robotBodyWidth, 0.12 * robotBodyWidth, 0.3 * robotBodyWidth);
							gluCylinder(gluNewQuadric(), 1.2, 0.8, 1.5, 20.0, 10.0);	
						glPopMatrix();

						glPushMatrix(); // lvl 5 hand
							glTranslatef(0.0, -4.0, 0.0);

							glPushMatrix(); //palm
								glScalef(0.16 * robotBodyWidth, 0.16 * robotBodyWidth, 0.16 * robotBodyWidth);
								glutSolidCube(1.0);
							glPopMatrix();


							glPushMatrix(); //lvl 6 fingers
								glTranslatef(0.5, -1.0, 0.2);
								glScalef(0.06 * robotBodyWidth, 0.22 * robotBodyWidth, 0.2 * robotBodyWidth);
								glutSolidCube(1.0);
							glPopMatrix();
						
						glPopMatrix();

					glPopMatrix();

				glPopMatrix();

			glPopMatrix();
				
		glPopMatrix();

	glPopMatrix();
}

void drawRightArm()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix(); // lvl 1 Left arm
		glTranslatef(-0.5 * robotBodyWidth - upperArmWidth, 3.8, -0.5);
		glRotatef(shoulderAngle, 1.0, 0.0, 0.0); //rotate shoulder 
		glRotatef(-5.0, 0.0, 0.0, 1.0);

		glPushMatrix();	// lvl 2 shoulder

			glPushMatrix(); // shoulder
			glScalef(3.0, 3.0, 3.0);
			glutSolidCube(1.0);
			glPopMatrix();

			glPushMatrix(); // lvl 3 middle arm
				glTranslatef(0.0, -2.0, 0.0);

				glPushMatrix(); //middle arm
				glScalef(2.0, 6.0, 2.0);
				glutSolidCube(1.0);
				glPopMatrix();

				glPushMatrix(); // lvl 4 forearm
					glTranslatef(0.0, -3.0, 0.0);

					glPushMatrix();
						glRotatef(elbowAngle, 1.0, 0.0, 0.0); //rotate elbow

						glPushMatrix(); //forearm
						glRotatef(90.0, 1.0, 0.0, 0.0);
						glScalef(0.145 * robotBodyWidth, 0.12 * robotBodyWidth, 0.3 * robotBodyWidth);
						gluCylinder(gluNewQuadric(), 1.2, 0.8, 1.5, 20, 10);
						glPopMatrix();

						glPushMatrix(); // lvl 5 hand
							glTranslatef(0.0, -4.0, 0.0);

							glPushMatrix(); //palm
							glScalef(0.16 * robotBodyWidth, 0.16 * robotBodyWidth, 0.16 * robotBodyWidth);
							glutSolidCube(1.0);
							glPopMatrix();


							glPushMatrix(); //lvl 6 fingers
							glTranslatef(-0.5, -1.0, 0.2);
							glScalef(0.06 * robotBodyWidth, 0.22 * robotBodyWidth, 0.2 * robotBodyWidth);
							glutSolidCube(1.0);
							glPopMatrix();

						glPopMatrix();

					glPopMatrix();

				glPopMatrix();

			glPopMatrix();

		glPopMatrix();

	glPopMatrix();
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 30.0, 60.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a': // non-animated movemenets, w,a,s,d
		robotAngle += 2.0;
		break;
	case 'd':
		robotAngle -= 2.0;
		break;
	case 'A':
		robotAngle -= 2.0;
		break;
	case 'D':
		robotAngle += 2.0;
		break;
	case 'w':
		robotX += robotWalkSpeed * sin(M_PI * 2.0 * (robotAngle / 360.0));
		robotZ += robotWalkSpeed * cos(M_PI * 2.0 * (robotAngle / 360.0));
		break;
	case 's':
		robotX -= robotWalkSpeed * sin(M_PI * 2.0 * (robotAngle / 360.0));
		robotZ -= robotWalkSpeed * cos(M_PI * 2.0 * (robotAngle / 360.0));
		break;
	case 'i': //animated movements i,j,k,l
		stop = false;
		glutTimerFunc(10.0, animationFwdHandler, 0.0);
		glutTimerFunc(10.0, animationWalkingHandler, 0.0);
		break;
	case 'k':
		stop = false;
		glutTimerFunc(10.0, animationBwdHandler, 0.0);
		glutTimerFunc(10.0, animationWalkingHandler, 0.0);
		break;
	case 'j':
		stop = false;
		glutTimerFunc(10.0, animationLHandler, 0.0);
		break;
	case 'l':
		stop = false;
		glutTimerFunc(10.0, animationRHandler, 0.0);
		break;
	case 'z': //invididual joint movements
		shoulderAngle += 2.0;
		break;
	case 'Z':
		shoulderAngle -= 2.0;
		break;
	case 'x':
		elbowAngle += 2.0;
		break;
	case 'X':
		elbowAngle -= 2.0;
		break;
	case 'g':
		stop = false;
		glutTimerFunc(10.0, animationGunHandler, 0.0);
		break;
	case 'G':
		stop = true;
		break;
	case 'c': // leg joints
		leftLegAngle -= 2.0;
		break;
	case 'C':
		leftLegAngle += 2.0;
		break;
		break;
	case 'f':
		rightLegAngle -= 2.0;
		break;
	case 'F':
		rightLegAngle += 2.0;
		break;
	case 'v': // knee joints
		leftKneeAngle -= 2.0;
		break;
	case 'V':
		leftKneeAngle += 2.0;
		break;
	case 'b':
		rightKneeAngle -= 2.0;
		break;
	case 'B':
		rightKneeAngle += 2.0;
	case '.':
		stop = true;
		break;

	}

	glutPostRedisplay();   // Trigger a window redisplay
}

void animationGunHandler(int param) // rotate to the left
{
	if (!stop)
	{
		gunAngle += 0.1;
		glutPostRedisplay();
		glutTimerFunc(10.0, animationGunHandler, 0.0);
	}
}
void animationLHandler(int param) // rotate to the left
{
	if (!stop)
	{
		robotAngle += 0.1;
		glutPostRedisplay();
		glutTimerFunc(10.0, animationLHandler, 0.0);
	}
}
void animationRHandler(int param) // rotate the right
{
	if (!stop)
	{
		robotAngle -= 0.1;
		glutPostRedisplay();
		glutTimerFunc(10.0, animationRHandler, 0.0);
	}
}
void animationFwdHandler(int param) // move robot forward
{
	if (!stop)
	{
		robotX += 0.01 * robotWalkSpeed * sin(M_PI * 2.0 * (robotAngle / 360.0));
		robotZ += 0.01 * robotWalkSpeed * cos(M_PI * 2.0 * (robotAngle / 360.0));
		glutPostRedisplay();
		glutTimerFunc(10.0, animationFwdHandler, 0.0);
	}
}
void animationBwdHandler(int param) // move robot backwards
{
	if (!stop)
	{
		robotX -= 0.01 * robotWalkSpeed * sin(M_PI * 2.0 * (robotAngle / 360.0));
		robotZ -= 0.01 * robotWalkSpeed * cos(M_PI * 2.0 * (robotAngle / 360.0));
		glutPostRedisplay();
		glutTimerFunc(10.0, animationBwdHandler, 0.0);
	}
}

void animationWalkingHandler(int param) {
	if (!stop) {
		// animate left leg raising
		if(isLeftLegStep) { // if left leg is stepping, then increase leg angle and decrease knee angle
			leftLegAngle += leftLegIncrement;
			leftKneeAngle -= leftKneeIncrement;
			if (leftLegAngle <= -70.0) { //once it raises the leg to the max then reverse the increment to go downwards
				leftLegIncrement = -leftLegIncrement;
				leftKneeIncrement = -leftKneeIncrement;
			}
			else if (leftLegAngle >= 1.0) {  //once the left leg goes down to default position, then left leg stops stepping and right leg starts, also reverse increment for when the leg steps again
				isLeftLegStep = false;
				isRightLegStep = true;
				leftLegIncrement = -leftLegIncrement;
				leftKneeIncrement = -leftKneeIncrement;
			}
		}
		else if(isRightLegStep) {
			// animate right leg raising
			rightLegAngle += rightLegIncrement;
			rightKneeAngle -= rightKneeIncrement;
			if (rightLegAngle <= -70.0) {
				rightLegIncrement = -rightLegIncrement;
				rightKneeIncrement = -rightKneeIncrement;
			}

			else if (rightLegAngle >= 1.0) {
				isLeftLegStep = true;
				isRightLegStep = false;
				rightLegIncrement = -rightLegIncrement;
				rightKneeIncrement = -rightKneeIncrement;
			}
		}
		glutPostRedisplay();
		glutTimerFunc(50.0, animationWalkingHandler, 0.0);
	}
	else { //reset joints
		leftLegAngle = -7.0;
		rightLegAngle = -7.0;
		leftKneeAngle = 6.0;
		rightKneeAngle = 6.0;
	}
	
		
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

