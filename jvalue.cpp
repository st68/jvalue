
#include "jvalue.h"
#include "crbncpy.h"
#include <set>
using namespace std;

jerr *
jerr::error( const char *xMsg )
{
	static jerr TheOne;
	TheOne.mMsg = xMsg;
	return &TheOne;
}

private_jvalue_data&
private_jvalue_data::operator=( const private_jvalue_data& xData )
{
	deleteValue();
	switch( mType = xData.mType )
	{
		case JBOOL:
			mValue.mBool = xData.mValue.mBool;
			break;
		case JINTEGER:
			mValue.mInteger = xData.mValue.mInteger;
			break;
		case JDOUBLE:
			mValue.mDouble = xData.mValue.mDouble;
			break;
		case JSTRING:
			mValue.mString = scopy( xData.mValue.mString );
			break;
		case JOBJECT:
			mValue.mObject = new object_map_t( *xData.mValue.mObject );
			break;
		case JARRAY:
			mValue.mArray = new array_vector_t( *xData.mValue.mArray );
			break;
		case JNULL:
		case JBAD:
		default:
			break;
	}
	unlock();
	return *this;
}

void
private_jvalue_data::push_back( const char *xValue )
{
	toJARRAY();										// if not already an array, make it so
	mValue.mArray->push_back( jvalue( xValue ) );	// push a jvalue string onto the array
	unlock();
}

void
private_jvalue_data::push_back( long long xValue )
{
	toJARRAY();
	mValue.mArray->push_back( jvalue( xValue ) );
	unlock();
}

void
private_jvalue_data::push_back( double xValue )
{
	toJARRAY();
	mValue.mArray->push_back( jvalue( xValue ) );
	unlock();
}

void
private_jvalue_data::push_back( bool xValue )
{
	toJARRAY();
	mValue.mArray->push_back( jvalue( xValue ) );
	unlock();
}

void
private_jvalue_data::push_back( jvalue& xValue )
{
	toJARRAY();
	mValue.mArray->push_back( xValue );
	unlock();
}

jvalue&
private_jvalue_data::operator[]( size_t xPos )
{
	toJARRAY();								// convert to an array if not already one
	while ( xPos >= mValue.mArray->size() )		// if array not large enough
		mValue.mArray->push_back( jvalue() );	//   push NULL's so access is always valid
	jvalue& Q = (*mValue.mArray)[xPos];				// return reference to appropriate location
	unlock();
	return Q;
}

jvalue&
private_jvalue_data::operator[]( const char *xName )
{
	if ( !xName )
		throw jerr::error( "missing object-element identifier" );
	lock(__LINE__);
	if ( mType != JOBJECT )		// convert to object if not already one
	{
		deleteValueNL();
		mType = JOBJECT;
		mValue.mObject = new object_map_t;
	}
	jvalue& Q = (*mValue.mObject)[xName];	// return reference to appropriate location (create if not there)
	unlock();
	return Q;
}

void
private_jvalue_data::deleteValueNL()	// private function to delete data in union if necessary
{
	switch( mType )		// special case for strings, objects, arrays
	{
		case JSTRING: delete[] mValue.mString; break;
		case JOBJECT: delete mValue.mObject; break;	// will potentially be recursive
		case JARRAY:  delete mValue.mArray;  break;	// will potentially be recursive
		case JBAD:    throw jerr::error( "deleting deleted jvalue value?" );
		default: break;	// most don't require extra work
	}
}

void
private_jvalue_data::toJARRAY_NL()	// convert to array if necessary
{
	if ( mType == JARRAY )
		return;
	deleteValueNL();
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
private_jvalue_data::size()
{
	lock(__LINE__);
	size_t S = sizeNL();
	unlock();
	return S;
}

size_t
private_jvalue_data::size() const
{
	return sizeNL();
}

size_t
private_jvalue_data::sizeNL() const
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
		case JBAD:     throw jerr::error( "accessing deleted jvalue (size)" );
	}
	return 0;
}

bool
private_jvalue_data::empty()
{
	lock(__LINE__);
	bool E = emptyNL();
	unlock();
	return E;
}

bool
private_jvalue_data::empty() const
{
	return emptyNL();
}

bool	// private
private_jvalue_data::emptyNL() const
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
		case JBAD:     throw jerr::error( "accessing deleted jvalue (empty)" );
	}
	return false;
}

char *	// static
private_jvalue_data::scopy( const char *xIn )
{
	if ( !xIn ) xIn = "";	// always return something
	size_t len = strlen( xIn );
	char *RV = new char[len + 1];
	strcpy( RV, xIn );
	return RV;
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
		case JBAD:     throw jerr::error( "accessing deleted jvalue (print)" );
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
	if ( Vector.empty() )
	{
		// nothing to print...
	}
	else if ( Vector.size() <= 4 )
	{
		os << Vector[0];
		for ( size_t index = 1; index < Vector.size(); index++ )
		{
			os << ",";
			os << Vector[index];
		}
	}
	else
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

static inline size_t
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

static inline size_t
readHex4( istream& is )
{
	return
		(readHex( is ) << 12) |
		(readHex( is ) << 8)  |
		(readHex( is ) << 4)  |
		 readHex( is );
}

static inline void
utfEncode( size_t xIn, string& xString ) // encode xIn into UTF-8, append to xString
{
	if ( xIn <= 0x7F )
	{
		// single character
		xString += xIn;
		return;
	}
	if ( xIn <= 0x7FF )
	{
		// two characters
		int C1 = ((xIn >> 6) & 0x1F) | 0xC0;
		int C2 = (xIn        & 0x3F) | 0x80;
		xString += C1;
		xString += C2;
		return;
	}
	if ( xIn <= 0xFFFF )
	{
		// three characters
		int C1 = ((xIn >> 12) & 0x0F) | 0xE0;
		int C2 = ((xIn >> 6)  & 0x3F) | 0x80;
		int C3 = ( xIn        & 0x3F) | 0x80;
		xString += C1;
		xString += C2;
		xString += C3;
		return;
	}
	if ( xIn <= 0x10FFFF )
	{
		// four characters
		int C1 = ((xIn >> 18) & 0x07) | 0xF0;
		int C2 = ((xIn >> 12) & 0x3F) | 0x80;
		int C3 = ((xIn >> 6)  & 0x3F) | 0x80;
		int C4 = ( xIn        & 0x3F) | 0x80;
		xString += C1;
		xString += C2;
		xString += C3;
		xString += C4;
		return;
	}
	// illegal character
	xString += "<BADC>";
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
					utfEncode( readHex4( is ), xString ); // encode into UTF-8
					break;
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
	unlock();
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
	if ( LastC == ']' )
	{
		is.get();	// flush the ]
	}
	else
	{
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
	}
	return true;
}

#if 0
#ifndef SINGLE_THREAD
static mutex ONE;
static std::set<const private_jvalue_data *> waiting;
static std::set<const private_jvalue_data *> locked;

void
private_jvalue_data::lock( int xLine )
{
	ONE.lock();
	waiting.insert( this );
	ONE.unlock();

	mLockData.lock();

	ONE.lock();
	locked.insert( this );
	waiting.erase( this );
	ONE.unlock();
	printf( "j: %lu/%lu\n", waiting.size(), locked.size() );
	fflush(stdout);
}

void
private_jvalue_data::unlock()
{
	mLockData.unlock();

	ONE.lock();
	locked.erase( this );
	ONE.unlock();
}

#endif
#endif

