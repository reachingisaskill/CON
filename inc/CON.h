
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
  enum class Type { Null, String, Integer, Float, Boolean, Object };

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
    // Mapping of identifier to object pointer
    typedef std::map<std::string, Object*> ObjectMap;

    private:
      // Map of all the children
      ObjectMap _children;

      // String holding the literal value
      std::string _value;

    public:
      // Initialise empty object
      Object();

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


      // If it has children it must be an object
      bool isObject() const { return _children.size() > 0; }


      // Set the value string
      void setValue( std::string );

      // Add a child to the map
      void addChild( std::string, Object );


      // Return different interpretations of the value
      // String
      const std::string& asString() const;
      // Integer
      int asInt() const;
      // Float
      float asFloat() const;
      // Double
      double asDouble() const;
      // Boolean
      bool asBool() const;


      // Return a child
      Object& get( std::string );
      Object& operator[]( std::string id ) { return this->get( id ); }
      const Object& get( std::string ) const;
      const Object& operator[]( std::string id ) const { return this->get( id ); }

      // Debug Print function
      void print( std::ostream&, size_t depth = 0 );
  };

}

#endif // CON_INCLUDE_FILE_H_

