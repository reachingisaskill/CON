
#include "CON.h"

#include <iostream>


int main( int, char** )
{

  std::cout << "Building some by hand." << std::endl;

  try
  {
    CON::Object root( CON::Type::Object );

    root.addChild( "hello", CON::Object( CON::Type::String ) );

    root["hello"].setValue( "This is a string" );

    CON::Object array_obj( CON::Type::Array );

    CON::Object nestled_obj( CON::Type::Object );
    nestled_obj.addChild( "Child1", CON::Object( CON::Type::Null ) );
    nestled_obj.addChild( "Child2", CON::Object( CON::Type::Null ) );
    nestled_obj.addChild( "Child3", CON::Object( CON::Type::Null ) );

    array_obj.push( "1" );
    array_obj.push( "2" );
    array_obj.push( "3" );
    array_obj.push( nestled_obj );
    array_obj.push( "4" );

    root.addChild( "test_array", array_obj );

    CON::writeToStream( root, std::cout );

  }
  catch ( CON::Exception& ex )
  {
    std::cerr << "Error : " << ex.what() << std::endl;

    if ( ex.number() )
    {
      for ( CON::Exception::iterator it = ex.begin(); it != ex.end(); ++it )
      {
        std::cerr << (*it) << std::endl;
      }
    }
  }

  return 0;
}

