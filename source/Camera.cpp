#include "Camera.h"


Camera::Camera(void) {
	Reset();
}


void Camera::Reset() {
	eye = Vector3f(2.78f, 2.73f, -2.5f);	
	//eye = Vector3f(2.78f, 2.73f, 0.025f);
	up = Vector3f(0.0f, 1.0f, 0.0f);		
	target = Vector3f(0.0f, 0.0f, 1.0f);
	angle_horiz = 0.0f;
	angle_vert = 0.0f;
	isObserving = false;
	observeCenter = Vector3f(0.0f, 0.0f, 0.0f);
	observeRadius = 3.0f;
	observeSpeed = 0.7f;
	observeTimer.ResetTimer();
}


void Camera::lookFromPatch(Patch* p, PatchLook dir, Vector3f v_offset) {
	eye = p->getCenter() + v_offset;

	Vector3f normal = p->getNormal(); // vychozi normala - smer pohledu FRONT
	Vector3f patchUp = p->getUp();	// pomocny UP vektor patche

	// podle smeru pohledu upravit normalu a up vektor
	switch (dir) {
		case PATCH_LOOK_FRONT:
			target = normal;
			up = patchUp;
			break;

		case PATCH_LOOK_UP:
			target = patchUp;
			up = -normal;
			break;

		case PATCH_LOOK_DOWN:
			target = -patchUp;
			up = normal;
			break;

		case PATCH_LOOK_LEFT:
			target = -normal.v_Cross(patchUp);
			up = patchUp;
			break;

		case PATCH_LOOK_RIGHT:
			target = normal.v_Cross(patchUp);
			up = patchUp;
			break;
	}
}

void Camera::lookFromPatch(Patch* p, PatchLook dir) {
	eye = p->getCenter();

	Vector3f normal = p->getNormal(); // vychozi normala - smer pohledu FRONT
	Vector3f patchUp = p->getUp();	// pomocny UP vektor patche

	// podle smeru pohledu upravit normalu a up vektor
	switch (dir) {
		case PATCH_LOOK_FRONT:
			target = normal;
			up = patchUp;
			break;

		case PATCH_LOOK_UP:
			target = patchUp;
			up = -normal;
			break;

		case PATCH_LOOK_DOWN:
			target = -patchUp;
			up = normal;
			break;

		case PATCH_LOOK_LEFT:
			target = -normal.v_Cross(patchUp);
			up = patchUp;
			break;

		case PATCH_LOOK_RIGHT:
			target = normal.v_Cross(patchUp);
			up = patchUp;
			break;
	}
}


void Camera::Move(float x, float y, float z) {
	
	// posunout o z-nasobek smeru pohledu
	Vector3f dir = target;
	dir.Normalize();
	eye += dir * z;

	// posunout do strany o x-nasobek right vektoru
	Vector3f r = target;
	r.Normalize();
	r = -r.Cross(up);
	eye += r * x;
}


void Camera::Aim(float vertical_angle, float horizontal_angle) {		

	// kamera se bude otacet po jednotkove kouli
	angle_horiz += horizontal_angle;
	angle_vert += vertical_angle;

	float f_pi_2 = f_pi / 2;

	// osetrit pretaceni pres hlavu a pod nohama
	// pridat korekci, protoze pokud je target shodny s up vektorem, ma LookAt nedefinovane chovani
	if (angle_vert >= f_pi_2)
		angle_vert = f_pi_2 - 0.0001f;
	if (angle_vert <= -f_pi_2)
		angle_vert = -f_pi_2 + 0.0001f;

	// uhly sferickych souradnic jsou velke a male fi
	float l_phi = f_pi_2 - angle_horiz;
	float u_phi = f_pi_2 - angle_vert;	

	target = Vector3f(
		sin(u_phi) * cos(l_phi),
		cos(u_phi),
		sin(u_phi) * sin(l_phi)
	);
}


Matrix4f Camera::GetMatrix() {
	if (isObserving) {		
		/*
		target = observeCenter;
		eye = Vector3f( float(sin(observeTimer.f_Time() * observeSpeed)), target.y, 
			float(cos(observeTimer.f_Time() * observeSpeed)) ) * observeRadius;
			*/
		target = observeCenter - eye;
		target.Normalize();
		//target.y = cos(observeTimer.f_Time());
		//target.z = 1.0f;

//		Move(1, 0, 0);
	}

	Matrix4f m;
	CGLTransform::LookAt(m, eye, target + eye, up);
	return m;	
}


void Camera::DebugDump() {
	printf("----------------------------------------------------\n");
	printf("pos %f %f %f\n", eye.x, eye.y, eye.z);
	printf("target %f %f %f\n", target.x, target.y, target.z);
	printf("up  %f %f %f\n", up.x, up.y, up.z);
}


void Camera::toggleObserve() {
	isObserving = !isObserving;

	if (isObserving)
		observeCenter = Vector3f(2.78f, 2.73f, 2.73f);
}