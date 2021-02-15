
#include "CON.h"

#include <iostream>
#include <vector>
#include <string>


typedef std::vector< std::string > StringVector;
typedef std::vector< StringVector > PathVector;


void failWithHelp();


int main( int argN, char** argV )
{
  PathVector paths;

  if ( argN < 2 )
  {
    failWithHelp();
    return 1;
  }
  else
  {
    for ( int arg_count = 1; arg_count < argN; ++arg_count )
    {
      StringVector path;
      std::string current_string;
      char *pointer = argV[arg_count];

      if ( *pointer == '/' ) ++pointer;

      while ( *pointer != '\0' )
      {
        if ( *pointer == '/' )
        {
          path.push_back( current_string );
          current_string.clear();
        }
        else
        {
          current_string.push_back( *pointer );
        }

        ++pointer;
      }

      if ( current_string.size() > 0 )
      {
        path.push_back( current_string );
      }

      paths.push_back( path );
    }
  }


  try
  {
    CON::Object the_object = CON::buildFromStream( std::cin );

    for ( PathVector::const_iterator path_it = paths.begin(); path_it != paths.end(); ++path_it )
    {
      CON::Object* location = &the_object;

      for ( StringVector::const_iterator it = path_it->begin(); it != path_it->end(); ++it )
      {
        location = &location->get( *it );
      }

      std::cout << location->asString() << std::endl;
    }
  }
  catch( CON::Exception& ex )
  {
    std::cerr << "An error occured: " << ex.what() << std::endl;
    for ( CON::Exception::iterator it = ex.begin(); it != ex.end(); ++it )
    {
      std::cerr << (*it) << std::endl;
    }
    return 1;
  }
  catch( std::exception& ex )
  {
    std::cerr << "Unknwon error occured. Exiting." << std::endl;
    return 1;
  }


  return 0;
}

void failWithHelp()
{
  std::cerr << "Usage: con [options] configuration_paths...\n" 
               "  Note: No options are currently supported :p" << std::endl;
}

