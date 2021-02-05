
#include "CON.h"

#include <iostream>
#include <fstream>

const char* tempFileName = "./.temp/intermediate_file.con";


int main( int, char** )
{

  std::cout << "Building some by hand." << std::endl;

  try
  {
    CON::Object root( CON::Type::Object );

    root.addChild( "hello", CON::Object( CON::Type::String ) );

    root["hello"].setValue( "This is a string" );

    std::cout << "Making array object" << std::endl;
    CON::Object array_obj( CON::Type::Array );

    std::cout << "Making nestled object" << std::endl;
    CON::Object nestled_obj( CON::Type::Object );
    nestled_obj.addChild( "Child1", CON::Object( CON::Type::Null ) );
    nestled_obj.addChild( "Child2", CON::Object( CON::Type::Null ) );
    nestled_obj.addChild( "Child3", CON::Object( CON::Type::Null ) );
    nestled_obj.addChild( "Child4", CON::Object( CON::Type::String ) );

    nestled_obj["Child4"].setValue( "Hello again!" );

    std::cout << "Filling array" << std::endl;
    array_obj.push( "1" );
    array_obj.push( "2" );
    array_obj.push( "3" );
    array_obj.push( nestled_obj );
    array_obj.push( "4" );

    root.addChild( "test_array", array_obj );

    std::ofstream temp_file( tempFileName, std::ios_base::out );
    CON::writeToStream( root, std::cout );

    std::cout << "Writing temp file" << std::endl;
    CON::writeToStream( root, temp_file );
    temp_file.close();
    std::cout << "Reading temp file" << std::endl;

    CON::Object new_version = CON::buildFromFile( tempFileName );
    CON::writeToStream( new_version, std::cout );



    std::cout << std::endl;
    if ( root == new_version )
    {
      std::cout << "They are identical!" << std::endl;
    }
    else
    {
      std::cout << "They are NOT identical!" << std::endl;
    }

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

