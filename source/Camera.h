#pragma once

#include "Vector.h"
#include "Transform.h"
#include "Patch.h"

class Camera {

	private:
		Vector3f eye;			// pozice kamery
		Vector3f target;		// smer pohledu
		Vector3f up;			// up vektor kamery
		float angle_horiz;
		float angle_vert;
		
	public:

		static enum PatchLook {
			PATCH_LOOK_FRONT = 0,
			PATCH_LOOK_UP,
			PATCH_LOOK_DOWN,
			PATCH_LOOK_LEFT,
			PATCH_LOOK_RIGHT
		};

		Camera(void);
		void Reset();
		void lookFromPatch(Patch* p, PatchLook dir);
		void Move(float x, float y, float z);		
		void Aim(float vertical_angle, float horizontal_angle);
		Matrix4f GetMatrix();
		void DebugDump();

};

