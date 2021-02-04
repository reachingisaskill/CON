
#include "CON.h"

#include <iostream>

#ifndef CON_BUFFER_SIZE
#define CON_BUFFER_SIZE 5000
#endif

namespace CON
{

////////////////////////////////////////////////////////////////////////////////////////////////////
  // Exception function definiions

  Exception::Exception( size_t lineNumber, std::string error ) :
    _errors(),
    _what(),
    _parseError( true )
  {
    std::stringstream ss;
    ss << "Error on line: " << lineNumber << " : " << error;

    _errors.push_back( ss.str() );

    ss.str( "" );
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


////////////////////////////////////////////////////////////////////////////////////////////////////
  // CON Object member function definitions

  Object::Object() :
    _children(),
    _value()
  {
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


  void Object::addChild( std::string name, Object* obj )
  {
    ObjectMap::iterator found = _children.find( name );
    if ( found != _children.end() )
    {
      delete found->second;
      found->second = obj;
    }
    else
    {
      _children[name] = obj;
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


  Object* Object::get( std::string identifier ) const
  {
    ObjectMap::const_iterator found = _children.find( identifier );
    if ( found == _children.end() )
    {
      std::string string = "Could not file identifier \"";
      string += identifier;
      string += "\" in children";
      throw Exception( string );
    }

    return found->second;
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // The parsing data structures

  struct Token
  {
    enum Type { Text, Quote, Colon, Comma, OpenBracket, CloseBracket, Comment };

    std::string string;
    Type type;
    size_t lineNumber;
  };


  Object* parseTokens( std::vector<Token>::iterator&, std::vector<Token>::iterator& );


////////////////////////////////////////////////////////////////////////////////////////////////////
  // The parsing logic

  Object* buildFromFile( std::string filename )
  {
    std::ifstream infile( filename, std::ios_base::in );

    if ( ! infile.is_open() )
    {
      std::string string( "Failed to open file \"" );
      string += filename;
      string += "\"";
      throw Exception( string );
    }

    return buildFromStream( infile );
  }


  Object* buildFromString( std::string& data )
  {
    std::stringstream ss( data );

    return buildFromStream( ss );
  }


  Object* buildFromStream( std::istream& input )
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
        case Token::OpenBracket :
          std::cout << " OpenBracket ";
          break;
        case Token::CloseBracket :
          std::cout << " CloseBracket ";
          break;
        case Token::Comment :
          std::cout << " Comment ";
          break;
      }
      std::cout << it->lineNumber << " : " << it->string << std::endl;
    }

    // Find the start of the root node
    std::vector<Token>::iterator root_begin = tokens.begin();
    std::vector<Token>::iterator root_end = tokens.end();

    while( (root_begin != tokens.end()) && (root_begin->type != Token::OpenBracket) ) ++root_begin;

    if ( root_begin == tokens.end() )
    {
      throw Exception( "Could not find root object in data stream." );
    }

    Object* new_object = parseTokens( ++root_begin, root_end );
    return new_object;
  }


  Object* parseTokens( std::vector<Token>::iterator& start, std::vector<Token>::iterator& end )
  {
    // Awful file
    if ( start == end )
    {
      throw Exception( "Unexpected end of file" );
    }

    std::vector<Token>::iterator current = start;
    Object* object = new Object();

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
      if ( current->type != Token::Text )
      {
        delete object;
        throw Exception( current->lineNumber, "Valid identifier expected" );
      }
      else
      {
        identifier = current->string;
        ++current;
      }

      if ( current == end )
      {
        delete object;
        throw Exception( (--current)->lineNumber, std::string( "Colon expected following identifier: " ) + identifier );
      }
      else if ( current->type != Token::Colon )
      {
        delete object;
        throw Exception( current->lineNumber, std::string( "Colon expected following identifier: " ) + identifier );
      }
      else
      {
        ++current;
      }

      if ( current == end )
      {
        delete object;
        throw Exception( (--current)->lineNumber, std::string( "Value expected for identifier " ) + identifier );
      }
      else if ( ( current->type == Token::Text ) || ( current->type == Token::Quote ) )
      {
        Object* child = new Object();

        child->setValue( current->string );
        object->addChild( identifier, child );

        ++current;
      }
      else if ( current->type == Token::OpenBracket )
      {
        ++current;
        try
        {
          object->addChild( identifier, parseTokens( current, end ) );
        }
        catch( Exception& ex )
        {
          delete object;
          throw ex;
        }
      }
      else
      {
        delete object;
        throw Exception( current->lineNumber, std::string( "Invalid value for identifier " ) + identifier );
      }

      if ( current == end )
      {
        delete object;
        throw Exception( start->lineNumber, "Closing bracket not found" );
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
        throw Exception( current->lineNumber, "Excpted either comma or closing bracket" );
      }
    }

    throw Exception( start->lineNumber, "Closing bracket not found" );
  }

}

