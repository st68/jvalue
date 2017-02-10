
yet another json reader/writer

uses shared pointer to automatically garbage collect

{
	jvalue X;	// creates a NULL json object
	cout << X;	// produces "null"

	X["a"] = "123";	// updates X to an object, creates a json string with "123" in it
	cout << X;	// {"a":"123"}
	printf( "%s\n", X["a"].String() );	// produces: 123

	X.push_back( "123" );	// updates X to an ARRAY, creates a new json string
	X.push_back( 123 );		// adds another item to the ARRAY
	cout << X;	// ["123",123]

	jvalue Y;
	Y["b"] = X;
	Y["c"] = X;
	cout << Y;	// {"b":["123",123],"c":["123",123]}

	jvalue Z;
	Z = Y;		// Z and Y point to same -- changing one affects both

	stringstream SS;
	SS << Y;
	Z << SS;	// Z now has a copy of Y
}

// all jvalue structures deleted, and all memory released

mostly thread-safe (derived from shared pointer).
exception: begin/end used to access Objects -- if Object modified...

