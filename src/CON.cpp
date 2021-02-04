
#include "CON.h"

#include <iostream>

#ifndef CON_BUFFER_SIZE
#define CON_BUFFER_SIZE 5000
#endif

namespace CON
{

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
    _value()
  {
  }


  Object::Object( const Object& other ) :
    _children(),
    _value( other._value )
  {
    for ( ObjectMap::const_iterator it = other._children.begin(); it != other._children.end() ; ++it )
    {
      _children[it->first] = new Object( *it->second );
    }
  }


  Object::Object( Object&& other ) :
    _children( std::move( other._children ) ),
    _value( std::move( other._value ) )
  {
  }


  Object& Object::operator=( const Object& other )
  {
    for ( ObjectMap::iterator it = _children.begin(); it != _children.end() ; ++it )
    {
      delete it->second;
    }
    _children.clear();

    _value = other._value;

    for ( ObjectMap::const_iterator it = other._children.begin(); it != other._children.end() ; ++it )
    {
      _children[it->first] = new Object( *it->second );
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

    _value = std::move( other._value );
    _children = std::move( other._children );

    return *this;
  }


  Object::~Object()
  {
    for ( ObjectMap::iterator it = _children.begin(); it != _children.end(); ++it )
    {
      delete it->second;
    }
    _children.clear();
  }


  void Object::setValue( std::string val )
  {
    _value = val;
  }


  void Object::addChild( std::string name, Object obj )
  {
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


  bool Object::asBool() const
  {
    if ( this->isObject() )
    {
      throw Exception( "Cannot cast object to bool" );
    }

    bool result = true;
    return result;
  }


  Object& Object::get( std::string identifier )
  {
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


  void Object::print( std::ostream& output, size_t indent )
  {
    auto indentLambda = [ &output, &indent ]() { for ( size_t i = 0; i < indent; ++i ) output << "  "; };

    if ( _children.size() > 0 )
    {
      output << '\n';
      indentLambda();
      output << "{" << '\n';
      ++indent;
      for ( ObjectMap::iterator it = _children.begin(); it != _children.end(); ++it )
      {
        indentLambda();
        output << it->first << " : ";
        it->second->print( output, indent );
      }
      --indent;
      indentLambda();
      output << "{" << '\n';
    }
    else
    {
      output << '\"' << _value << "\",\n";
    }
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // The parsing data structures

  struct Token
  {
    enum Type { Text, Quote, Colon, Comma, OpenBracket, CloseBracket, Comment, Filepath };

    std::string string;
    Type type;
    size_t lineNumber;
  };


  // Turn a vector of tokens into a complete object tree
  Object parseTokens( std::vector<Token>::iterator&, std::vector<Token>::iterator&, ErrorList& );


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Errors and validation
  std::string makeError( size_t ln, std::string err )
  {
    std::stringstream ss;
    ss << "Error on line " << ln << ": " << err << '.';
    return ss.str();
  }

  // Validates a string to be numeric or boolean exactly
  bool validateExpression( std::string& text )
  {
    if ( text == "true" ) return true;
    else if ( text == "false" ) return true;
    else
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
                  new_token.type = Token::OpenBracket;
                  new_token.lineNumber = lineNumber;
                  tokens.push_back( new_token );
                }
              }
              break;


            case '}' :
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
                new_token.type = Token::CloseBracket;
                new_token.lineNumber = lineNumber;
                tokens.push_back( new_token );
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

//    for ( std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it )
//    {
//      std::cout << "Token : ";
//      switch( it->type )
//      {
//        case Token::Text :
//          std::cout << " Text ";
//          break;
//        case Token::Quote :
//          std::cout << " Quote ";
//          break;
//        case Token::Colon :
//          std::cout << " Colon ";
//          break;
//        case Token::Comma :
//          std::cout << " Comma ";
//          break;
//        case Token::OpenBracket :
//          std::cout << " OpenBracket ";
//          break;
//        case Token::CloseBracket :
//          std::cout << " CloseBracket ";
//          break;
//        case Token::Comment :
//          std::cout << " Comment ";
//          break;
//        case Token::Filepath :
//          std::cout << " File ";
//          break;
//      }
//      std::cout << it->lineNumber << " : " << it->string << std::endl;
//    }

    // Find the start of the root node
    std::vector<Token>::iterator root_begin = tokens.begin();
    std::vector<Token>::iterator root_end = tokens.end();

    while( (root_begin != tokens.end()) && (root_begin->type != Token::OpenBracket) ) ++root_begin;

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
    Object object;

    // The empty object
    if ( current->type == Token::CloseBracket )
    {
      start = current;
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
        if ( ! validateExpression( current->string ) )
        {
          errors.push_back( makeError( current->lineNumber, std::string( "Invalid expression. Must be boolean, numeric or string" ) ) );
        }
        else
        {
          Object child;
          child.setValue( current->string );
          object.addChild( identifier, child );
        }

        ++current;
      }
      else if ( current->type == Token::Quote )
      {
        Object child;

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
      else if ( current->type == Token::OpenBracket )
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
      else if ( current->type == Token::CloseBracket )
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

}

