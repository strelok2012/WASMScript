
struct Vec3 {
	float x;
	float y;
	float z;

	Vec3(float x, float y, float z)
	: x(x), y(y), z(z) { }

	~Vec3() { }

	float sq_dist() const{
		return x * x + y * y + z * z;
	}

};

//float Vec3::sq_dist() const
Vec3 makeVec3(float x, float y, float z) {
	return Vec3 ( x, y, z );
}

//float Vec3::sq_dist() const
float sq_dist(const Vec3 &vec) {
	return vec.sq_dist();
}
