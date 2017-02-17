
#include "jvalue.h"

jerr *
jerr::error( const char *xMsg )
{
	static jerr TheOne;
	TheOne.mMsg = xMsg;
	return &TheOne;
}

void
private_jvalue_data::push_back( const char *xValue )
{
	toJARRAY();										// if not already an array, make it so
	mValue.mArray->push_back( jvalue( xValue ) );	// push a jvalue string onto the array
}

void
private_jvalue_data::push_back( long long xValue )
{
	toJARRAY();
	mValue.mArray->push_back( jvalue( xValue ) );
}

void
private_jvalue_data::push_back( double xValue )
{
	toJARRAY();
	mValue.mArray->push_back( jvalue( xValue ) );
}

void
private_jvalue_data::push_back( bool xValue )
{
	toJARRAY();
	mValue.mArray->push_back( jvalue( xValue ) );
}

void
private_jvalue_data::push_back( jvalue& xValue )
{
	toJARRAY();
	mValue.mArray->push_back( xValue );
}

jvalue&
private_jvalue_data::operator[]( size_t xPos )
{
	toJARRAY();									// convert to an array if not already one
	while ( xPos >= mValue.mArray->size() )		// if array not large enough
		mValue.mArray->push_back( jvalue() );	//   push NULL's so access is always valid
	return (*mValue.mArray)[xPos];				// return reference to appropriate location
}

jvalue&
private_jvalue_data::operator[]( const char *xName )
{
	if ( mType != JOBJECT )						// convert to object if not already one
	{
		deleteValue();
		mType = JOBJECT;
		mValue.mObject = new object_map_t;
	}
	return (*mValue.mObject)[xName];			// return reference to appropriate location (create if not there)
}

void
private_jvalue_data::deleteValue()	// private function to delete data in union if necessary
{
	switch( mType )		// special case for strings, objects, arrays
	{
		case JSTRING: delete mValue.mString; break;
		case JOBJECT: delete mValue.mObject; break;	// will potentially be recursive
		case JARRAY:  delete mValue.mArray;  break;	// will potentially be recursive
		default: break;	// most don't require extra work
	}
}

void
private_jvalue_data::toJARRAY()	// convert to array if necessary
{
	if ( mType == JARRAY )
		return;
	deleteValue();
	mType = JARRAY;
	mValue.mArray = new array_vector_t;
}

/*
 * routines to print out the json structures
 *  DOES NOT ATTEMPT TO PRETTY-PRINT
 * all goes on one line
 *
 */

static void
cr( std::ostream& os, unsigned int xLevel )
{
	os << endl;
	while ( xLevel-- > 0 )
		os << " ";
}

static inline unsigned char *	// xBuffer points to next character
printChar( std::ostream& os, unsigned char *xBuffer )
{
	char buffer[16];
	unsigned char C = *xBuffer++;
	if ( C == 0 )
		return NULL;
	if ( C == '"' )
		os << "\\\"";
	else if ( C == '\\' )
		os << "\\\\";
	else if ( C == '/' )
		os << "\\/";
	else if ( C == '\b' )
		os << "\\b";
	else if ( C == '\f' )
		os << "\\f";
	else if ( C == '\n' )
		os << "\\n";
	else if ( C == '\r' )
		os << "\\r";
	else if ( C == '\t' )
		os << "\\t";
	else if ( C < 32 || C == 127 )
	{
		sprintf( buffer, "\\u%04x", C );
		os << buffer;
	}
	else if ( C & 0x80 )	// see if UTF-8
	{
		unsigned int Q = 0;
		if ( 0xC0 <= C && C <= 0xDF )		// two bytes
		{
			Q  = ((unsigned int)(C & 0x1F))          << 6;
			Q |= ((unsigned int)(*xBuffer++ & 0x3F));
		}
		else if ( 0xE0 <= C && C <= 0xEF )	// three bytes
		{
			Q  = ((unsigned int)(C & 0xF))           << 12;
			Q |= ((unsigned int)(*xBuffer++ & 0x3F)) << 6;
			Q |= ((unsigned int)(*xBuffer++ & 0x3F));
		}
		else if ( 0xF0 <= C && C <= 0xF7 )	// four bytes
		{
			Q  = ((unsigned int)(C & 0x7))           << 18;
			Q |= ((unsigned int)(*xBuffer++ & 0x3F)) << 12;
			Q |= ((unsigned int)(*xBuffer++ & 0x3F)) << 6;
			Q |= ((unsigned int)(*xBuffer++ & 0x3F));
		}
		else
			Q = C;
		sprintf( buffer, "\\u%04x", Q );
		os << buffer;
	}
	else
		os << C;
	return xBuffer;
}

static inline void
printString( std::ostream& os, const char *xString )
{
	os << '"';
	if ( unsigned char *String = (unsigned char *)xString )
		while ( String = printChar( os, String ) );
	os << '"';
}

size_t
private_jvalue_data::size() const
{
	switch( mType )
	{
		case JNULL:    return 0;
		case JBOOL:    return 1;
		case JSTRING:  return mValue.mString ? strlen( mValue.mString ) : 0;
		case JINTEGER: return 1;
		case JDOUBLE:  return 1;
		case JOBJECT:  return mValue.mObject ? mValue.mObject->size() : 0;
		case JARRAY:   return mValue.mArray ? mValue.mArray->size() : 0;
	}
	return 0;
}

bool
private_jvalue_data::empty() const
{
	switch( mType )
	{
		case JNULL:    return true;
		case JBOOL:    return false;
		case JSTRING:  return mValue.mString == NULL || mValue.mString[0] == 0;
		case JINTEGER: return false;
		case JDOUBLE:  return false;
		case JOBJECT:  return mValue.mObject ? mValue.mObject->empty() : true;
		case JARRAY:   return mValue.mArray ? mValue.mArray->empty() : true;
	}
	return false;
}

void
private_jvalue_data::print( std::ostream& os, unsigned int xLevel ) const
{
	switch( mType )
	{
		case JNULL:    os << "null";                            break;
		case JBOOL:    os << (mValue.mBool ? "true" : "false"); break;
		case JSTRING:  printString( os, mValue.mString );       break;
		case JINTEGER: os << mValue.mInteger;                   break;
		case JDOUBLE:  os << mValue.mDouble;                    break;
		case JOBJECT:  printObject( os, xLevel + 1 );           break;
		case JARRAY:   printArray( os, xLevel + 1 );            break;
	}
}

void
private_jvalue_data::printObject( std::ostream& os, unsigned int xLevel ) const
{
	os << "{";
	object_map_t::const_iterator IT = mValue.mObject->begin();
	object_map_t::const_iterator EN = mValue.mObject->end();
	object_map_t::const_iterator BG = IT;
	if ( mValue.mObject->size() <= 2 )
	{
		for ( ; IT != EN; IT++ )
		{
			if ( IT != BG )
				os << ", ";
			printString( os, IT->first.c_str() );
			os << ":";
			IT->second->print( os, xLevel + 1 );
		}
	}
	else
	{
		for ( ; IT != EN; IT++ )
		{
			if ( IT != BG )
				os << ",";
			cr( os, xLevel );
			printString( os, IT->first.c_str() );
			os << ":";
			IT->second->print( os, xLevel + 1 );
		}
		cr( os, xLevel );
	}
	os << "}";
}

void
private_jvalue_data::printArray( std::ostream& os, unsigned int xLevel ) const
{
	array_vector_t& Vector = *mValue.mArray;
	os << "[";
	if ( Vector.size() <= 4 )
	{
		if ( !Vector.empty() )
			os << Vector[0];
		for ( size_t index = 1; index < Vector.size(); index++ )
		{
			os << ",";
			if ( Vector.size() > 4 ) os << endl;
			os << Vector[index];
		}
	}
	else if ( !Vector.empty() )
	{
		cr( os, xLevel );
		os << Vector[0];
		for ( size_t index = 1; index < Vector.size(); index++ )
		{
			os << ",";
			cr( os, xLevel );
			os << Vector[index];
		}
		cr( os, xLevel );
	}
	os << "]";
}

/*
 * the parser functions
 *
 */

static inline int
flushSpace( istream& is )
{
	int C = is.peek();
	while ( isspace( C ) )
	{
		is.get();		// flush whitespace
		C = is.peek();	// look at next character
	}
	return C;
}

bool	// returns false if there is a parsing error
private_jvalue_data::parse( istream& is )
{
	Null();	// clean out anything already here...
	int C = flushSpace( is );
	if ( C < 0 ) return false;	// EOF
	// dispatch to correct parse function based on leading character of the object
	if ( isdigit( C ) || C == '.' || C == '-' ) return parseNumber( is );
	if ( C == '"' ) return parseString( is );
	if ( C == '{' ) return parseObject( is );
	if ( C == '[' ) return parseArray( is );
	if ( C == 'N' || C == 'n' ) return parseNull( is );
	if ( C == 'T' || C == 't' ) return parseTrue( is );
	if ( C == 'F' || C == 'f' ) return parseFalse( is );
	// none of the above -- bad stream
	char buffer[128];
	sprintf( buffer, "could not determine json type from leading character: %c<%02x>", C, C );
	throw jerr::error( crbncpy( buffer ) );
	return false;
}

static inline int
readHex( istream& is )
{
	int C = is.get();
	if ( isdigit( C ) )
		return C - '0';
	if ( 'A' <= C && C <= 'F' )
		return C - 'A' + 10;
	if ( 'a' <= C && C <= 'f' )
		return C - 'a' + 10;
	throw jerr::error( "bad hex character" );
	return 0;
}

static inline int
readHex4( istream& is )
{
	return
		(readHex( is ) << 12) |
		(readHex( is ) << 8)  |
		(readHex( is ) << 4)  |
		 readHex( is );
}

static bool
rawParseString( istream& is, string& xString )	// helper function for parseString, parsePair
{
	xString.clear();
	int FirstC = flushSpace( is );
	if ( FirstC != '"' )
		return false;
	is.get();	// flush the double quotes
	for ( ;; )
	{
		int C = is.get();
		if ( C == EOF )
			throw jerr::error( "private_jvalue_data::parseString : found EOF inside string" );
		if ( C == '"' )
			break;
		if ( C == '\\' )
		{
			switch( C = is.get() )
			{
				case 'b':
					xString += '\b';
					break;
				case 'f':
					xString += '\f';
					break;
				case 'n':
					xString += '\n';
					break;
				case 'r':
					xString += '\r';
					break;
				case 't':
					xString += '\t';
					break;
				case EOF:
					return false;
				case 'u':
					C = readHex4( is );
				default:
					xString += C;
					break;
			}
		}
		else
			xString += C;
	}
	return true;
}

bool
private_jvalue_data::parseString( istream& is )
{
	string Answer;
	if ( !rawParseString( is, Answer ) )
		return false;
	String( Answer.c_str() );
	return true;
}

bool
private_jvalue_data::parseNumber( istream& is )
{
	bool period = false;
	bool exponent = false;
	string Answer;

	if ( is.peek() == '-' )
	{
		Answer += '-';
		is.get();	// swallow the character
	}

	while ( !is.eof() )
	{
		int C = is.get();
		if ( isdigit( C ) )
			Answer += C;
		else if ( C == '.' && !period )
		{
			Answer += C;
			period = true;
		}
		else if ( (C == 'e' || C == 'E') && !exponent )
		{
			Answer += 'e';
			C = is.get();
			if ( isdigit( C ) )
			{
				Answer += C;
			}
			else if ( C == '-' || C == '+' )
			{
				Answer += C;
				C = is.get();
				if ( !isdigit( C ) )
					throw jerr::error( "private_jvalue_data::parseNumber : missing digits in exponent" );
				Answer += C;
			}
			else
				throw jerr::error( "private_jvalue_data::parseNumber : bad exponential format" );
			exponent = true;
		}
		else
		{
			is.putback( C );
			break;
		}
	}

	if ( period | exponent )
		Double( atof( Answer.c_str() ) );
	else
		Integer( atoll( Answer.c_str() ) );

	return true;
}

bool
private_jvalue_data::parseNull( istream& is )
{
	char buffer[4];
	is.read( buffer, sizeof(buffer) );

	if ( strncasecmp( "null", buffer, 4 ) != 0 )
		throw jerr::error( "private_jvalue_data::parseNull : string is not 'null'" );

	Null();

	return true;
}

bool
private_jvalue_data::parseTrue( istream& is )
{
	char buffer[4];
	is.read( buffer, sizeof(buffer) );

	if ( strncasecmp( "true", buffer, sizeof(buffer) ) != 0 )
		throw jerr::error( "private_jvalue_data::parseTrue : string is not 'true'" );

	Bool( true );

	return true;
}

bool
private_jvalue_data::parseFalse( istream& is )
{
	char buffer[5];
	is.read( buffer, sizeof(buffer) );

	if ( strncasecmp( "false", buffer, sizeof(buffer) ) != 0 )
		throw jerr::error( "private_jvalue_data::parseNull : string is not 'false'" );

	Bool( false );

	return true;
}

bool
private_jvalue_data::parsePair( istream& is )	// helper function for parseObject
{
	int FirstC = flushSpace( is );
	if ( FirstC == '}' )
		return false;
	string Name;
	if ( !rawParseString( is, Name ) )
		return false;
	flushSpace( is );
	int C = is.get();
	if ( C != ':' )
		return false;
	jvalue Value;
	if ( !Value.parse( is ) )
		return false;
	(*this)[Name] = Value;
	return true;
}

bool
private_jvalue_data::parseObject( istream& is )
{
	int FirstC = is.get();
	if ( FirstC != '{' )
		throw jerr::error( "private_jvalue_data::parseObject : first character is not '{'" );
	deleteValue();
	mType = JOBJECT;
	mValue.mObject = new object_map_t;
	flushSpace( is );
	int SecondC = is.peek();	// nothing in the object?
	if ( SecondC == '}' )
	{
		is.get();	// swallow the character
		return true;
	}
	for ( ;; )
	{
		if ( !parsePair( is ) )
			throw jerr::error( "private_jvalue_data::parseObject : bad pair in object" );
		flushSpace( is );
		int LastC = is.get();
		if ( LastC == '}' )
			break;
		if ( LastC != ',' )
			throw jerr::error( "private_jvalue_data::parseObject : missing comma" );
	}
	return true;
}

bool
private_jvalue_data::parseArray( istream& is )
{
	int FirstC = is.get();
	if ( FirstC != '[' )
		throw jerr::error( "private_jvalue_data::parseArray : first character is not '['" );
	int LastC = flushSpace( is );
	if ( LastC != ']' )
		for ( ;; )
		{
			jvalue Value;
			if ( !Value->parse( is ) )
				throw jerr::error( "private_jvalue_data::parseArray : issue parsing value in array" );
			push_back( Value );
			flushSpace( is );
			LastC = is.get();
			if ( LastC == ']' )
				break;
			if ( LastC != ',' )
				throw jerr::error( "private_jvalue_data::parseArray : missing comma between values" );
			flushSpace( is );
		}
	return true;
}

