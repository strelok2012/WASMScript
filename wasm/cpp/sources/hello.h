
typedef struct {
	char valueChar;
	short valueShort;
	int valueInt;
	long valueLong;
	long long valueLongLong;

	unsigned char valueUnsignedChar;
	unsigned short valueUnsignedShort;
	unsigned int valueUnsignedInt;
	unsigned long valueUnsignedLong;
	unsigned long long valueUnsignedLongLong;

	float valueFloat;
	double valueDouble;

	char *valueCharPointer;
	char valueCharArray[25];
	int valueIntArray[3];
} test_struct;


int header_function(int i_test);

