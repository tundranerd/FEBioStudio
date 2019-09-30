#include "stdafx.h"
#include <qopengl.h>
#include "glx.h"

//-----------------------------------------------------------------------------
void GLX::translate(const vec3d& r)
{
	glTranslated(r.x, r.y, r.z);
}

//-----------------------------------------------------------------------------
void GLX::rotate(const quatd& q)
{
	double w = q.GetAngle();
	if (w != 0)
	{
		vec3d r = q.GetVector();
		glRotated(180*w/PI, r.x, r.y, r.z);
	}
}

//-----------------------------------------------------------------------------
void GLX::drawLine(double x0, double y0, double x1, double y1, double a0, double a1, GLColor c, int n)
{
	double x, y;
	double f, g;
	double h1, h2, h3, a;
	glBegin(GL_LINE_STRIP);
	for (int i=0; i<=n; ++i)
	{
		f = ((double) i / (double) n);
		g = (1.0 - f);
		h1 = (g*(1.0 - 2.0*f));
		h2 = (f*(1.0 - 2.0*g));
		h3 = (4.0*f*g);

		a = (255.0*(h1*a0 + h2*a0 + h3*a1));
		if (a>255) a = 255;
		if (a < 0) a = 0;
		glColor4ub(c.r, c.g, c.b, (uchar) a);

		x = x0 + i*(x1 - x0)/n;
		y = y0 + i*(y1 - y0)/n;

		glVertex2d(x, y);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawCircle(double R, int N)
{
	double x, y;
	glBegin(GL_LINE_LOOP);
	{
		for (int i=0; i<N; ++i)
		{
			x = R*cos(i*2*PI/N);
			y = R*sin(i*2*PI/N);
			glVertex3d(x,y,0);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawCircle(const vec3d& c, double R, int N)
{
	double x, y;
	glBegin(GL_LINE_LOOP);
	{
		for (int i=0; i<N; ++i)
		{
			x = c.x + R*cos(i*2*PI/N);
			y = c.y + R*sin(i*2*PI/N);
			glVertex3d(x,y,c.z);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawPoint(const vec3d& r)
{
	glBegin(GL_POINTS);
	{
		glVertex3d(r.x, r.y, r.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawLine(const vec3d& a, const vec3d& b)
{
	glBegin(GL_LINES);
	{
		glVertex3d(a.x, a.y, a.z);
		glVertex3d(b.x, b.y, b.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawLine(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB)
{
	glBegin(GL_LINES);
	{
		glColor3ub(colA.r, colA.g, colA.b); glVertex3d(a.x, a.y, a.z);
		glColor3ub(colB.r, colB.g, colB.b); glVertex3d(b.x, b.y, b.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawLine_(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB)
{
	glColor3ub(colA.r, colA.g, colA.b); glVertex3d(a.x, a.y, a.z);
	glColor3ub(colB.r, colB.g, colB.b); glVertex3d(b.x, b.y, b.z);
}

//-----------------------------------------------------------------------------
void GLX::drawArc(const vec3d& c, double R, double w0, double w1, int N)
{
	glBegin(GL_LINE_STRIP);
	{
		for (int i=0; i<=N; ++i)
		{
			double w = w0 + i*(w1 - w0)/N;
			double x = c.x + R*cos(w);
			double y = c.y + R*sin(w);
			glVertex3d(x, y, c.z);
		}		
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLX::drawHelix(const vec3d& a, const vec3d& b, double R, double p, int N)
{
    vec3d c = b - a;
    double L = c.Length();
    
    if (L > 2*R) {
        c.Normalize();
        vec3d e = c ^ vec3d(1,0,0);
        if (e.Length() == 0) e = c ^ vec3d(0,1,0);
        e.Normalize();
        vec3d d = e ^ c; d.Normalize();
        double fp = (L - 2*R)/p;
        int n = fp*N;
        double dq = 2*PI/N;
        
        vec3d va = a + c*R;
        vec3d vb = b - c*R;
        vec3d x;
        glBegin(GL_LINE_STRIP);
        {
            glVertex3d(a.x,a.y,a.z);
            glVertex3d(va.x,va.y,va.z);
            for (int i=0; i<n; ++i)
            {
                x = va + (d*cos(i*dq) + e*sin(i*dq))*R + c*(((L - 2*R)*i)/n);
                glVertex3d(x.x,x.y,x.z);
            }
            glVertex3d(vb.x,vb.y,vb.z);
            glVertex3d(b.x,b.y,b.z);
        }
        glEnd();
    }
    else
        GLX::drawLine(a, b);
}

void GLX::quad4(vec3d r[4], vec3d n[4])
{
	vertex3d(r[0], n[0]); vertex3d(r[1], n[1]); vertex3d(r[2], n[2]);
	vertex3d(r[2], n[2]); vertex3d(r[3], n[3]); vertex3d(r[0], n[0]);
}

void GLX::quad4(vec3d r[4], vec3f n[4], float t[4])
{
	vertex3d(r[0], n[0], t[0]); vertex3d(r[1], n[1], t[1]); vertex3d(r[2], n[2], t[2]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[3], n[3], t[3]); vertex3d(r[0], n[0], t[0]);
}

void GLX::quad8(vec3d r[8], vec3f n[8], float t[8])
{
	vertex3d(r[7], n[7], t[7]); vertex3d(r[0], n[0], t[0]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[4], n[4], t[4]); vertex3d(r[1], n[1], t[1]); vertex3d(r[5], n[5], t[5]);
	vertex3d(r[5], n[5], t[5]); vertex3d(r[2], n[2], t[2]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[6], n[6], t[6]); vertex3d(r[3], n[3], t[3]); vertex3d(r[7], n[7], t[7]);
	vertex3d(r[7], n[7], t[7]); vertex3d(r[4], n[4], t[4]); vertex3d(r[5], n[5], t[5]);
	vertex3d(r[7], n[7], t[7]); vertex3d(r[5], n[5], t[5]); vertex3d(r[6], n[6], t[6]);
}

void GLX::quad9(vec3d r[9], vec3f n[9], float t[9])
{
	const int T[8][3] = {
		{ 0,4,8 },{ 8,7,0 },{ 4,1,5 },{ 5,8,4 },
		{ 7,8,6 },{ 6,3,7 },{ 8,5,2 },{ 2,6,8 } };

	glNormal3f(n[T[0][0]].x, n[T[0][0]].y, n[T[0][0]].z); glTexCoord1f(t[T[0][0]]); glVertex3f(r[T[0][0]].x, r[T[0][0]].y, r[T[0][0]].z);
	glNormal3f(n[T[0][1]].x, n[T[0][1]].y, n[T[0][1]].z); glTexCoord1f(t[T[0][1]]); glVertex3f(r[T[0][1]].x, r[T[0][1]].y, r[T[0][1]].z);
	glNormal3f(n[T[0][2]].x, n[T[0][2]].y, n[T[0][2]].z); glTexCoord1f(t[T[0][2]]); glVertex3f(r[T[0][2]].x, r[T[0][2]].y, r[T[0][2]].z);

	glNormal3f(n[T[1][0]].x, n[T[1][0]].y, n[T[1][0]].z); glTexCoord1f(t[T[1][0]]); glVertex3f(r[T[1][0]].x, r[T[1][0]].y, r[T[1][0]].z);
	glNormal3f(n[T[1][1]].x, n[T[1][1]].y, n[T[1][1]].z); glTexCoord1f(t[T[1][1]]); glVertex3f(r[T[1][1]].x, r[T[1][1]].y, r[T[1][1]].z);
	glNormal3f(n[T[1][2]].x, n[T[1][2]].y, n[T[1][2]].z); glTexCoord1f(t[T[1][2]]); glVertex3f(r[T[1][2]].x, r[T[1][2]].y, r[T[1][2]].z);

	glNormal3f(n[T[2][0]].x, n[T[2][0]].y, n[T[2][0]].z); glTexCoord1f(t[T[2][0]]); glVertex3f(r[T[2][0]].x, r[T[2][0]].y, r[T[2][0]].z);
	glNormal3f(n[T[2][1]].x, n[T[2][1]].y, n[T[2][1]].z); glTexCoord1f(t[T[2][1]]); glVertex3f(r[T[2][1]].x, r[T[2][1]].y, r[T[2][1]].z);
	glNormal3f(n[T[2][2]].x, n[T[2][2]].y, n[T[2][2]].z); glTexCoord1f(t[T[2][2]]); glVertex3f(r[T[2][2]].x, r[T[2][2]].y, r[T[2][2]].z);

	glNormal3f(n[T[3][0]].x, n[T[3][0]].y, n[T[3][0]].z); glTexCoord1f(t[T[3][0]]); glVertex3f(r[T[3][0]].x, r[T[3][0]].y, r[T[3][0]].z);
	glNormal3f(n[T[3][1]].x, n[T[3][1]].y, n[T[3][1]].z); glTexCoord1f(t[T[3][1]]); glVertex3f(r[T[3][1]].x, r[T[3][1]].y, r[T[3][1]].z);
	glNormal3f(n[T[3][2]].x, n[T[3][2]].y, n[T[3][2]].z); glTexCoord1f(t[T[3][2]]); glVertex3f(r[T[3][2]].x, r[T[3][2]].y, r[T[3][2]].z);

	glNormal3f(n[T[4][0]].x, n[T[4][0]].y, n[T[4][0]].z); glTexCoord1f(t[T[4][0]]); glVertex3f(r[T[4][0]].x, r[T[4][0]].y, r[T[4][0]].z);
	glNormal3f(n[T[4][1]].x, n[T[4][1]].y, n[T[4][1]].z); glTexCoord1f(t[T[4][1]]); glVertex3f(r[T[4][1]].x, r[T[4][1]].y, r[T[4][1]].z);
	glNormal3f(n[T[4][2]].x, n[T[4][2]].y, n[T[4][2]].z); glTexCoord1f(t[T[4][2]]); glVertex3f(r[T[4][2]].x, r[T[4][2]].y, r[T[4][2]].z);

	glNormal3f(n[T[5][0]].x, n[T[5][0]].y, n[T[5][0]].z); glTexCoord1f(t[T[5][0]]); glVertex3f(r[T[5][0]].x, r[T[5][0]].y, r[T[5][0]].z);
	glNormal3f(n[T[5][1]].x, n[T[5][1]].y, n[T[5][1]].z); glTexCoord1f(t[T[5][1]]); glVertex3f(r[T[5][1]].x, r[T[5][1]].y, r[T[5][1]].z);
	glNormal3f(n[T[5][2]].x, n[T[5][2]].y, n[T[5][2]].z); glTexCoord1f(t[T[5][2]]); glVertex3f(r[T[5][2]].x, r[T[5][2]].y, r[T[5][2]].z);

	glNormal3f(n[T[6][0]].x, n[T[6][0]].y, n[T[6][0]].z); glTexCoord1f(t[T[6][0]]); glVertex3f(r[T[6][0]].x, r[T[6][0]].y, r[T[6][0]].z);
	glNormal3f(n[T[6][1]].x, n[T[6][1]].y, n[T[6][1]].z); glTexCoord1f(t[T[6][1]]); glVertex3f(r[T[6][1]].x, r[T[6][1]].y, r[T[6][1]].z);
	glNormal3f(n[T[6][2]].x, n[T[6][2]].y, n[T[6][2]].z); glTexCoord1f(t[T[6][2]]); glVertex3f(r[T[6][2]].x, r[T[6][2]].y, r[T[6][2]].z);

	glNormal3f(n[T[7][0]].x, n[T[7][0]].y, n[T[7][0]].z); glTexCoord1f(t[T[7][0]]); glVertex3f(r[T[7][0]].x, r[T[7][0]].y, r[T[7][0]].z);
	glNormal3f(n[T[7][1]].x, n[T[7][1]].y, n[T[7][1]].z); glTexCoord1f(t[T[7][1]]); glVertex3f(r[T[7][1]].x, r[T[7][1]].y, r[T[7][1]].z);
	glNormal3f(n[T[7][2]].x, n[T[7][2]].y, n[T[7][2]].z); glTexCoord1f(t[T[7][2]]); glVertex3f(r[T[7][2]].x, r[T[7][2]].y, r[T[7][2]].z);
}

void GLX::tri3(vec3d r[3], vec3f n[3])
{
	vertex3d(r[0], n[0]);
	vertex3d(r[1], n[1]);
	vertex3d(r[2], n[2]);
}

void GLX::tri3(vec3d r[3], vec3f n[3], float t[3])
{
	vertex3d(r[0], n[0], t[0]);
	vertex3d(r[1], n[1], t[1]);
	vertex3d(r[2], n[2], t[2]);
}

void GLX::tri6(vec3d r[6], vec3f n[6], float t[6])
{
	vertex3d(r[0], n[0], t[0]); vertex3d(r[3], n[3], t[3]); vertex3d(r[5], n[5], t[5]);
	vertex3d(r[1], n[1], t[1]); vertex3d(r[4], n[4], t[4]); vertex3d(r[3], n[3], t[3]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[5], n[5], t[5]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[3], n[3], t[3]); vertex3d(r[4], n[4], t[4]); vertex3d(r[5], n[5], t[5]);
}

void GLX::tri7(vec3d r[7], vec3f n[7], float t[7])
{
	vertex3d(r[0], n[0], t[0]); vertex3d(r[3], n[3], t[3]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[1], n[1], t[1]); vertex3d(r[6], n[6], t[6]); vertex3d(r[3], n[3], t[3]);
	vertex3d(r[1], n[1], t[1]); vertex3d(r[4], n[4], t[4]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[2], n[2], t[6]); vertex3d(r[6], n[6], t[6]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[5], n[5], t[5]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[0], n[0], t[0]); vertex3d(r[6], n[6], t[6]); vertex3d(r[5], n[5], t[5]);
}

void GLX::tri10(vec3d r[10], vec3f n[10], float t[10])
{
	vertex3d(r[0], n[0], t[0]); vertex3d(r[3], n[3], t[3]); vertex3d(r[7], n[7], t[7]);
	vertex3d(r[1], n[1], t[1]); vertex3d(r[5], n[5], t[5]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[8], n[8], t[8]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[7], n[7], t[7]); vertex3d(r[3], n[3], t[3]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[3], n[3], t[3]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[4], n[4], t[4]); vertex3d(r[5], n[5], t[5]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[5], n[5], t[5]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[6], n[6], t[6]); vertex3d(r[8], n[8], t[8]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[8], n[8], t[8]); vertex3d(r[7], n[7], t[7]);
}

void GLX::drawLine(double x0, double y0, double x1, double y1)
{
	glBegin(GL_LINES);
	{
		glVertex2d(x0, y0);
		glVertex2d(x1, y1);
	}
	glEnd();
}

void GLX::drawLine(double x0, double y0, double z0, double x1, double y1, double z1)
{
	glBegin(GL_LINES);
	{
		glVertex3d(x0, y0, z0);
		glVertex3d(x1, y1, z1);
	}
	glEnd();
}

void GLX::drawLine(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2)
{
	glBegin(GL_LINE_STRIP);
	{
		glVertex3d(x0, y0, z0);
		glVertex3d(x1, y1, z1);
		glVertex3d(x2, y2, z2);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a sub-divided 4-noded quadrilateral
void GLX::smoothQUAD4(vec3d r[4], vec3f n[4], float t[4], int ndivs)
{
	const int T[2][3] = { { 0,1,2 },{ 2,3,0 } };
	float sa[4], ta[4], h[4];
	for (int i = 0; i<ndivs; ++i)
	{
		sa[0] = -1.f + i*2.f / ndivs;
		sa[1] = -1.f + (i + 1)*2.f / ndivs;
		sa[2] = -1.f + (i + 1)*2.f / ndivs;
		sa[3] = -1.f + i*2.f / ndivs;

		for (int j = 0; j<ndivs; ++j)
		{
			ta[0] = -1.f + j*2.f / ndivs;
			ta[1] = -1.f + j*2.f / ndivs;
			ta[2] = -1.f + (j + 1)*2.f / ndivs;
			ta[3] = -1.f + (j + 1)*2.f / ndivs;

			for (int l = 0; l < 2; ++l)
			{
				for (int k = 0; k < 3; ++k)
				{
					float sak = sa[T[l][k]];
					float tak = ta[T[l][k]];
					h[0] = 0.25f*(1 - sak)*(1 - tak);
					h[1] = 0.25f*(1 + sak)*(1 - tak);
					h[2] = 0.25f*(1 + sak)*(1 + tak);
					h[3] = 0.25f*(1 - sak)*(1 + tak);

					vec3d nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3];
					vec3d rk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(rk.x, rk.y, rk.z);
				}
			}
		}
	}
}

void GLX::smoothQUAD8(vec3d r[8], vec3f n[8], float t[8], int ndivs)
{
	const int T[2][3] = { { 0,1,2 },{ 2,3,0 } };
	float sa[4], ta[4], h[8];
	for (int i = 0; i<ndivs; ++i)
	{
		sa[0] = -1.f + i*2.f / ndivs;
		sa[1] = -1.f + (i + 1)*2.f / ndivs;
		sa[2] = -1.f + (i + 1)*2.f / ndivs;
		sa[3] = -1.f + i*2.f / ndivs;

		for (int j = 0; j<ndivs; ++j)
		{
			ta[0] = -1.f + j*2.f / ndivs;
			ta[1] = -1.f + j*2.f / ndivs;
			ta[2] = -1.f + (j + 1)*2.f / ndivs;
			ta[3] = -1.f + (j + 1)*2.f / ndivs;

			for (int l = 0; l < 2; ++l)
			{
				for (int k = 0; k < 3; ++k)
				{
					float sak = sa[T[l][k]];
					float tak = ta[T[l][k]];
					h[4] = 0.5f*(1 - sak*sak)*(1 - tak);
					h[5] = 0.5f*(1 - tak*tak)*(1 + sak);
					h[6] = 0.5f*(1 - sak*sak)*(1 + tak);
					h[7] = 0.5f*(1 - tak*tak)*(1 - sak);

					h[0] = 0.25f*(1 - sak)*(1 - tak) - (h[4] + h[7])*0.5f;
					h[1] = 0.25f*(1 + sak)*(1 - tak) - (h[4] + h[5])*0.5f;
					h[2] = 0.25f*(1 + sak)*(1 + tak) - (h[5] + h[6])*0.5f;
					h[3] = 0.25f*(1 - sak)*(1 + tak) - (h[6] + h[7])*0.5f;

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7];
					vec3d rk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6] + r[7] * h[7];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6] + t[7] * h[7];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3d(rk.x, rk.y, rk.z);
				}
			}
		}
	}
}

void GLX::smoothQUAD9(vec3d r[9], vec3f n[9], float t[9], int ndivs)
{
	const int T[2][3] = { { 0,1,2 },{ 2,3,0 } };
	float sa[4], ta[4], h[9], R[3], S[3];
	for (int i = 0; i<ndivs; ++i)
	{
		sa[0] = -1.f + i*2.f / ndivs;
		sa[1] = -1.f + (i + 1)*2.f / ndivs;
		sa[2] = -1.f + (i + 1)*2.f / ndivs;
		sa[3] = -1.f + i*2.f / ndivs;

		for (int j = 0; j<ndivs; ++j)
		{
			ta[0] = -1.f + j*2.f / ndivs;
			ta[1] = -1.f + j*2.f / ndivs;
			ta[2] = -1.f + (j + 1)*2.f / ndivs;
			ta[3] = -1.f + (j + 1)*2.f / ndivs;

			for (int l = 0; l < 2; ++l)
			{
				for (int k = 0; k < 4; ++k)
				{
					float sak = sa[T[l][k]];
					float tak = ta[T[l][k]];

					R[0] = 0.5f*sak*(sak - 1.f);
					R[1] = 0.5f*sak*(sak + 1.f);
					R[2] = 1.0f - sak*sak;

					S[0] = 0.5f*tak*(tak - 1.f);
					S[1] = 0.5f*tak*(tak + 1.f);
					S[2] = 1.0f - tak*tak;

					h[0] = R[0] * S[0];
					h[1] = R[1] * S[0];
					h[2] = R[1] * S[1];
					h[3] = R[0] * S[1];
					h[4] = R[2] * S[0];
					h[5] = R[1] * S[2];
					h[6] = R[2] * S[1];
					h[7] = R[0] * S[2];
					h[8] = R[2] * S[2];

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7] + n[8] * h[8];
					vec3d rk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6] + r[7] * h[7] + r[8] * h[8];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6] + t[7] * h[7] + t[8] * h[8];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(rk.x, rk.y, rk.z);
				}
			}
		}
	}
}

void GLX::smoothTRI6(vec3d r[6], vec3f n[6], float t[6], int ndivs)
{
	float sa[2], ta[2], h[8];
	int nj = ndivs;

	int sl[2][3] = { { 0,1,0 },{ 1,1,0 } };
	int tl[2][3] = { { 0,0,1 },{ 0,1,1 } };

	for (int i = 0; i<ndivs; ++i)
	{
		ta[0] = (float)i / ndivs;
		ta[1] = (float)(i + 1) / ndivs;

		for (int j = 0; j<nj; ++j)
		{
			sa[0] = (float)j / ndivs;
			sa[1] = (float)(j + 1) / ndivs;

			for (int k = 0; k<3; ++k)
			{
				float sak = sa[sl[0][k]];
				float tak = ta[tl[0][k]];
				float rak = 1.f - sak - tak;

				h[0] = rak*(2.f*rak - 1.f);
				h[1] = sak*(2.f*sak - 1.f);
				h[2] = tak*(2.f*tak - 1.f);
				h[3] = 4.f*rak*sak;
				h[4] = 4.f*sak*tak;
				h[5] = 4.f*rak*tak;

				vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5];
				vec3d xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5];
				float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5];

				glNormal3f(nk.x, nk.y, nk.z);
				glTexCoord1f(tk);
				glVertex3f(xk.x, xk.y, xk.z);
			}

			if (j != nj - 1)
			{
				for (int k = 0; k<3; ++k)
				{
					float sak = sa[sl[1][k]];
					float tak = ta[tl[1][k]];
					float rak = 1.f - sak - tak;

					h[0] = rak*(2.f*rak - 1.f);
					h[1] = sak*(2.f*sak - 1.f);
					h[2] = tak*(2.f*tak - 1.f);
					h[3] = 4.f*rak*sak;
					h[4] = 4.f*sak*tak;
					h[5] = 4.f*rak*tak;

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5];
					vec3d xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(xk.x, xk.y, xk.z);
				}
			}
		}
		nj -= 1;
	}
}

void GLX::smoothTRI7(vec3d r[7], vec3f n[7], float t[7], int ndivs)
{
	int sl[2][3] = { { 0,1,0 },{ 1,1,0 } };
	int tl[2][3] = { { 0,0,1 },{ 0,1,1 } };

	float sa[2], ta[2], tk, h[7];
	int nj = ndivs;

	for (int i = 0; i<ndivs; ++i)
	{
		ta[0] = (float)i / ndivs;
		ta[1] = (float)(i + 1) / ndivs;

		for (int j = 0; j<nj; ++j)
		{
			sa[0] = (float)j / ndivs;
			sa[1] = (float)(j + 1) / ndivs;

			for (int k = 0; k<3; ++k)
			{
				float sak = sa[sl[0][k]];
				float tak = ta[tl[0][k]];
				float rak = 1.f - sak - tak;

				h[6] = 27.f*rak*sak*tak;
				h[0] = rak*(2.f*rak - 1.f) + h[6] / 9.f;
				h[1] = sak*(2.f*sak - 1.f) + h[6] / 9.f;
				h[2] = tak*(2.f*tak - 1.f) + h[6] / 9.f;
				h[3] = 4.f*rak*sak - 4.f*h[6] / 9.f;
				h[4] = 4.f*sak*tak - 4.f*h[6] / 9.f;
				h[5] = 4.f*tak*rak - 4.f*h[6] / 9.f;

				vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6];
				vec3d xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6];
				float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6];

				glNormal3f(nk.x, nk.y, nk.z);
				glTexCoord1f(tk);
				glVertex3f(xk.x, xk.y, xk.z);
			}

			if (j != nj - 1)
			{
				for (int k = 0; k<3; ++k)
				{
					float sak = sa[sl[1][k]];
					float tak = ta[tl[1][k]];
					float rak = 1.f - sak - tak;

					h[6] = 27.f*rak*sak*tak;
					h[0] = rak*(2.f*rak - 1.f) + h[6] / 9.f;
					h[1] = sak*(2.f*sak - 1.f) + h[6] / 9.f;
					h[2] = tak*(2.f*tak - 1.f) + h[6] / 9.f;
					h[3] = 4.f*rak*sak - 4.f*h[6] / 9.f;
					h[4] = 4.f*sak*tak - 4.f*h[6] / 9.f;
					h[5] = 4.f*tak*rak - 4.f*h[6] / 9.f;

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6];
					vec3d xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(xk.x, xk.y, xk.z);
				}
			}
		}
		nj -= 1;
	}
}

void GLX::smoothTRI10(vec3d r[10], vec3f n[10], float t[10], int ndivs)
{
	float sa[2], ta[2], tk, h[10];
	vec3d nk, xk;
	int i, j, k;
	int nj = ndivs;

	int sl[2][3] = { { 0, 1, 0 },{ 1, 1, 0 } };
	int tl[2][3] = { { 0, 0, 1 },{ 0, 1, 1 } };

	for (i = 0; i<ndivs; ++i)
	{
		ta[0] = (float)i / ndivs;
		ta[1] = (float)(i + 1) / ndivs;

		for (j = 0; j<nj; ++j)
		{
			sa[0] = (float)j / ndivs;
			sa[1] = (float)(j + 1) / ndivs;

			for (k = 0; k<3; ++k)
			{
				float sak = sa[sl[0][k]];
				float tak = ta[tl[0][k]];
				float rak = 1.f - sak - tak;

				h[0] = 0.5f*(3.f*rak - 1.f)*(3.f*rak - 2.f)*rak;
				h[1] = 0.5f*(3.f*sak - 1.f)*(3.f*sak - 2.f)*sak;
				h[2] = 0.5f*(3.f*tak - 1.f)*(3.f*tak - 2.f)*tak;
				h[3] = 9.f / 2.f*(3.f*rak - 1.f)*rak*sak;
				h[4] = 9.f / 2.f*(3.f*sak - 1.f)*rak*sak;
				h[5] = 9.f / 2.f*(3.f*sak - 1.f)*sak*tak;
				h[6] = 9.f / 2.f*(3.f*tak - 1.f)*sak*tak;
				h[7] = 9.f / 2.f*(3.f*rak - 1.f)*rak*tak;
				h[8] = 9.f / 2.f*(3.f*tak - 1.f)*rak*tak;
				h[9] = 27.f*rak*sak*tak;

				nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7] + n[8] * h[8] + n[9] * h[9];
				xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6] + r[7] * h[7] + r[8] * h[8] + r[9] * h[9];
				tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6] + t[7] * h[7] + t[8] * h[8] + t[9] * h[9];

				glNormal3f(nk.x, nk.y, nk.z);
				glTexCoord1f(tk);
				glVertex3f(xk.x, xk.y, xk.z);
			}

			if (j != nj - 1)
			{
				for (k = 0; k<3; ++k)
				{
					float sak = sa[sl[1][k]];
					float tak = ta[tl[1][k]];
					float rak = 1.f - sak - tak;

					h[0] = 0.5f*(3.f*rak - 1.f)*(3.f*rak - 2.f)*rak;
					h[1] = 0.5f*(3.f*sak - 1.f)*(3.f*sak - 2.f)*sak;
					h[2] = 0.5f*(3.f*tak - 1.f)*(3.f*tak - 2.f)*tak;
					h[3] = 9.f / 2.f*(3.f*rak - 1.f)*rak*sak;
					h[4] = 9.f / 2.f*(3.f*sak - 1.f)*rak*sak;
					h[5] = 9.f / 2.f*(3.f*sak - 1.f)*sak*tak;
					h[6] = 9.f / 2.f*(3.f*tak - 1.f)*sak*tak;
					h[7] = 9.f / 2.f*(3.f*rak - 1.f)*rak*tak;
					h[8] = 9.f / 2.f*(3.f*tak - 1.f)*rak*tak;
					h[9] = 27.f*rak*sak*tak;

					nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7] + n[8] * h[8] + n[9] * h[9];
					xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6] + r[7] * h[7] + r[8] * h[8] + r[9] * h[9];
					tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6] + t[7] * h[7] + t[8] * h[8] + t[9] * h[9];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(xk.x, xk.y, xk.z);
				}
			}
		}
		nj -= 1;
	}
}
