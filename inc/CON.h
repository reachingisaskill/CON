
#ifndef CON_INCLUDE_FILE_H_
#define CON_INCLUDE_FILE_H_

#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <list>


namespace CON
{

////////////////////////////////////////////////////////////////////////////////
  // Forward Declarations
  // Primary object class
  class Object;
  class Exception;
  struct ParseError;


////////////////////////////////////////////////////////////////////////////////
  // Data types for the values
  enum class Type { Null, String, Numeric, Boolean, Array, Object };

  // List of errors
  typedef std::vector<std::string> ErrorList;


////////////////////////////////////////////////////////////////////////////////
  // Creation functions
  // Specify filename
  Object buildFromFile( std::string );

  // Specify complete string
  Object buildFromString( std::string& );

  // Specify input stream
  Object buildFromStream( std::istream& );

  // Writing functions
  // Output to stream
  void writeToStream( Object&, std::ostream& );


////////////////////////////////////////////////////////////////////////////////
  // Custom exception class
  class Exception : public std::exception
  {
    public:
      typedef ErrorList::const_iterator iterator;

    private:
      // List the specific errors
      ErrorList _errors;

      // If needed, have the filename
      std::string _filename;

      // Cache the what string
      std::string _what;

      // Parser error flag
      bool _parseError;

      // Logic to update the type and clear out the unecessary data
      void _updateType( Type );

    public:
      // Parse error built from filname, line num, error
      Exception( ErrorList& );

      // Other crap went wrong
      explicit Exception( std::string );


      // Return a short summary
      virtual const char* what() const noexcept override;

      // Number of errors
      size_t number() const { return _errors.size(); }

      // Basic iteration
      iterator begin() const { return _errors.begin(); }
      iterator end() const { return _errors.end(); }

      // Set a file name if its relevant
      void setFilename( std::string );
  };


////////////////////////////////////////////////////////////////////////////////
  // Basic hierarchical object
  class Object
  {
    // Easier for writing to be a friend
    friend void printObject( Object&, std::ostream&, size_t );

    // Mapping of identifier to object pointer
    typedef std::map<std::string, Object*> ObjectMap;
    typedef std::vector<Object*> Array;

    private:
      // Map of all the children
      ObjectMap _children;

      // If array store the array items here
      Array _array;

      // String holding the literal value
      std::string _value;

      // Type of value stored
      Type _type;

    public:
      // Initialise empty object
      Object();

      // Initialise empty object with a default type
      explicit Object( Type );

      // Copy constructions
      Object( const Object& );

      // Move constructions
      Object( Object&& );

      // Copy assign
      Object& operator=( const Object& );

      // Move assign
      Object& operator=( Object&& );

      // Clearup the memory. Delete all the children
      ~Object();


////////////////////////////////////////////////////////////////////////////////
      // Set the type for the object will destroy internal data if the types are not compatible
      void setType( Type );
      Type getType() const { return _type; }


////////////////////////////////////////////////////////////////////////////////
      // If Type == Numeric or String or Boolean

      // Returns whether the object type is null. There are no other actions for a null object
      bool isNull() const { return _type == Type::Null; }


////////////////////////////////////////////////////////////////////////////////
      // If Type == Numeric or String or Boolean

      // Set the value string
      void setRawValue( std::string, Type );
      // Set the value string
      void setValue( std::string );
//      void setValue( char );
      void setValue( int );
      void setValue( long );
      void setValue( float );
      void setValue( double );
      void setValue( bool );
//      void setValue( unsigned char );
//      void setValue( unsigned int );
//      void setValue( unsigned long );
//      void setValue( unsigned float );
//      void setValue( unsigned double );

      // Return different interpretations of the value
      // String
      const std::string& asString() const;
      // Character
      char asChar() const;
      // Integer
      int asInt() const;
      // Float
      float asFloat() const;
      // Double
      double asDouble() const;
      // Boolean
      bool asBool() const;


////////////////////////////////////////////////////////////////////////////////
      // If Type == Object

      // If it has children it must be an object
      bool isObject() const { return _children.size() > 0; }

      // Return true if a child node exists with that name
      bool has( std::string ) const;

      // Add a child to the map
      void addChild( std::string, Object );

      // Return a child
      Object& get( std::string );
      Object& operator[]( std::string id ) { return this->get( id ); }
      const Object& get( std::string ) const;
      const Object& operator[]( std::string id ) const { return this->get( id ); }


////////////////////////////////////////////////////////////////////////////////
      // If Type == Array

      // Push an object to array.
      void push( Object& );
      void push( std::string );
      void push( char );
      void push( int );
      void push( long );
      void push( float );
      void push( double );

      // Return a index value from the array
      Object& get( size_t );
      Object& operator[]( size_t id ) { return this->get( id ); }
      const Object& get( size_t ) const;
      const Object& operator[]( size_t id ) const { return this->get( id ); }
  };

}

#endif // CON_INCLUDE_FILE_H_

