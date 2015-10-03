#include "Camera.h"
#include <math.h>

Camera::Camera()
{
	Init();
	Update();
}

Camera::Camera(float x_=0, float y_=0, float z_=6)
{
	x = x_; y = y_; z = z_; 
	pitch = 0; roll = 0; yaw = 0;
	forwardx =0; forwardy = 0; forwardz = 0;
	upx =0; upy = 1; upz = 0;
}
void Camera::Init()
{
	x = 0; y = 0; z = 6; 
	pitch = 0; roll = 0; yaw = 0;
	forwardx =0; forwardy = 0; forwardz = 0;
	upx =0; upy = 1; upz = 0;
}
void Camera::Reset()
{
	Init();
}
void Camera::Update()
{
	float CosP, CosR, CosY;
	float SinP, SinR, SinY;

	CosP = cosf(pitch*3.1415/180);
	CosR = cosf(roll*3.1415/180);
	CosY = cosf(yaw*3.1415/180);
	SinP = sinf(pitch*3.1415/180);
	SinR = sinf(roll*3.1415/180);
	SinY = sinf(yaw*3.1415/180);

	//forward vector
	forwardx = SinY * CosP * 360;
	forwardy = SinP * 360;
	forwardz = CosP * -CosY * 360;
	
	//up vector
	upx = -CosY * SinR - SinY * SinP * CosR;
	upy = CosP * CosR;
	upz = -SinY * SinR - SinP * CosR * -CosY;

	//side vector(right)
	sidex = (forwardy * upz) - (forwardz * upy);
	sidey = (forwardx * upz) - (forwardz * upx);
	sidez = (forwardx * upy) - (forwardy * upx);
}


void Camera::MoveX(float movex)
{
	//x += movex;
}

void Camera::MoveY(float movey)
{
	y += movey;
}

void Camera::MoveZ(float movez)
{
	//z += movez;
}
void Camera::MoveLeft()
{
	x -= sidex/36000;
	//y -= sidey/36000;
	z -= sidez/36000;
}
void Camera::MoveRight()
{
	x += sidex/36000;
	//y += sidey/36000;
	z += sidez/36000;
}
void Camera::MoveForward()
{
	x += forwardx/36000;
	//y += forwardy/36000;
	z += forwardz/36000;
}
void Camera::MoveBackward()
{
	x -= forwardx/36000;
	//y -= forwardy/36000;
	z -= forwardz/36000;
}
void Camera::RotateX(float n)
{
	yaw += n;
}

void Camera::RotateY(float n)
{
	pitch += n;
}

void Camera::RotateZ(float n)
{
	roll += n;
}
void Camera::LeanRight()
{
	if(roll>-25)
	{
		RotateZ(-0.1);
		x += sidex/72000;
		z += sidez/72000;
	}
}
void Camera::LeanLeft()
{
	if(roll<25)
	{
		RotateZ(0.1);
		x -= sidex/72000;
		z -= sidez/72000;
	}
}
void Camera::LeanCentre()
{
	if(roll>0)
	{
		RotateZ(-0.1);
		x += sidex/72000;
		z += sidez/72000;
	}
	else if(roll<0)
	{
		RotateZ(0.1);
		x -= sidex/72000;
		z -= sidez/72000;
	}
}

