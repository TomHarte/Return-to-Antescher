#include "ConvexObject.h"
#include "ebgf.h"
#include "Gameplay.h"
#include <math.h>

ConvexObject::ConvexObject()
{
	Verts = NULL;
	Edges = NULL;
	Faces = NULL;
}

ConvexObject::~ConvexObject()
{
	delete[] Verts; Verts = NULL;
	delete[] Edges; Edges = NULL;
	delete[] Faces; Faces = NULL;
}

void ConvexObject::SetSphere()
{
	Smooth = true;

	/* set up verts - do 8x16 vertices = 128 total*/
	NumVerts = 256;
	Verts = new V[256];
	int x, y;
	for(x = 0; x < 16; x++)
	{
		float xang = ((float)x / 8.0f) * 3.141592654f;
		for(y = 0; y < 16; y++)
		{
			int idx = (y << 4) | x;
			float yang = ((float)y / 15.0f) * 3.141592654f;

			Verts[idx].Pos[0] = (float)(0.5f*sin(xang)*sin(yang));
			Verts[idx].Pos[1] = (float)(0.5f*cos(xang)*sin(yang));
			Verts[idx].Pos[2] = (float)(0.5f*cos(yang));
		}
	}

	/* set up faces */
	Faces = new F[240];
	NumFaces = 16 * 15;
	int c = NumFaces;
	while(c--)
	{
		int yband, xpos1, xpos2;
		xpos1 = c&15;
		yband = (c >> 4);
		xpos2 = (xpos1+1)&15;

		Faces[c].Verts[3] = xpos1 | (yband << 4);
		Faces[c].Verts[2] = xpos2 | (yband << 4);
		Faces[c].Verts[1] = xpos2 | ((yband+1) << 4);
		Faces[c].Verts[0] = xpos1 | ((yband+1) << 4);
	}

	/* collect edges */
	Edges = new E[496];
	GenerateEdges();
	GenerateNormals();
}

void ConvexObject::GenerateEdges()
{
	NumEdges = 0;
	int c = NumFaces;
	while(c--)
	{
		int ic = 4;
		while(ic--)
			Faces[c].Edges[ic] = GetEdge(Faces[c].Verts[ic], Faces[c].Verts[(ic+1)%4]);
	}
}

void ConvexObject::GenerateNormals()
{
	/* set to 0 at verts */
	int c = NumVerts;
	while(c--)
		Verts[c].Normal[0] = Verts[c].Normal[1] = Verts[c].Normal[2] = 0;

	c = NumFaces;
	while(c--)
	{
		float Vec1[3], Vec2[3];

		Vec1[0] = Verts[ Faces[c].Verts[2] ].Pos[0] - Verts[ Faces[c].Verts[1] ].Pos[0];
		Vec1[1] = Verts[ Faces[c].Verts[2] ].Pos[1] - Verts[ Faces[c].Verts[1] ].Pos[1];
		Vec1[2] = Verts[ Faces[c].Verts[2] ].Pos[2] - Verts[ Faces[c].Verts[1] ].Pos[2];

		Vec2[0] = Verts[ Faces[c].Verts[1] ].Pos[0] - Verts[ Faces[c].Verts[0] ].Pos[0];
		Vec2[1] = Verts[ Faces[c].Verts[1] ].Pos[1] - Verts[ Faces[c].Verts[0] ].Pos[1];
		Vec2[2] = Verts[ Faces[c].Verts[1] ].Pos[2] - Verts[ Faces[c].Verts[0] ].Pos[2];

		float l1 = (float)sqrt(Vec1[0]*Vec1[0] + Vec1[1]*Vec1[1] + Vec1[2]*Vec1[2]);
		float l2 = (float)sqrt(Vec2[0]*Vec2[0] + Vec2[1]*Vec2[1] + Vec2[2]*Vec2[2]);
		if(fabs(l1) < 0.01 || fabs(l2) < 0.01)
		{
			Vec1[0] = Verts[ Faces[c].Verts[3] ].Pos[0] - Verts[ Faces[c].Verts[2] ].Pos[0];
			Vec1[1] = Verts[ Faces[c].Verts[3] ].Pos[1] - Verts[ Faces[c].Verts[2] ].Pos[1];
			Vec1[2] = Verts[ Faces[c].Verts[3] ].Pos[2] - Verts[ Faces[c].Verts[2] ].Pos[2];

			Vec2[0] = Verts[ Faces[c].Verts[2] ].Pos[0] - Verts[ Faces[c].Verts[1] ].Pos[0];
			Vec2[1] = Verts[ Faces[c].Verts[2] ].Pos[1] - Verts[ Faces[c].Verts[1] ].Pos[1];
			Vec2[2] = Verts[ Faces[c].Verts[2] ].Pos[2] - Verts[ Faces[c].Verts[1] ].Pos[2];
		}

		Faces[c].Normal[0] = -(Vec1[1]*Vec2[2] - Vec2[1]*Vec1[2]);
		Faces[c].Normal[1] = -(Vec1[2]*Vec2[0] - Vec2[2]*Vec1[0]);
		Faces[c].Normal[2] = -(Vec1[0]*Vec2[1] - Vec2[0]*Vec1[1]);

		float mul = 1.0f / (float)sqrt(Faces[c].Normal[0]*Faces[c].Normal[0] + Faces[c].Normal[1]*Faces[c].Normal[1] + Faces[c].Normal[2]*Faces[c].Normal[2]);
		Faces[c].Normal[0] *= mul;
		Faces[c].Normal[1] *= mul;
		Faces[c].Normal[2] *= mul;
		
		/* and add on to vertices... */
		int ec = 4;
		while(ec--)
		{
			Verts[ Faces[c].Verts[ec] ].Normal[0] += Faces[c].Normal[0];
			Verts[ Faces[c].Verts[ec] ].Normal[1] += Faces[c].Normal[1];
			Verts[ Faces[c].Verts[ec] ].Normal[2] += Faces[c].Normal[2];
		}
	}

	/* normalise at verts */
	c = NumVerts;
	while(c--)
	{
		float mul = -1.0f / (float)sqrt(Verts[c].Normal[0]*Verts[c].Normal[0] + Verts[c].Normal[1]*Verts[c].Normal[1] + Verts[c].Normal[2]*Verts[c].Normal[2]);
		Verts[c].Normal[0] *= mul;
		Verts[c].Normal[1] *= mul;
		Verts[c].Normal[2] *= mul;
	}
}

void ConvexObject::SetCube()
{
	Smooth = false;

	/* set up verts */
	NumVerts = 8;
	Verts = new V[8];
	int c = 8;
	while(c--)
	{
		Verts[c].Pos[0] = (c&1) ? -0.5f : 0.5f;
		Verts[c].Pos[1] = (c&2) ? -0.5f : 0.5f;
		Verts[c].Pos[2] = (c&4) ? -0.5f : 0.5f;
	}

	/* set up faces */
	Faces = new F[6];
	NumFaces = 6;

	/* z high, x and y vary */
	Faces[0].Verts[3] = 4;
	Faces[0].Verts[2] = 4 | 2;
	Faces[0].Verts[1] = 4 | 2 | 1;
	Faces[0].Verts[0] = 4 | 1;

	/* z low, x and y vary */
	Faces[1].Verts[0] = 0;
	Faces[1].Verts[1] = 2;
	Faces[1].Verts[2] = 2 | 1;
	Faces[1].Verts[3] = 1;

	/* x high, y and z vary */
	Faces[2].Verts[0] = 1;
	Faces[2].Verts[1] = 1 | 2;
	Faces[2].Verts[2] = 1 | 2 | 4;
	Faces[2].Verts[3] = 1 | 4;

	/* x low, y and z vary */
	Faces[3].Verts[3] = 0;
	Faces[3].Verts[2] = 2;
	Faces[3].Verts[1] = 2 | 4;
	Faces[3].Verts[0] = 4;

	/* y high, x and z vary */
	Faces[4].Verts[3] = 2;
	Faces[4].Verts[2] = 2 | 1;
	Faces[4].Verts[1] = 2 | 1 | 4;
	Faces[4].Verts[0] = 2 | 4;

	/* y low, x and z vary */
	Faces[5].Verts[0] = 0;
	Faces[5].Verts[1] = 1;
	Faces[5].Verts[2] = 1 | 4;
	Faces[5].Verts[3] = 4;

	/* collect edges */
	Edges = new E[12];
	GenerateEdges();
	GenerateNormals();
}

int ConvexObject::GetEdge(int v1, int v2)
{
	int s = 0;
	while(s < NumEdges)
	{
		if(
			(Edges[s].Verts[0] == v1 && Edges[s].Verts[1] == v2) ||
			(Edges[s].Verts[1] == v1 && Edges[s].Verts[0] == v2)
		)
			return s;
		s++;
	}

	Edges[NumEdges].Verts[0] = v1;
	Edges[NumEdges].Verts[1] = v2;

	NumEdges++;
	return NumEdges-1;
}

void ConvexObject::Draw(float lx, float ly, float lz)
{
	/* apply scaling */
	int c = NumVerts;
	while(c--)
	{
		Verts[c].Scaled[0] = Verts[c].Pos[0] * lx;
		Verts[c].Scaled[1] = Verts[c].Pos[1] * ly;
		Verts[c].Scaled[2] = Verts[c].Pos[2] * lz;
	}

	/* draw all faces */
	glBegin(GL_QUADS);
		c = NumFaces;
		if(Smooth)
		{
			while(c--)
			{
				// internal normal format is the opposite to OpenGLs
				glNormal3fv( Verts[ Faces[c].Verts[0] ].Normal);	glVertex3fv( Verts[ Faces[c].Verts[0] ].Scaled);
				glNormal3fv( Verts[ Faces[c].Verts[1] ].Normal);	glVertex3fv( Verts[ Faces[c].Verts[1] ].Scaled);
				glNormal3fv( Verts[ Faces[c].Verts[2] ].Normal);	glVertex3fv( Verts[ Faces[c].Verts[2] ].Scaled);
				glNormal3fv( Verts[ Faces[c].Verts[3] ].Normal);	glVertex3fv( Verts[ Faces[c].Verts[3] ].Scaled);
			}
		}
		else
		{
			while(c--)
			{
				// internal normal format is the opposite to OpenGLs
				glNormal3f( -Faces[c].Normal[0], -Faces[c].Normal[1], -Faces[c].Normal[2] );
				glVertex3fv( Verts[ Faces[c].Verts[0] ].Scaled);
				glVertex3fv( Verts[ Faces[c].Verts[1] ].Scaled);
				glVertex3fv( Verts[ Faces[c].Verts[2] ].Scaled);
				glVertex3fv( Verts[ Faces[c].Verts[3] ].Scaled);
			}
		}
	glEnd();
}

void ConvexObject::ProjectShadow(float *Light, float lx, float ly, float lz)
{
	/* quick distance check */
	float Distance = (float)sqrt(Light[0]*Light[0] + Light[1]*Light[1] + Light[2]*Light[2]);
	if(Distance > LIGHT_WIDTH) return;

	/* clear all edge visibility flags */
	int c = NumEdges;
	while(c--)
		Edges[c].Visible = 0;
		
	/* apply scaling */
	c = NumVerts;
	while(c--)
	{
		Verts[c].Scaled[0] = Verts[c].Pos[0] * lx;
		Verts[c].Scaled[1] = Verts[c].Pos[1] * ly;
		Verts[c].Scaled[2] = Verts[c].Pos[2] * lz;
	}

	/* decide which sides are visible, and set visibility at edges */
	c = NumFaces;
	while(c--)
	{
		if(
			Faces[c].Visible =
			(
				(Verts[ Faces[c].Verts[0] ].Scaled[0] - Light[0])* Faces[c].Normal[0] +
				(Verts[ Faces[c].Verts[1] ].Scaled[1] - Light[1])* Faces[c].Normal[1] +
				(Verts[ Faces[c].Verts[2] ].Scaled[2] - Light[2])* Faces[c].Normal[2]
				> 0
			)
		)
		{
			int ec = 4;
			while(ec--)
				Edges[ Faces[c].Edges[ec]].Visible++;
		}
	}

	/* make projected set of vertices */
	float ProjectDistance = 4.0f * ((float)LIGHT_WIDTH - Distance) / Distance;
	c = NumVerts;
	while(c--)
	{
		Verts[c].Projected[0] = Verts[c].Scaled[0] + (Verts[c].Scaled[0] - Light[0])*ProjectDistance;
		Verts[c].Projected[1] = Verts[c].Scaled[1] + (Verts[c].Scaled[1] - Light[1])*ProjectDistance;
		Verts[c].Projected[2] = Verts[c].Scaled[2] + (Verts[c].Scaled[2] - Light[2])*ProjectDistance;
	}

	// render back, if depth test fails increment, otherwise do nothing
	glCullFace(GL_FRONT);
	glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
	DoDraw();

	// render front, if depth test fails decrement, otherwise do nothing
	glCullFace(GL_BACK);
	glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
	DoDraw();
}
	
void ConvexObject::DoDraw()
{
	/* draw visible faces and fan out visible edges */
	glBegin(GL_QUADS);
		int c = NumFaces;
		while(c--)
		{
			if(Faces[c].Visible)
			{
				int ec = 4;
				while(ec--)
					if(Edges[ Faces[c].Edges[ec] ].Visible == 1)
					{
						/* genuine edge */
						glVertex3fv( Verts[ Faces[c].Verts[ec] ].Scaled);
						glVertex3fv( Verts[ Faces[c].Verts[ec] ].Projected);
						glVertex3fv( Verts[ Faces[c].Verts[(ec+1)&3] ].Projected);
						glVertex3fv( Verts[ Faces[c].Verts[(ec+1)&3] ].Scaled);
					}
			}
			else
			{
				glVertex3fv( Verts[ Faces[c].Verts[3] ].Scaled);
				glVertex3fv( Verts[ Faces[c].Verts[2] ].Scaled);
				glVertex3fv( Verts[ Faces[c].Verts[1] ].Scaled);
				glVertex3fv( Verts[ Faces[c].Verts[0] ].Scaled);
				glVertex3fv( Verts[ Faces[c].Verts[0] ].Projected);
				glVertex3fv( Verts[ Faces[c].Verts[1] ].Projected);
				glVertex3fv( Verts[ Faces[c].Verts[2] ].Projected);
				glVertex3fv( Verts[ Faces[c].Verts[3] ].Projected);
			}
		}
	glEnd();
}

ConvexObject CubeObject, SphereObject;

void ConvexObject::FlipNormals()
{
	int c = NumVerts;
	while(c--)
	{
		Verts[c].Normal[0] *= -1.0f;
		Verts[c].Normal[1] *= -1.0f;
		Verts[c].Normal[2] *= -1.0f;
	}
}
