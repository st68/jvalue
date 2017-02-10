
#ifndef jvalueHeader
#define jvalueHeader

/*
 * a generic json data structure and parser
 *
 * uses shared_ptr to point to all objects
 *   advantage: thread safe
 *   advantage: when accessing sub-components, does not copy whole structures
 *
 * ALL WORK IS DONE IN THE PRIVATE_JVALUE_DATA CLASS
 *     BUT PRIVATE_JVALUE_DATA's SHOULD NEVER BE DIRECTLY ACCESSED
 *
 * deleting a jvalue will delete all objects underneath (if not used elsewhere)
 *    caveat: it is possible to create a circular, non-tree structure
 *            will not cause error during delete, but objects will not be deleted, causing memory-leak
 *
 * DOES NOT CURRENTLY IMPLEMENT THESE ASPECTS OF THE JSON GRAMMAR:
 *   missing: unsigned long long
 *   missing: long long long
 *
 * use:
 *   jvalue A( "A" );		// creates a json string value named A
 *   jvalue B = A;			// new pointer to the same object A
 *   cout << B << endl;		// produces "A"
 *   A = "B";				// changes the value of A to "B"
 *   cout << B << endl;		// produces "B"
 *   A.push_back( 2 );		// turns A into an array with one element (the integer 2)
 *   A.push_back( true );	// adds another element
 *   cout << B << endl;		// produces [1, true]
 *   cout << B[0] << endl;	// produces 2
 *   A[1]["abc"] = 2;		// change the value of A[1] to an object
 *   A[1]["xyz"] = 3;		// update the object in A[1] with another entry
 *   cout << B << endl;		// produces [2, {"abc":2 "xyz":3}]
 *
 * comments:
 *   has seperate holders for integer and doubles
 *   all integer types mapped onto long long (so no unsigned long long)
 *   could implement "copy" function to actually make a complete copy when desired
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include "crbncpy.h"

using namespace std;
class jvalue;

typedef map<string,jvalue> object_map_t;
typedef vector<jvalue> array_vector_t;

enum jType { JNULL, JBOOL, JSTRING, JINTEGER, JDOUBLE, JOBJECT, JARRAY };

class jerr
{
		jerr() : mMsg(NULL) {}
		jerr( const jerr& );                 // not implemented
		const jerr operator=( const jerr& ); // not implemented
		~jerr() {}
	public:
		static jerr *error( const char *xMsg );
		const char *message() { return mMsg; }
	private:
		const char *mMsg;
};

class private_jvalue_data
{

	public:

		private_jvalue_data() : mType(JNULL)                             {}
		private_jvalue_data( bool xValue ) : mType(JNULL)                { Bool( xValue ); }
		private_jvalue_data( const char *xValue ) : mType(JNULL)         { String( xValue ); }
		private_jvalue_data( const string& xValue ) : mType(JNULL)       { String( xValue.c_str() ); }
		private_jvalue_data( long long xValue ) : mType(JNULL)           { Integer( xValue ); }
		private_jvalue_data( unsigned int xValue ) : mType(JNULL)        { Integer( xValue ); }
		private_jvalue_data( unsigned long int xValue ) : mType(JNULL)   { Integer( xValue ); }
		private_jvalue_data( unsigned long long xValue ) : mType(JNULL)  { Integer( xValue ); }
		private_jvalue_data( int xValue ) : mType(JNULL)                 { Integer( xValue ); }
		private_jvalue_data( char xValue ) : mType(JNULL)                { Integer( xValue ); }
		private_jvalue_data( float xValue ) : mType(JNULL)               { Double( xValue ); }
		private_jvalue_data( double xValue ) : mType(JNULL)              { Double( xValue ); }
		private_jvalue_data( object_map_t *xValue ) : mType(JNULL)       { Object( xValue ); }
		private_jvalue_data( array_vector_t *xValue ) : mType(JNULL)     { Array( xValue ); }

		private_jvalue_data( const private_jvalue_data& xData );	// copy from data
		private_jvalue_data( const jvalue& xValue );	// copy from value

		~private_jvalue_data() { deleteValue(); }

		private_jvalue_data& operator=( const private_jvalue_data& xData )
			{ deleteValue(); mType = xData.mType; mValue = xData.mValue; return *this; }

		private_jvalue_data& operator=( const string& xValue )      {  String( xValue.c_str() ); return *this; }
		private_jvalue_data& operator=( const char *xValue )        {  String( xValue ); return *this; }
		private_jvalue_data& operator=( long long xValue )          { Integer( xValue ); return *this; }
		private_jvalue_data& operator=( double xValue )             {  Double( xValue ); return *this; }
		private_jvalue_data& operator=( bool xValue )               {    Bool( xValue ); return *this; }

		private_jvalue_data& operator=( unsigned int xValue )       { return operator=( (long long)xValue ); }
		private_jvalue_data& operator=( unsigned long int xValue )  { return operator=( (long long)xValue ); }
		private_jvalue_data& operator=( unsigned long long xValue ) { return operator=( (long long)xValue ); }
		private_jvalue_data& operator=( int xValue )                { return operator=( (long long)xValue ); }
		private_jvalue_data& operator=( char xValue )               { return operator=( (long long)xValue ); }
		private_jvalue_data& operator=( float xValue )              { return operator=( (double)xValue ); }

		jvalue& operator[]( size_t xPos );
		jvalue& operator[]( const char *xString );
		jvalue& operator[]( const string& xString ) { return operator[]( xString.c_str() ); }

		jvalue& operator[]( unsigned long long xValue ) { return operator[]( (size_t)xValue ); }
		jvalue& operator[]( int xValue )                { return operator[]( (size_t)xValue ); }
		jvalue& operator[]( char xValue )               { return operator[]( (size_t)xValue ); }

		// add elements to an Array
		void push_back( const string& xValue ) { push_back( xValue.c_str() ); }
		void push_back( const char *xValue );
		void push_back( long long xValue );
		void push_back( double xValue );
		void push_back( bool xValue );
		void push_back( jvalue& xValue );

		void push_back( unsigned int xValue )       { push_back( (long long)xValue ); }
		void push_back( unsigned long int xValue )  { push_back( (long long)xValue ); }
		void push_back( int xValue )                { push_back( (long long)xValue ); }
		void push_back( char xValue )               { push_back( (long long)xValue ); }
		void push_back( unsigned char xValue )      { push_back( (long long)xValue ); }
		void push_back( unsigned long long xValue ) { push_back( (long long)xValue ); }
		void push_back( float xValue )              { push_back( (double)xValue );    }

		// comparisons

		bool operator==( const string& xValue )      const { return strcmp( String(), xValue.c_str() ) == 0; }
		bool operator==( const char *xValue )        const { return xValue ? strcmp( String(), xValue ) == 0 : false; }
		bool operator==( long long xValue )          const { return Integer() == xValue; }
		bool operator==( double xValue )             const { return Double() == xValue; }
		bool operator==( bool xValue )               const { return Bool() == xValue; }

		bool operator==( unsigned int xValue )       const { return operator==( (long long)xValue ); }
		bool operator==( unsigned long int xValue )  const { return operator==( (long long)xValue ); }
		bool operator==( unsigned long long xValue ) const { return operator==( (long long)xValue ); }
		bool operator==( int xValue )                const { return operator==( (long long)xValue ); }
		bool operator==( char xValue )               const { return operator==( (long long)xValue ); }
		bool operator==( float xValue )              const { return operator==( (double)xValue ); }

		bool operator<=( const string& xValue )      const { return strcmp( String(), xValue.c_str() ) <= 0; }
		bool operator<=( const char *xValue )        const { return xValue ? strcmp( String(), xValue ) <= 0 : false; }
		bool operator<=( long long xValue )          const { return Integer() <= xValue; }
		bool operator<=( double xValue )             const { return Double() <= xValue; }
		bool operator<=( bool xValue )               const { return true; }

		bool operator<=( unsigned int xValue )       const { return operator<=( (long long)xValue ); }
		bool operator<=( unsigned long int xValue )  const { return operator<=( (long long)xValue ); }
		bool operator<=( unsigned long long xValue ) const { return operator<=( (long long)xValue ); }
		bool operator<=( int xValue )                const { return operator<=( (long long)xValue ); }
		bool operator<=( char xValue )               const { return operator<=( (long long)xValue ); }
		bool operator<=( float xValue )              const { return operator<=( (double)xValue ); }

		bool operator<( const string& xValue )      const { return strcmp( String(), xValue.c_str() ) < 0; }
		bool operator<( const char *xValue )        const { return xValue ? strcmp( String(), xValue ) < 0 : false; }
		bool operator<( long long xValue )          const { return Integer() < xValue; }
		bool operator<( double xValue )             const { return Double() < xValue; }
		bool operator<( bool xValue )               const { return (int)Bool() < (int)xValue; }

		bool operator<( unsigned int xValue )       const { return operator<( (long long)xValue ); }
		bool operator<( unsigned long int xValue )  const { return operator<( (long long)xValue ); }
		bool operator<( unsigned long long xValue ) const { return operator<( (long long)xValue ); }
		bool operator<( int xValue )                const { return operator<( (long long)xValue ); }
		bool operator<( char xValue )               const { return operator<( (long long)xValue ); }
		bool operator<( float xValue )              const { return operator<( (double)xValue ); }

		// misc functions

		bool            Bool()    const { return mType == JBOOL && mValue.mBool;            }
		const char     *String()  const { return mType == JSTRING && mValue.mString ? mValue.mString : "";   }
		long long       Integer() const { return mType == JINTEGER ? mValue.mInteger : (mType == JDOUBLE ? (long long)mValue.mDouble : (mType == JSTRING && mValue.mString ? atoll( mValue.mString ) : 0LL)); }
		double          Double()  const { return mType == JDOUBLE  ? mValue.mDouble : (mType == JINTEGER ? (double)mValue.mInteger : (mType == JSTRING && mValue.mString ? atof( mValue.mString ) : 0.0)); }
		object_map_t   *Object()  const { return mType == JOBJECT  ? mValue.mObject : NULL; }
		array_vector_t *Array()   const { return mType == JARRAY   ? mValue.mArray : NULL;  }

		void Null()                          { deleteValue(); mType = JNULL;                                        }
		void Bool( bool xValue )             { deleteValue(); mType = JBOOL;    mValue.mBool = xValue;              }
		void String( const char *xValue )    { deleteValue(); mType = JSTRING;  mValue.mString = crbncpy( xValue ); }
		void Integer( long long xValue )     { deleteValue(); mType = JINTEGER; mValue.mInteger = xValue;           }
		void Double( double xValue )         { deleteValue(); mType = JDOUBLE;  mValue.mDouble = xValue;            }
		void Object( object_map_t *xValue )  { deleteValue(); mType = JOBJECT;  mValue.mObject = xValue;            }
		void Array( array_vector_t *xValue ) { deleteValue(); mType = JARRAY;   mValue.mArray = xValue;             }

		jType type() const { return mType; }
		size_t size() const;
		bool empty() const;

		bool isNull() const    { return mType == JNULL;    }
		bool isBool() const    { return mType == JBOOL;    }
		bool isString() const  { return mType == JSTRING;  }
		bool isInteger() const { return mType == JINTEGER; }
		bool isDouble() const  { return mType == JDOUBLE;  }
		bool isObject() const  { return mType == JOBJECT;  }
		bool isArray() const   { return mType == JARRAY;   }

		// THESE ONLY WORK IF JVALUE IS ALREADY AN OBJECT
		object_map_t::const_iterator begin() const { return mValue.mObject->begin(); }
		object_map_t::const_iterator end() const { return mValue.mObject->end(); }

		void print( ostream&, unsigned int xLevel = 0 ) const;
		bool parse( istream& is );

	protected:

		union
		{
			bool            mBool;
			char           *mString;
			long long       mInteger;
			double          mDouble;
			object_map_t   *mObject;
			array_vector_t *mArray;
		} mValue;

		jType mType;

	private:

		void deleteValue();
		void toJARRAY();

		void printObject( ostream&, unsigned int ) const;
		void printArray( ostream&, unsigned int ) const;

		bool parseString( istream& is );
		bool parseNumber( istream& is );
		bool parseNull(   istream& is );
		bool parseTrue(   istream& is );
		bool parseFalse(  istream& is );
		bool parsePair(   istream& is );	// for objects
		bool parseObject( istream& is );
		bool parseArray(  istream& is );

};

inline ostream& operator<<( ostream& os, const private_jvalue_data& xJD )
    { xJD.print( os ); return os; }

class jvalue : public shared_ptr<private_jvalue_data>		// a jvalue is only an overloaded shared_ptr; it contains no other data
{

	public:

		jvalue()                            : shared_ptr<private_jvalue_data>( new private_jvalue_data() )         {} // a Null value
		jvalue( const char *xValue )        : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( const string& xValue )      : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( long long xValue )          : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( unsigned int xValue )       : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( unsigned long int xValue )  : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( unsigned long long xValue ) : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( int xValue )                : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( char xValue )               : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( float xValue )              : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( double xValue )             : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}
		jvalue( bool xValue )               : shared_ptr<private_jvalue_data>( new private_jvalue_data( xValue ) ) {}

		jvalue( const jvalue& xValue ) = default;

		~jvalue() {}	// deletes shared_ptr, which may delete the associated private_jvalue_data

		jvalue& operator=( const jvalue& xValue ) = default;

		jvalue& operator=( const std::string& xValue ) { shared_ptr<private_jvalue_data>::get()->operator=( xValue ); return *this; }
		jvalue& operator=( const char *xValue )        { shared_ptr<private_jvalue_data>::get()->operator=( xValue ); return *this; }
		jvalue& operator=( long long xValue )          { shared_ptr<private_jvalue_data>::get()->operator=( xValue ); return *this; }
		jvalue& operator=( double xValue )             { shared_ptr<private_jvalue_data>::get()->operator=( xValue ); return *this; }
		jvalue& operator=( bool xValue )               { shared_ptr<private_jvalue_data>::get()->operator=( xValue ); return *this; }

		jvalue& operator=( unsigned int xValue )       { return operator=( (long long)xValue ); }
		jvalue& operator=( unsigned long int xValue )  { return operator=( (long long)xValue ); }
		jvalue& operator=( unsigned long long xValue ) { return operator=( (long long)xValue ); }
		jvalue& operator=( int xValue )                { return operator=( (long long)xValue ); }
		jvalue& operator=( char xValue )               { return operator=( (long long)xValue ); }
		jvalue& operator=( float xValue )              { return operator=( (double)xValue ); }

		jvalue& operator[]( int xPos )                   { return shared_ptr<private_jvalue_data>::get()->operator[]( (size_t)xPos ); }	// fixes problem with A[0]
		jvalue& operator[]( size_t xPos )                { return shared_ptr<private_jvalue_data>::get()->operator[]( xPos ); }
		jvalue& operator[]( long long xPos )             { return shared_ptr<private_jvalue_data>::get()->operator[]( (size_t)xPos ); }	// fixes problem with A[0]
		jvalue& operator[]( const char *xString )        { return shared_ptr<private_jvalue_data>::get()->operator[]( xString ); }
		jvalue& operator[]( const std::string& xString ) { return shared_ptr<private_jvalue_data>::get()->operator[]( xString ); }

		// add elements to an Array
		void push_back( const std::string& xValue ) { push_back( xValue.c_str() ); }
		void push_back( const char *xValue )        { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( long long xValue )          { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( double xValue )             { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( bool xValue )               { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }

		void push_back( unsigned int xValue )       { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( unsigned long int xValue )  { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( int xValue )                { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( char xValue )               { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( unsigned char xValue )      { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( unsigned long long xValue ) { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }
		void push_back( float xValue )              { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }

		void push_back( jvalue xValue )             { shared_ptr<private_jvalue_data>::get()->push_back( xValue ); }

		// compare various things

		bool operator==( const std::string& xValue ) const { return shared_ptr<private_jvalue_data>::get()->operator==( xValue ); }
		bool operator==( const char *xValue )        const { return shared_ptr<private_jvalue_data>::get()->operator==( xValue ); }
		bool operator==( long long xValue )          const { return shared_ptr<private_jvalue_data>::get()->operator==( xValue ); }
		bool operator==( double xValue )             const { return shared_ptr<private_jvalue_data>::get()->operator==( xValue ); }
		bool operator==( bool xValue )               const { return shared_ptr<private_jvalue_data>::get()->operator==( xValue ); }

		bool operator==( unsigned int xValue )       const { return operator==( (long long)xValue ); }
		bool operator==( unsigned long int xValue )  const { return operator==( (long long)xValue ); }
		bool operator==( unsigned long long xValue ) const { return operator==( (long long)xValue ); }
		bool operator==( int xValue )                const { return operator==( (long long)xValue ); }
		bool operator==( char xValue )               const { return operator==( (long long)xValue ); }
		bool operator==( float xValue )              const { return operator==( (double)xValue ); }

		bool operator<=( const std::string& xValue ) const { return shared_ptr<private_jvalue_data>::get()->operator<=( xValue ); }
		bool operator<=( const char *xValue )        const { return shared_ptr<private_jvalue_data>::get()->operator<=( xValue ); }
		bool operator<=( long long xValue )          const { return shared_ptr<private_jvalue_data>::get()->operator<=( xValue ); }
		bool operator<=( double xValue )             const { return shared_ptr<private_jvalue_data>::get()->operator<=( xValue ); }
		bool operator<=( bool xValue )               const { return shared_ptr<private_jvalue_data>::get()->operator<=( xValue ); }

		bool operator<=( unsigned int xValue )       const { return operator<=( (long long)xValue ); }
		bool operator<=( unsigned long int xValue )  const { return operator<=( (long long)xValue ); }
		bool operator<=( unsigned long long xValue ) const { return operator<=( (long long)xValue ); }
		bool operator<=( int xValue )                const { return operator<=( (long long)xValue ); }
		bool operator<=( char xValue )               const { return operator<=( (long long)xValue ); }
		bool operator<=( float xValue )              const { return operator<=( (double)xValue ); }

		bool operator<( const std::string& xValue )  const { return shared_ptr<private_jvalue_data>::get()->operator<( xValue ); }
		bool operator<( const char *xValue )         const { return shared_ptr<private_jvalue_data>::get()->operator<( xValue ); }
		bool operator<( long long xValue )           const { return shared_ptr<private_jvalue_data>::get()->operator<( xValue ); }
		bool operator<( double xValue )              const { return shared_ptr<private_jvalue_data>::get()->operator<( xValue ); }
		bool operator<( bool xValue )                const { return shared_ptr<private_jvalue_data>::get()->operator<( xValue ); }

		bool operator<( unsigned int xValue )        const { return operator<( (long long)xValue ); }
		bool operator<( unsigned long int xValue )   const { return operator<( (long long)xValue ); }
		bool operator<( unsigned long long xValue )  const { return operator<( (long long)xValue ); }
		bool operator<( int xValue )                 const { return operator<( (long long)xValue ); }
		bool operator<( char xValue )                const { return operator<( (long long)xValue ); }
		bool operator<( float xValue )               const { return operator<=( (double)xValue ); }

		// misc functions

		jType type() const { return shared_ptr<private_jvalue_data>::get()->type(); }
		size_t size() const { return shared_ptr<private_jvalue_data>::get()->size(); }
		bool empty() const { return shared_ptr<private_jvalue_data>::get()->empty(); }

		bool isNull() const    { return type() == JNULL;    }
		bool isBool() const    { return type() == JBOOL;    }
		bool isString() const  { return type() == JSTRING;  }
		bool isInteger() const { return type() == JINTEGER; }
		bool isDouble() const  { return type() == JDOUBLE;  }
		bool isObject() const  { return type() == JOBJECT;  }
		bool isArray() const   { return type() == JARRAY;   }

		bool            Bool()    { return shared_ptr<private_jvalue_data>::get()->Bool()    ; }
		const char     *String()  { return shared_ptr<private_jvalue_data>::get()->String()  ; }
		long long       Integer() { return shared_ptr<private_jvalue_data>::get()->Integer() ; }
		double          Double()  { return shared_ptr<private_jvalue_data>::get()->Double()  ; }
		object_map_t   *Object()  { return shared_ptr<private_jvalue_data>::get()->Object()  ; }
		array_vector_t *Array()   { return shared_ptr<private_jvalue_data>::get()->Array()   ; }

		void Null()                          { shared_ptr<private_jvalue_data>::get()->Null();            }
		void Bool( bool xValue )             { shared_ptr<private_jvalue_data>::get()->Bool(    xValue ); }
		void String( const char *xValue )    { shared_ptr<private_jvalue_data>::get()->String(  xValue ); }
		void Integer( long long xValue )     { shared_ptr<private_jvalue_data>::get()->Integer( xValue ); }
		void Double( double xValue )         { shared_ptr<private_jvalue_data>::get()->Double(  xValue ); }
		void Object( object_map_t *xValue )  { shared_ptr<private_jvalue_data>::get()->Object(  xValue ); }
		void Array( array_vector_t *xValue ) { shared_ptr<private_jvalue_data>::get()->Array(   xValue ); }

		// THESE ONLY WORK IF JVALUE IS ALREADY AN OBJECT
		object_map_t::const_iterator begin() const { return shared_ptr<private_jvalue_data>::get()->begin(); }
		object_map_t::const_iterator end() const { return shared_ptr<private_jvalue_data>::get()->end(); }

		void print( std::ostream& os ) const { shared_ptr<private_jvalue_data>::get()->print( os ); }
		bool parse( std::istream& is ) { return shared_ptr<private_jvalue_data>::get()->parse( is ); }

};

inline std::ostream& operator<<( std::ostream& os, const jvalue& xJV )
	{ xJV.print( os ); return os; }

inline std::istream& operator>>( std::istream& is, jvalue& xJV )
	{ xJV.parse( is ); return is; }

#endif

