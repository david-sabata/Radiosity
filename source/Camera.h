#pragma once

#include "Vector.h"
#include "Transform.h"

class Camera {

	private:
		Vector3f eye;			// pozice kamery
		Vector3f target;		// smer pohledu
		Vector3f up;			// up vektor kamery
		float angle_horiz;
		float angle_vert;
		
	public:
		Camera(void);
		void Reset();
		void Move(float x, float y, float z);		
		void Aim(float vertical_angle, float horizontal_angle);
		Matrix4f GetMatrix();
		void DebugDump();

};

