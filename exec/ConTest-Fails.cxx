
#include "CON.h"

#include <iostream>


int main( int, char** )
{

  std::cout << "Building root CON object." << std::endl;

  try
  {
    CON::Object object = CON::buildFromFile( "./dat/test-fail1.con" );

    writeToStream( object, std::cout );

    std::cout << object.get( "identifier" ).asString() << std::endl;
    std::cout << object.get( "another_id" ).asFloat() << std::endl;
    std::cout << object.get( "some_stuff" ).asString() << std::endl;
    std::cout << std::endl;
    std::cout << object.get( "sub_object" ).get( "yo" ).asString() << std::endl;

  }
  catch ( CON::Exception& ex )
  {
    std::cerr << "Error : " << ex.what() << std::endl;

    for ( CON::Exception::iterator it = ex.begin(); it != ex.end(); ++it )
    {
      std::cerr << (*it) << std::endl;
    }
  }

  return 0;
}

