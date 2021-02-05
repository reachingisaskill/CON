
#include "CON.h"

#include <iostream>

#ifndef CON_BUFFER_SIZE
#define CON_BUFFER_SIZE 5000
#endif

namespace CON
{

////////////////////////////////////////////////////////////////////////////////////////////////////
  // Some useful function declarations

  bool validateNumeric( std::string );


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Exception function definiions

  Exception::Exception( ErrorList& errors ) :
    _errors( errors),
    _what(),
    _parseError( true )
  {
    std::stringstream ss;
    ss << "Found " << _errors.size() << " parse errors";
    _what = ss.str();
  }


  Exception::Exception( std::string error ) :
    _errors(),
    _what( error ),
    _parseError( false )
  {
  }


  const char* Exception::what() const noexcept
  {
    return _what.c_str();
  }


  void Exception::setFilename( std::string file )
  {
    _filename = file;
    _what += std::string( " from file: " ) + _filename;
  }

////////////////////////////////////////////////////////////////////////////////////////////////////
  // CON Object member function definitions

  Object::Object() :
    _children(),
    _array(),
    _value(),
    _type( Type::Null )
  {
  }


  Object::Object( Type t ) :
    _children(),
    _array(),
    _value(),
    _type( t )
  {
  }


  Object::Object( const Object& other ) :
    _children(),
    _array(),
    _value( other._value ),
    _type( other._type )
  {
    for ( ObjectMap::const_iterator it = other._children.begin(); it != other._children.end() ; ++it )
    {
      _children[it->first] = new Object( *it->second );
    }
    for ( Array::const_iterator it = other._array.begin(); it != other._array.end() ; ++it )
    {
      _array.push_back( new Object( *(*it) ) );
    }
  }


  Object::Object( Object&& other ) :
    _children( std::move( other._children ) ),
    _array( std::move( other._array ) ),
    _value( std::move( other._value ) ),
    _type( std::move( other._type ) )
  {
  }


  Object& Object::operator=( const Object& other )
  {
    for ( ObjectMap::iterator it = _children.begin(); it != _children.end() ; ++it )
    {
      delete it->second;
    }
    _children.clear();
    for ( Array::iterator it = _array.begin(); it != _array.end() ; ++it )
    {
      delete *it;
    }
    _array.clear();

    _value = other._value;
    _type = other._type;

    for ( ObjectMap::const_iterator it = other._children.begin(); it != other._children.end() ; ++it )
    {
      _children[it->first] = new Object( *it->second );
    }
    for ( Array::iterator it = _array.begin(); it != _array.end() ; ++it )
    {
      _array.push_back( new Object( *(*it) ) );
    }

    return *this;
  }


  Object& Object::operator=( Object&& other )
  {
    for ( ObjectMap::iterator it = _children.begin(); it != _children.end() ; ++it )
    {
      delete it->second;
    }
    _children.clear();
    for ( Array::iterator it = _array.begin(); it != _array.end() ; ++it )
    {
      delete *it;
    }
    _array.clear();


    _value = std::move( other._value );
    _children = std::move( other._children );
    _array = std::move( other._array );
    _type = std::move( other._type );

    return *this;
  }


  Object::~Object()
  {
    for ( ObjectMap::iterator it = _children.begin(); it != _children.end(); ++it )
    {
      delete it->second;
    }
    _children.clear();
    for ( Array::iterator it = _array.begin(); it != _array.end() ; ++it )
    {
      delete *it;
    }
    _array.clear();
  }


  size_t Object::getSize() const
  {
    switch( _type )
    {
      case Type::Null :
        return 0;
        break;
        
      case Type::String :
      case Type::Numeric :
      case Type::Boolean :
        return 1;
        break;

      case Type::Array :
        return _array.size();
          break;

      case Type::Object :
        return _children.size();
          break;
    }
    return 0;
  }


  void Object::setType( Type type )
  {
    if ( _type == type ) return;
    switch( _type )
    {
      case Type::Null :
        break;

      case Type::String :
      case Type::Numeric :
      case Type::Boolean :
        switch ( type )
        {
          case Type::String :
          case Type::Numeric :
          case Type::Boolean :
            break;

          case Type::Null :
          case Type::Array :
          case Type::Object :
            _value.clear();
            break;
        }
        break;

      case Type::Array :
        switch ( type )
        {
          case Type::Array :
            break;

          case Type::String :
          case Type::Numeric :
          case Type::Boolean :
          case Type::Null :
          case Type::Object :
            for ( Array::iterator it = _array.begin(); it != _array.end() ; ++it )
            {
              delete *it;
            }
            _array.clear();
            break;
        }
        break;

      case Type::Object :
        switch ( type )
        {
          case Type::Object :
            break;

          case Type::String :
          case Type::Numeric :
          case Type::Boolean :
          case Type::Null :
          case Type::Array :
            for ( ObjectMap::iterator it = _children.begin(); it != _children.end(); ++it )
            {
              delete it->second;
            }
            _children.clear();
            break;
        }
        break;
    }

    _type = type;
  }


  void Object::setValue( std::string val )
  {
    setType( Type::String );
    _value = val;
  }


  void Object::setValue( const char* val )
  {
    setType( Type::String );
    _value = val;
  }


  void Object::setValue( int val )
  {
    setType( Type::Numeric );
    _value = std::to_string( val );
  }


  void Object::setValue( long val )
  {
    setType( Type::Numeric );
    _value = std::to_string( val );
  }


  void Object::setValue( float val )
  {
    setType( Type::Numeric );
    _value = std::to_string( val );
  }


  void Object::setValue( double val )
  {
    setType( Type::Numeric );
    _value = std::to_string( val );
  }


  void Object::setValue( bool val )
  {
    setType( Type::Boolean );
    if ( val )
      _value = "true";
    else
      _value = "false";
  }


  void Object::addChild( std::string name, Object obj )
  {
    setType( Type::Object );

    ObjectMap::iterator found = _children.find( name );
    if ( found != _children.end() )
    {
      delete found->second;
      found->second = new Object( obj );
    }
    else
    {
      _children[name] = new Object( obj );
    }
  }


  bool Object::has( std::string name ) const
  {
    if ( _children.find( name ) != _children.end() )
    {
      return true;
    }
    else
    {
      return false;
    }
  }


  void Object::setRawValue( std::string string, Type type )
  {
    switch ( type )
    {
      case Type::Null :
        _value = "";
        _type = type;
        break;

      case Type::String :
        _value = string;
        _type = type;
        break;

      case Type::Numeric :
        if ( validateNumeric( string ) )
        {
          _value = string;
          _type = type;
        }
        else
        {
          throw Exception( "String is not a valid numeric type." );
        }
        break;

      case Type::Boolean :
        if ( ( string == "true" ) || ( string == "false" ) )
        {
          _value = string;
          _type = type;
        }
        else
        {
          throw Exception( "String is not a valid boolean type." );
        }
        break;

      case Type::Object :
        throw Exception( "Cannot set a value for an object type." );

      case Type::Array :
        throw Exception( "Cannot set a value for an array type." );
    }
  }


  const std::string& Object::asString() const
  {
    if ( _type == Type::Object || _type == Type::Array )
    {
      throw Exception( "Cannot cast object or arrays to a value type." );
    }

    return _value;
  }


  char Object::asChar() const
  {
    if ( _type == Type::Object || _type == Type::Array )
    {
      throw Exception( "Cannot cast object or arrays to a value type." );
    }

    if ( _type != Type::String )
    {
      throw Exception( "Type is not string. Cannot convert to char." );
    }

    return _value[0];
  }


  int Object::asInt() const
  {
    if ( _type == Type::Object || _type == Type::Array )
    {
      throw Exception( "Cannot cast object or arrays to a value type." );
    }

    if ( _type != Type::Numeric )
    {
      throw Exception( "Type is not numeric. Cannot convert to int." );
    }

    return std::stoi( _value );
  }


  float Object::asFloat() const
  {
    if ( _type == Type::Object || _type == Type::Array )
    {
      throw Exception( "Cannot cast object or arrays to a value type." );
    }

    if ( _type != Type::Numeric )
    {
      throw Exception( "Type is not numeric. Cannot convert to float." );
    }

    return std::stof( _value );
  }


  double Object::asDouble() const
  {
    if ( _type == Type::Object || _type == Type::Array )
    {
      throw Exception( "Cannot cast object or arrays to a value type." );
    }

    if ( _type != Type::Numeric )
    {
      throw Exception( "Type is not numeric. Cannot convert to double." );
    }

    return std::stod( _value );
  }


  bool Object::asBool() const
  {
    if ( _type == Type::Object || _type == Type::Array )
    {
      throw Exception( "Cannot cast object or arrays to a value type." );
    }

    if ( _type != Type::Boolean )
    {
      throw Exception( "Type is not boolean." );
    }

    if ( _value == "true" )
    {
      return true;
    }
    else
    {
      return false;
    }
  }


  Object& Object::get( std::string identifier )
  {
    if ( _type != Type::Object )
    {
      throw Exception( "Calling get(identifier) when not an object type" );
    }

    ObjectMap::iterator found = _children.find( identifier );
    if ( found == _children.end() )
    {
      std::string string = "Could not file identifier \"";
      string += identifier;
      string += "\" in children";
      throw Exception( string );
    }

    return *found->second;
  }


  const Object& Object::get( std::string identifier ) const
  {
    if ( _type != Type::Object )
    {
      throw Exception( "Calling get(identifier) when not an object type" );
    }

    ObjectMap::const_iterator found = _children.find( identifier );
    if ( found == _children.end() )
    {
      std::string string = "Could not file identifier \"";
      string += identifier;
      string += "\" in children";
      throw Exception( string );
    }

    return *found->second;
  }


  Object& Object::get( size_t id )
  {
    if ( _type != Type::Array )
    {
      throw Exception( "Calling get(size_t) when not an array type" );
    }

    if ( id > _array.size() )
    {
      std::stringstream string;
      string << "Array index " << id << " outside array bounds: " << _array.size();
      throw Exception( string.str() );
    }

    return *_array[id];
  }


  const Object& Object::get( size_t id ) const
  {
    if ( _type != Type::Array )
    {
      throw Exception( "Calling get(size_t) when not an array type" );
    }

    if ( id > _array.size() )
    {
      std::stringstream string;
      string << "Array index " << id << " outside array bounds: " << _array.size();
      throw Exception( string.str() );
    }

    return *_array[id];
  }


  void Object::push( Object& obj )
  {
    setType( Type::Array );
    _array.push_back( new Object( obj ) );
  }


  void Object::push( std::string s )
  {
    setType( Type::Array );
    Object* obj = new Object( Type::String );
    obj->setValue( s );
    _array.push_back( obj );
  }


  void Object::push( char c )
  {
    setType( Type::Array );
    Object* obj = new Object( Type::String );
    obj->setValue( c );
    _array.push_back( obj );
  }


  void Object::push( int i )
  {
    setType( Type::Array );
    Object* obj = new Object( Type::Numeric );
    obj->setValue( i );
    _array.push_back( obj );
  }


  void Object::push( long l )
  {
    setType( Type::Array );
    Object* obj = new Object( Type::Numeric );
    obj->setValue( l );
    _array.push_back( obj );
  }


  void Object::push( float f )
  {
    setType( Type::Array );
    Object* obj = new Object( Type::Numeric );
    obj->setValue( f );
    _array.push_back( obj );
  }


  void Object::push( double d )
  {
    setType( Type::Array );
    Object* obj = new Object( Type::Numeric );
    obj->setValue( d );
    _array.push_back( obj );
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Comparison operators

  bool Object::operator==( Object& other ) const
  {
    if ( other._type != this->_type ) 
    {
      return false;
    }

    switch( _type )
    {
      case Type::Null:
        return true;
        break;

      case Type::String :
      case Type::Numeric :
      case Type::Boolean :
        if ( this->_value == other._value )
        {
          return true;
        }
        else
        {
          return false;
        }
        break;

      case Type::Array :
        {
          Array::const_iterator this_it = this->_array.begin();
          Array::const_iterator that_it = other._array.begin();
          while ( this_it != this->_array.end() && that_it != other._array.end() )
          {
            if ( *(*this_it) != *(*that_it ) ) return false;
            ++this_it;
            ++that_it;
          }
          if ( this_it == this->_array.end() && that_it == other._array.end() ) return true;
          else
          {
            return false;
          }
        }
        break;

      case Type::Object:
        {
          ObjectMap::const_iterator this_it = this->_children.begin();
          ObjectMap::const_iterator that_it = other._children.begin();
          while ( this_it != this->_children.end() && that_it != other._children.end() )
          {
            if ( this_it->first != that_it->first )
            {
              return false;
            }
            if ( *this_it->second != *that_it->second )
            {
              return false;
            }
            ++this_it;
            ++that_it;
          }
          if ( this_it == this->_children.end() && that_it == other._children.end() ) return true;
          else
          {
            return false;
          }
        }
    }

    return true;
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // The parsing data structures

  struct Token
  {
    enum Type { Text, Quote, Colon, Comma, OpenObject, CloseObject, OpenArray, CloseArray, Comment, Filepath };

    std::string string;
    Type type;
    size_t lineNumber;
  };


  // Turn a vector of tokens into a complete object tree
  Object parseTokens( std::vector<Token>::iterator&, std::vector<Token>::iterator&, ErrorList& );

  void parseArray( std::vector<Token>::iterator&, std::vector<Token>::iterator&, ErrorList&, Object& );

////////////////////////////////////////////////////////////////////////////////////////////////////
  // Errors and validation
  std::string makeError( size_t ln, std::string err )
  {
    std::stringstream ss;
    ss << "Error on line " << ln << ": " << err << '.';
    return ss.str();
  }


  // Validates a string to be numeric or boolean exactly
  bool validateExpression( std::string& text, Type& valid_type )
  {
    if ( text == "true" )
    {
      valid_type = Type::Boolean;
      return true;
    }
    else if ( text == "false" )
    {
      valid_type = Type::Boolean;
      return true;
    }
    else if ( text == "null" )
    {
      valid_type = Type::Null;
      return true;
    }
    else if ( validateNumeric( text ) )
    {
      valid_type = Type::Numeric;
      return true;
    }
    else
    {
      valid_type = Type::String;
      return false;
    }
  }


  bool validateNumeric( std::string text )
  {
    bool digit = false;
    bool point = false;
    for ( std::string::const_iterator it = text.begin(); it != text.end(); ++it )
    {
      if ( std::isdigit( *it ) )
      {
        digit = true;
      }
      else if ( ( (*it) == '-' ) || ( (*it) == '+' ) )
      {
        if ( digit ) return false;
      }
      else if ( (*it) == '.' )
      {
        if ( point ) return false;
        if ( ! digit ) return false;
      }
      else
      {
        return false;
      }
    }
    return true;
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // The writing logic

  void parseQuote( std::ostream& output, const std::string& text )
  {
    output << "\"";
    for ( std::string::const_iterator it = text.begin(); it != text.end(); ++it )
    {
      switch ( *it )
      {
        case '"' :
          output << "\\\"";
          break;

        case '\\' :
          output << "\\\\";
          break;

        default :
          output << *it;
      }
    }
    output << "\"";
  }


  void printObject( Object& obj, std::ostream& output, size_t indent )
  {
    auto indentLambda = [ &output, &indent ]() { for ( size_t i = 0; i < indent; ++i ) output << "  "; };

    if ( obj._type == Type::Object )
    {
      indentLambda();
      output << "{" << '\n';
      ++indent;
      for ( Object::ObjectMap::iterator it = obj._children.begin(); it != obj._children.end(); ++it )
      {
        indentLambda();
        output << it->first << " : ";
        if ( it->second->getType() == Type::Object || it->second->getType() == Type::Array ) output << '\n';
        printObject( *it->second, output, indent );
        if ( it != ( --obj._children.end() ) )
        {
          output << ",\n";
        }
        else
        {
          output << "\n";
        }
      }
      --indent;
      indentLambda();
      output << "}";
    }
    else if ( obj._type == Type::Array )
    {
      indentLambda();
      output << "[" << '\n';
      ++indent;
      for ( Object::Array::iterator it = obj._array.begin(); it != obj._array.end(); ++it )
      {
        if ( (*it)->getType() != Type::Object && (*it)->getType() != Type::Array ) indentLambda();
        printObject( *(*it), output, indent );
        if ( it != ( --obj._array.end() ) )
        {
          output << ",\n";
        }
        else
        {
          output << "\n";
        }
      }
      --indent;
      indentLambda();
      output << "]";
    }
    else
    {
      switch( obj._type )
      {
        case Type::Null :
          output << "null";
          break;

        case Type::String :
          parseQuote( output, obj._value );
          break;

        case Type::Numeric :
          output << obj._value;
          break;

        case Type::Boolean :
          output << obj._value;
          break;

        default:
          // This should be impossible
          break;
      }
    }
  }


  void writeToStream( Object& obj, std::ostream& output )
  {
    printObject( obj, output, 0 );
    output << std::endl;
  }


  void writeToString( Object& obj, std::string& output )
  {
    std::stringstream ss;
    printObject( obj, ss, 0 );
    ss << std::endl;
    output = ss.str();
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // The parsing logic

  Object buildFromFile( std::string filename )
  {
    std::ifstream infile( filename, std::ios_base::in );

    if ( ! infile.is_open() )
    {
      std::string string( "Failed to open file \"" );
      string += filename;
      string += "\"";
      throw Exception( string );
    }

    Object object;
    try
    {
      object = buildFromStream( infile );
    }
    catch( Exception& ex )
    {
      ex.setFilename( filename );
      throw ex;
    }
    return object;
  }


  Object buildFromString( std::string& data )
  {
    std::stringstream ss( data );

    return buildFromStream( ss );
  }


  Object buildFromStream( std::istream& input )
  {
    // We need to analyse the white space
    input.unsetf( std::ios_base::skipws );

    // Current character being switch-ed
    char c;
    // Current string buffer
    std::string current_string;

    // Current line number
    size_t lineNumber = 1;

    // Token list
    std::vector< Token > tokens;

    // State flags
    bool escape = false;
    bool quote = false;
    bool comment = false;
    bool filepath = false;

    Token::Type current_type;

    while ( input >> c )
    {
      if ( comment )
      {
        if ( c == '\n' )
        {
          comment = false;
          ++lineNumber;
        }
        continue;
      }
      else if ( filepath )
      {
        if ( escape )
        {
          if ( c == '\n' )
          {
            comment = false;
            ++lineNumber;
          }

          current_string.push_back( c );
          escape = false;
        }
        else
        {
          switch( c )
          {
            case '\\' :
              escape = true;
              break;

            case '\n' :
              ++lineNumber;
              current_string.push_back( '\n' );
              break;

            case '>' :
              {
                filepath = false;
                Token new_token;
                new_token.string = current_string;
                new_token.type = Token::Filepath;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
                current_string.clear();
              }
              break;

            default:
              current_string.push_back( c );
              break;
          }
        }
      }
      else if ( quote )
      {
        if ( escape )
        {
          if ( c == '\n' )
          {
            comment = false;
            ++lineNumber;
          }

          current_string.push_back( c );
          escape = false;
        }
        else
        {
          switch( c )
          {
            case '\\' :
              escape = true;
              break;

            case '\n' :
              ++lineNumber;
              current_string.push_back( '\n' );
              break;

            case '\"' :
              {
                quote = false;
                Token new_token;
                new_token.string = current_string;
                new_token.type = Token::Quote;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
                current_string.clear();
              }
              break;

            default:
              current_string.push_back( c );
              break;
          }
        }
      }
      else
      {
        if ( escape )
        {
          if ( current_string.size() == 0 )
          {
            current_type = Token::Text;
          }
          escape = false;
        }
        else
        {
          switch( c )
          {
            case '\n' :
              ++lineNumber;
              break;


            case '\\' :
              if ( escape ) current_string.push_back( '\\' );
              else escape = true;
              break;

            case ' ' :
              if ( current_string.size() > 0 )
              {
                Token new_token;
                new_token.string = current_string;
                new_token.type = current_type;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
                current_string.clear();
              }
              break;



            case '{' :
              if ( escape ) current_string.push_back( '{' );
              else
              {
                if ( current_string.size() > 0 )
                {
                  Token new_token;
                  new_token.string = current_string;
                  new_token.type = current_type;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                  current_string.clear();
                }
                {
                  Token new_token;
                  new_token.string = '{';
                  new_token.type = Token::OpenObject;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                }
              }
              break;


            case '}' :
              if ( escape ) current_string.push_back( '}' );
              else
              {
                if ( current_string.size() > 0 )
                {
                  Token new_token;
                  new_token.string = current_string;
                  new_token.type = current_type;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                  current_string.clear();
                }
                {
                  Token new_token;
                  new_token.string = '}';
                  new_token.type = Token::CloseObject;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                }
              }
              break;


            case '[' :
              if ( escape ) current_string.push_back( '[' );
              else
              {
                if ( current_string.size() > 0 )
                {
                  Token new_token;
                  new_token.string = current_string;
                  new_token.type = current_type;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                  current_string.clear();
                }
                {
                  Token new_token;
                  new_token.string = '[';
                  new_token.type = Token::OpenArray;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                }
              }
              break;


            case ']' :
              if ( escape ) current_string.push_back( ']' );
              else
              {
                if ( current_string.size() > 0 )
                {
                  Token new_token;
                  new_token.string = current_string;
                  new_token.type = current_type;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                  current_string.clear();
                }
                {
                  Token new_token;
                  new_token.string = ']';
                  new_token.type = Token::CloseArray;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                }
              }
              break;


            case '\"' :
              if ( current_string.size() > 0 )
              {
                Token new_token;
                new_token.string = current_string;
                new_token.type = current_type;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
                current_string.clear();
              }

              quote = true;
              break;


            case '<' :
              if ( current_string.size() > 0 )
              {
                Token new_token;
                new_token.string = current_string;
                new_token.type = current_type;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
                current_string.clear();
              }

              filepath = true;
              break;


            case ',' :
              if ( current_string.size() > 0 )
              {
                Token new_token;
                new_token.string = current_string;
                new_token.type = current_type;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
                current_string.clear();
              }
              {
                Token new_token;
                new_token.string = ',';
                new_token.type = Token::Comma;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
              }
              break;


            case ':' :
              if ( current_string.size() > 0 )
              {
                Token new_token;
                new_token.string = current_string;
                new_token.type = current_type;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
                current_string.clear();
              }
              {
                Token new_token;
                new_token.string = ':';
                new_token.type = Token::Colon;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
              }
              break;


            case '#' :
              comment = true;
              break;


            default:
              if ( current_string.size() == 0 )
              {
                current_type = Token::Text;
              }
              current_string += c;
              break;
          }
        }
      }
    }

    /*
    // Useful debugging loop
    for ( std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it )
    {
      std::cout << "Token : ";
      switch( it->type )
      {
        case Token::Text :
          std::cout << " Text ";
          break;
        case Token::Quote :
          std::cout << " Quote ";
          break;
        case Token::Colon :
          std::cout << " Colon ";
          break;
        case Token::Comma :
          std::cout << " Comma ";
          break;
        case Token::OpenObject :
          std::cout << " OpenObject ";
          break;
        case Token::CloseObject :
          std::cout << " CloseObject ";
          break;
        case Token::OpenArray :
          std::cout << " OpenArray ";
          break;
        case Token::CloseArray :
          std::cout << " CloseArray ";
          break;
        case Token::Comment :
          std::cout << " Comment ";
          break;
        case Token::Filepath :
          std::cout << " File ";
          break;
      }
      std::cout << it->lineNumber << " : " << it->string << std::endl;
    }
    */

    // Find the start of the root node
    std::vector<Token>::iterator root_begin = tokens.begin();
    std::vector<Token>::iterator root_end = tokens.end();

    while( (root_begin != tokens.end()) && (root_begin->type != Token::OpenObject) ) ++root_begin;

    if ( root_begin == tokens.end() )
    {
      throw Exception( "Could not find root object in data stream." );
    }

    ErrorList errorList;
    Object object = parseTokens( ++root_begin, root_end, errorList );

    if ( errorList.size() > 0 )
      throw Exception( errorList );

    return object;
  }


  Object parseTokens( std::vector<Token>::iterator& start, std::vector<Token>::iterator& end, ErrorList& errors )
  {
    // Awful file
    if ( start == end )
    {
      throw Exception( "Unexpected end of file" );
    }

    std::vector<Token>::iterator current = start;
    Object object( Type::Object );

    // The empty object
    if ( current->type == Token::CloseObject )
    {
      start = ++current;
      return object;
    }

    std::string identifier;

    // Iterate through the tokens
    while ( current != end )
    {
//////////////////// The Identifier
      if ( current->type != Token::Text )
      {
        errors.push_back( makeError( current->lineNumber, "Valid identifier expected" ) );
//        throw Exception( errors );
        ++current;
      }
      else
      {
        identifier = current->string;
        ++current;
      }

//////////////////// Colon Separator
      if ( current == end )
      {
        errors.push_back( makeError( (--current)->lineNumber, std::string( "Colon expected following identifier: " ) + identifier ) );
        throw Exception( errors );
      }
      else if ( current->type != Token::Colon )
      {
        errors.push_back( makeError( current->lineNumber, std::string( "Colon expected following identifier: " ) + identifier ) );
//        throw Exception( errors );
      }
      else
      {
        ++current;
      }

//////////////////// Value expression
      if ( current == end )
      {
        errors.push_back( makeError( (--current)->lineNumber, std::string( "Value expected for identifier " ) + identifier ) );
        throw Exception( errors );
      }
      else if ( current->type == Token::Text ) 
      {
        Type valid_type;
        if ( ! validateExpression( current->string, valid_type ) )
        {
          errors.push_back( makeError( current->lineNumber, std::string( "Invalid expression. Must be boolean, numeric or string" ) ) );
        }
        else
        {
          Object child;
          child.setRawValue( current->string, valid_type );
          object.addChild( identifier, child );
        }

        ++current;
      }
      else if ( current->type == Token::Quote )
      {
        Object child( Type::String );

        child.setValue( current->string );
        object.addChild( identifier, child );

        ++current;
      }
      else if ( current->type == Token::Filepath )
      {
        try
        {
          object.addChild( identifier, buildFromFile( current->string ) );
        }
        catch( Exception& ex )
        {
          throw ex;
        }
        ++current;
      }
      else if ( current->type == Token::OpenObject )
      {
        ++current;
        try
        {
          object.addChild( identifier, parseTokens( current, end, errors ) );
        }
        catch( Exception& ex )
        {
          throw ex;
        }
      }
      else if ( current->type == Token::OpenArray )
      {
        try
        {
          Object child( Type::Array );
          parseArray( ++current, end, errors, child );
          object.addChild( identifier, child );
        }
        catch ( Exception& ex )
        {
          throw ex;
        }
      }
      else
      {
        errors.push_back( makeError( current->lineNumber, std::string( "Invalid value for identifier " ) + identifier ) );
        throw Exception( errors );
      }

//////////////////// Comma or closing bracket
      if ( current == end )
      {
        errors.push_back( makeError( start->lineNumber, "Closing bracket not found" ) );
//        throw Exception( errors );
      }
      else if ( current->type == Token::Comma )
      {
        ++current;
        continue;
      }
      else if ( current->type == Token::CloseObject )
      {
        start = ++current;
        return object;
      }
      else
      {
        errors.push_back( makeError( current->lineNumber, "Expected either comma or closing bracket" ) );
//        throw Exception( errors );
      }
    }

    errors.push_back( makeError( start->lineNumber, "Closing bracket not found" ) );
    return object;
  }


  void parseArray( std::vector<Token>::iterator& start, std::vector<Token>::iterator& end, ErrorList& errors, Object& object )
  {
    if ( start == end )
    {
      throw Exception( "Unexpected end of file" );
    }

    std::vector<Token>::iterator current = start;

    // The empty array
    if ( current->type == Token::CloseArray )
    {
      start = ++current;
      return;
    }

    while ( current != end )
    {
      if ( current->type == Token::Text )
      {
        Type valid_type;
        if ( ! validateExpression( current->string, valid_type ) )
        {
          errors.push_back( makeError( current->lineNumber, std::string( "Invalid expression. Must be boolean, numeric or string" ) ) );
        }
        else
        {
          Object child( valid_type );
          child.setRawValue( current->string, valid_type );
          object.push( child );
        }
        ++current;
      }
      else if ( current->type == Token::Quote )
      {
        Object child;

        child.setValue( current->string );
        object.push( child );

        ++current;
      }
      else if ( current->type == Token::Filepath )
      {
        try
        {
          Object child = buildFromFile( current->string );
          object.push( child );
        }
        catch( Exception& ex )
        {
          throw ex;
        }
        ++current;
      }
      else if ( current->type == Token::OpenObject )
      {
        try
        {
          Object child = parseTokens( ++current, end, errors );
          object.push( child );
        }
        catch( Exception& ex )
        {
          throw ex;
        }
      }
      else if ( current->type == Token::OpenArray )
      {
        try
        {
          Object child( Type::Array );
          parseArray( ++current, end, errors, child );
          object.push( child );
        }
        catch ( Exception& ex )
        {
          throw ex;
        }
      }
      else if ( current->type == Token::CloseArray )
      {
        errors.push_back( makeError( current->lineNumber, std::string( "Array ended unexpectedly" ) ) );
        start = ++current;
        return;
      }
      else
      {
        errors.push_back( makeError( current->lineNumber, std::string( "Expected valid value type, object or array within array defitinition" ) ) );
        ++current;
      }

      if ( current->type == Token::Comma )
      {
        ++current;
        continue;
      }
      else if ( current->type == Token::CloseArray )
      {
        start = ++current;
        return;
      }
      else
      {
        errors.push_back( makeError( current->lineNumber, std::string( "Expected comma or closing bracket following array item" ) ) );
        ++current;
      }

    }

    errors.push_back( makeError( start->lineNumber, "Closing square bracket not found" ) );
    start = current;
    return;
  }

}

