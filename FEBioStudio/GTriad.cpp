#include "stdafx.h"
#include "GTriad.h"
#include <qopengl.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

GTriad::GTriad(void)
{
	m_tcol.setRgb(0,0,0);
}

GTriad::~GTriad(void)
{
}

void GTriad::Render()
{
	// set the triad material properties
	GLfloat specular[] = {1.f,1.f,1.f,1.f};
	GLfloat zero[] = {0,0,0,1};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 64);

	// store the attribute bits
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	// reset the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// orient the triad
	vec3d r = m_rot.GetVector();
	double a = 180*m_rot.GetAngle()/PI;
	if ((a > 0) && (r.Length() > 0)) glRotated(a, r.x, r.y, r.z);	

	// create the cylinder object
	GLUquadricObj* pcyl = gluNewQuadric();

	const GLdouble r0 = .05;
	const GLdouble r1 = .15;

	// X-axis
	glPushMatrix();
	{
		glRotatef(90, 0, 1, 0);
		glColor3ub(255, 0, 0);
		gluCylinder(pcyl, r0, r0, .9, 5, 1);
		glTranslatef(0,0,.8f);
		gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	}
	glPopMatrix();

	//Y-axis
	glPushMatrix();
	{
		glRotatef(-90, 1, 0, 0);
		glColor3ub(0, 255, 0);
		gluCylinder(pcyl, r0, r0, .9, 5, 1);
		glTranslatef(0,0,.8f);
		gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	}
	glPopMatrix();

	//Z-axis
	glPushMatrix();
	{
		glColor3ub(0, 0, 255);
		gluCylinder(pcyl, r0, r0, .9, 5, 1);
		glTranslatef(0,0,.8f);
		gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	}
	glPopMatrix();

	// reset the modelview matrix
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	// determine the location of the lables
	float b = 1.1f;
	vec3d ex(b, 0.f, 0.f);
	vec3d ey(0.f, b, 0.f);
	vec3d ez(0.f, 0.f, b);
	m_rot.RotateVector(ex);
	m_rot.RotateVector(ey);
	m_rot.RotateVector(ez);

	// render the labels
/*	gl_color(m_tcol);
	gl_font(FL_HELVETICA, 12);
	gl_draw("X", (float) ex.x, (float) ex.y);
	gl_draw("Y", (float) ey.x, (float) ey.y);
	gl_draw("Z", (float) ez.x, (float) ez.y);
*/
	// cleanup
	gluDeleteQuadric(pcyl);
	glPopAttrib();
}
