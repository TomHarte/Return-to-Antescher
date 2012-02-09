#include "cubedrawing.h"
#include "ConvexObject.h"

void DrawInvertedCube()
{
	glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glVertex3f(-0.5, -0.5, -0.5);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(-0.5, 0.5, -0.5);

		glNormal3f(0, 0, -1);
		glVertex3f(0.5, -0.5, 0.5);
		glVertex3f(-0.5, -0.5, 0.5);
		glVertex3f(-0.5, 0.5, 0.5);
		glVertex3f(0.5, 0.5, 0.5);

		glNormal3f(-1, 0, 0);
		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(0.5, -0.5, 0.5);
		glVertex3f(0.5, 0.5, 0.5);

		glNormal3f(1, 0, 0);
		glVertex3f(-0.5, -0.5, -0.5);
		glVertex3f(-0.5, 0.5, -0.5);
		glVertex3f(-0.5, 0.5, 0.5);
		glVertex3f(-0.5, -0.5, 0.5);

		glNormal3f(0, -1, 0);
		glVertex3f(-0.5, 0.5, -0.5);
		glVertex3f(0.5, 0.5, -0.5);
		glVertex3f(0.5, 0.5, 0.5);
		glVertex3f(-0.5, 0.5, 0.5);

		glNormal3f(0, 1, 0);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(-0.5, -0.5, -0.5);
		glVertex3f(-0.5, -0.5, 0.5);
		glVertex3f(0.5, -0.5, 0.5);

	glEnd();
}

void DrawCube(Uint8 LFlags, Uint8 RFlags, Uint8 TFlags, Uint8 BFlags, Uint8 SFlags)
{
	glBegin(GL_QUADS);		
		if(!(SFlags&4))
		{
			glNormal3f(0, 0, -1);
			glVertex3f(-0.5, -0.5, -0.5);
			glVertex3f(0.5, -0.5, -0.5);
			glVertex3f(0.5, 0.5, -0.5);
			glVertex3f(-0.5, 0.5, -0.5);
		}

		if(!(SFlags&1))
		{
			glNormal3f(0, 0, 1);
			glVertex3f(0.5, -0.5, 0.5);
			glVertex3f(-0.5, -0.5, 0.5);
			glVertex3f(-0.5, 0.5, 0.5);
			glVertex3f(0.5, 0.5, 0.5);
		}

		if(!(RFlags&1))
		{
			glNormal3f(1, 0, 0);
			glVertex3f(0.5, 0.5, -0.5);
			glVertex3f(0.5, -0.5, -0.5);
			glVertex3f(0.5, -0.5, 0.5);
			glVertex3f(0.5, 0.5, 0.5);
		}

		if(!(LFlags&1))
		{
			glNormal3f(-1, 0, 0);
			glVertex3f(-0.5, -0.5, -0.5);
			glVertex3f(-0.5, 0.5, -0.5);
			glVertex3f(-0.5, 0.5, 0.5);
			glVertex3f(-0.5, -0.5, 0.5);
		}

		if(!(BFlags&1))
		{
			glNormal3f(0, 1, 0);
			glVertex3f(-0.5, 0.5, -0.5);
			glVertex3f(0.5, 0.5, -0.5);
			glVertex3f(0.5, 0.5, 0.5);
			glVertex3f(-0.5, 0.5, 0.5);
		}

		if(!(TFlags&1))
		{
			glNormal3f(0, -1, 0);
			glVertex3f(0.5, -0.5, -0.5);
			glVertex3f(-0.5, -0.5, -0.5);
			glVertex3f(-0.5, -0.5, 0.5);
			glVertex3f(0.5, -0.5, 0.5);
		}

	glEnd();
}

void DrawCubeShadowVolume(float *LightPos, float *cubepos, float xl, float yl, float zl)
{
	glPushMatrix();
		glTranslatef(cubepos[0], cubepos[1], cubepos[2]);

		float LightOffset[3] = {LightPos[0] - cubepos[0], LightPos[1] - cubepos[1], LightPos[2] - cubepos[2]};
		CubeObject.ProjectShadow(LightOffset, xl, yl, zl);
	glPopMatrix();
	return;
}

void DrawSphereShadowVolume(float *LightPos, float *cubepos, float xl, float yl, float zl)
{
	glPushMatrix();
		glTranslatef(cubepos[0], cubepos[1], cubepos[2]);

		float LightOffset[3] = {LightPos[0] - cubepos[0], LightPos[1] - cubepos[1], LightPos[2] - cubepos[2]};
		SphereObject.ProjectShadow(LightOffset, xl, yl, zl);
	glPopMatrix();
	return;
}

