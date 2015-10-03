
class Camera
{
	public:
	Camera::Camera();
	Camera::Camera(float x_, float y_, float z_);

	void Camera::Init();
	void Camera::Update();
	void Camera::Reset();

	void Camera::MoveX(float movex);
	void Camera::MoveY(float movey);
	void Camera::MoveZ(float n);

	void Camera::MoveLeft();
	void Camera::MoveRight();
	void Camera::MoveForward();
	void Camera::MoveBackward();

	void Camera::RotateX(float n);
	void Camera::RotateY(float n);
	void Camera::RotateZ(float n);

	void Camera::LeanRight();
	void Camera::LeanLeft();
	void Camera::LeanCentre();

	float x, y, z;
	float pitch, roll, yaw;
	float forwardx, forwardy, forwardz;
	float upx, upy, upz;
	float sidex, sidey, sidez;

	private:
	//I really should make variables private and set up accessors later BRO!!!!

};