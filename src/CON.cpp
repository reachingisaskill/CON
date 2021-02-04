
#include "CON.h"

#ifndef CON_BUFFER_SIZE
#define CON_BUFFER_SIZE 5000
#endif

namespace CON
{

////////////////////////////////////////////////////////////////////////////////////////////////////
  // Exception function definiions

  Exception::Exception( std::string filename, size_t lineNumber, std::string error ) :
    _errors(),
    _what(),
    _parseError( true )
  {
    std::stringstream ss;
    ss << "Error in file: \"" << filename << "\", Line: " << lineNumber << " : " << error;

    _errors.push_back( ss.str() );
  }


  Exception::Exception( std::string error ) :
    _errors(),
    _what( error ),
    _parseError( false )
  {
  }


  const char* Exception::what() const
  {
    if ( _parseError )
    {
      std::stringstream ss;
      ss << "Found " << _errors.size() << " parse errors";
      _what = ss.str();
    }

    return _what.c_str();
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // CON Object member function definitions

  Object::Object( std::string filename ) :
    _children(),
    _value()
  {
    std::ifstream infile( filename, std::ios_base::in );

    if ( ! infile.is_open() )
    {
      std::string string( "Failed to open file \"" );
      string += filename;
      string += "\"";
      throw Exception( string );
    }

    this->_load( infile );
  }


  Object::Object( std::istream& input, size_t& ln ) :
    _children(),
    _value()
  {
    this->_load( input, ln );
  }


  Object::~Object()
  {
    for ( ObjectMap::iterator it = _children.begin(); it != _children.end(); ++it )
    {
      delete it->second;
    }
    _children.clear();
  }


  const std::string& Object::asString() const
  {
    if ( this->isObject() )
    {
      throw Exception( "Cannot cast object to string" );
    }

    return _value;
  }


  int Object::asInt() const
  {
    if ( this->isObject() )
    {
      throw Exception( "Cannot cast object to integer" );
    }

    int result = stoi( _value );

    return result;
  }


  float Object::asFloat() const
  {
    if ( this->isObject() )
    {
      throw Exception( "Cannot cast object to float" );
    }

    float result = stof( _value );

    return result;
  }


  double Object::asDouble() const
  {
    if ( this->isObject() )
    {
      throw Exception( "Cannot cast object to double" );
    }

    double result = std::stod( _value );

    return result;
  }


  double Object::asBool() const
  {
    if ( this->isObject() )
    {
      throw Exception( "Cannot cast object to bool" );
    }

    bool result;
    result << _value;
    if ( _value.fail() )
    {
      throw Exception( "Could not convert object to bool" );
    }
    return result;
  }


  Object& Object::get( std::string identifier ) const
  {
    ObjectMap::iterator found = _children.find( identifier );
    if ( found == _children.end() )
    {
      std::string string = "Could not file identifier \";
      string += identifier;
      string += "\" in children";
      throw Exception( string );
    }

    return *found->second;
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // The parsing logic

  void Object::_load( std::istream& input, size_t& lineNumber )
  {
    // Current character being switch-ed
    char c;
    // Current string buffer
    std::string current_string;

    // Curent id being built
    std::string current_id;

    // Current value being build
    std::string current_value;

    // State flags
    bool escape = false;
    bool quote = false;

    enum Status { Identifier, Colon, Value, Comma };
    Status status = Identifier; // First string must be an identifier

    while ( input >> c )
    {

      switch( *it )
      {
        case '\n' :
          ++lineNumber;
          if ( quote && !escape )
          {
            current_string += c;
          }
          if ( escape ) escape = false;
          break;


        case '{' :
          if ( escape || quote )
          {
            current_string += c;
          }
          else
          {
            // Make a new object!
          }

          if ( escape ) escape = false;
          break;


        case '}' :
          if ( escape || quote )
          {
            current_string += c;
          }
          else
          {
            // Object finished!
          }

          if ( escape ) escape = false;
          break;


        case '\"' :
          if ( escape )
          {
            current_string += c;
          }
          else if ( quote ) // End quote
          {
            status = Comma;
          }
          else // Start quote
          {
            quote = true;
          }

          if ( escape ) escape = false;
          break;


        case ' ' :
          if ( quote )
          {
            current_string += c;
          }
          else
          {
            if ( status == Identifier )
            {
              status = Colon;
              current_id = current_string;
              current_string.clear();
            }
            else if ( status == Value )
            {
              current_value = current_string;
              current_string.clear();
              status = Comma;
            }
          }

          if ( escape ) escape = false;
          break;


        case ',' :


        case ':' :


        case '\\' :
          if ( ! escape )
          {
            escape = true;
          }
          else
          {
            current_string += c;
          }
          break;


        default:
          current_string += c;

          if ( escape ) escape = false;
          break;
      }

    }
  }

}

