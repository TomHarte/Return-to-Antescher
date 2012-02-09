#ifndef __CONVEXOBJECT_H
#define __CONVEXOBJECT_H

#include <stdio.h>

class ConvexObject
{
	public:
		ConvexObject();
		~ConvexObject();

		void SetCube();
		void SetSphere();
		void FlipNormals();

		void ProjectShadow(float *LightPos, float lx = 1, float ly = 1, float lz = 1);
		void Draw(float lx = 1, float ly = 1, float lz = 1);

		struct V
		{
			float Pos[3];
			float Scaled[3];
			float Projected[3];
			float MiniProjected[3];
			float Normal[3];
		} *Verts;
		struct E
		{
			int Verts[2];
			int Visible;
		} *Edges;
		struct F
		{
			int Verts[4];
			int Edges[4];
			float Normal[3];
			bool Visible;
		} *Faces;

		int NumFaces, NumEdges, NumVerts;
		
	private:
		int GetEdge(int v1, int v2);
		void DoDraw();
		void GenerateEdges();
		void GenerateNormals();
		bool Smooth;
};

extern ConvexObject CubeObject, SphereObject;

#endif
