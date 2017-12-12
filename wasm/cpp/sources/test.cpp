int g_x; // global variable g_x
const int g_y(2); // global variable g_y

extern int get_value_test(int);

void doSomething()
{
    // global variables can be seen and used everywhere in program
    g_x += get_value_test(g_y);
}

void doSomethingElse()
{
    // global variables can be seen and used everywhere in program
    g_x *= g_y;
}


struct Vec3 {
	float x;
	float y;
	float z;

	Vec3(float x, float y, float z)
	: x(x), y(y), z(z) { }

	virtual ~Vec3() { }

	virtual float sq_dist() const{
		return x * x + y * y + z * z;
	}

	virtual bool is_solid() const {
		return false;
	}
};

struct VecSize : Vec3 {
	float size;

	VecSize(float x, float y, float z, float size)
	: Vec3(x, y, z), size(size) { }

	virtual ~VecSize() { }

	virtual float sq_dist() const{
		return Vec3::sq_dist() + size * size;
	}

	virtual bool is_solid() const {
		return true;
	}

};
//float Vec3::sq_dist() const
Vec3 makeVec3(float x, float y, float z) {
	return Vec3 ( x, y, z );
}
//float Vec3::sq_dist() const
VecSize makeVec3Size(float x, float y, float z, float size) {
	return VecSize ( x, y, z, size );
}

//float Vec3::sq_dist() const
float sq_dist(const Vec3 &vec) {
	return vec.sq_dist();
}

