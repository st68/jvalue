
#include <iostream>
#include <sstream>

#include "jvalue.h"

using namespace std;

int
main()
{
#if 1
	cout << "constructors" << endl;

	jvalue V1;
	cout << V1 << endl;
	jvalue V2("abc");
	cout << V2 << endl;
	jvalue V3(99);
	cout << V3 << endl;
	jvalue V4(1.23);
	cout << V4 << endl;
	jvalue V5(true);
	cout << V5 << endl;

	cout << endl;
	cout << "operator=( value )" << endl;

	jvalue V;
	V = "abc";
	cout << V << endl;
	V = 99;
	cout << V << endl;
	V = 1.23;
	cout << V << endl;
	V = true;
	cout << V << endl;

	cout << endl;
	cout << "operator=( jvalue& )" << endl;

	V = V1;
	cout << V << endl;
	V = V2;
	cout << V << endl;
	V = V3;
	cout << V << endl;
	V = V4;
	cout << V << endl;
	V = V5;
	cout << V << endl;

	cout << endl;
	cout << "getting values" << endl;

	cout << V2.String() << endl;
	cout << V3.Integer() << endl;
	cout << V4.Double() << endl;
	cout << V5.Bool() << " (1 == true)" << endl;

	cout << endl;
	cout << "testing arrays" << endl;

	jvalue A;
	cout << A[(size_t)0] << endl;
	cout.flush();

	A[(size_t)0] = "123";
	A[1] = 123;
	A[2] = 1.23;
	A[3] = true;
	A.push_back( "PB1" );
	A.push_back( 987 );
	A.push_back( 12.34 );
	A.push_back( false );
	cout << A << endl;

	cout << endl;
	cout << "testing objects" << endl;

	jvalue B;
	cout << B["A"] << endl;
	B["Z"] = 123;
	B["X"] = 1.23;
	B["Y"] = true;
	B["B"] = "BBB";

	cout << B << endl;

	B["A"] = A;

	cout << B << endl;

	cout << endl;
	cout << "testing parsing" << endl;

	stringstream SS;
	SS << "\"A\"";
	jvalue PP;
	PP.parse( SS );
	cout << "test string: " << PP << endl;

	cout << "array parsing" << endl;
	SS << A;
	PP.parse( SS );
	cout << "test array: " << PP << endl;

	cout << "object parsing" << endl;

	SS << "{}";
	SS >> PP;
	cout << "test empty object: " << PP << endl;

	SS << "{\"A\":\"B\"}";
	SS >> PP;
	cout << "test small object: " << PP << endl;

	SS << B;
	SS >> PP;
	cout << "test big object: " << PP << endl;

	SS << "{ \"A\":{}, \"B\":{ \"C\":{\"D\":{}, \"E\":{}}, \"F\":{} }, \"G\":{} }";
	SS >> PP;
	cout << "test multi-level object: " << PP << endl;

	SS << "[ \"A\", \"B\", { \"C\":{\"D\":[ 1, 2, 3], \"E\":22}, \"F\":{} }, \"G\" ]";
	SS >> PP;
	cout << "test multi-level array+object: " << PP << endl;

	cout << "test special characters in string\n";
	V = "\"ABC\"\\/\b\f\n\r\t\u0004";
	cout << V << endl;
	SS << V;
	SS >> PP;
	cout << PP << endl;

#else

	jvalue PP;

	for ( ;; )
	{
		cin >> PP;
		if ( PP.isNull() ) break;
		cout << PP << endl;
	}
#endif

	return 0;
}

