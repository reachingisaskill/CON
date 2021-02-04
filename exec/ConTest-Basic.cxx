
#include "CON.h"

#include <iostream>


int main( int, char** )
{

  std::cout << "Building root CON object." << std::endl;

  CON::Object test_obj( "./dat/test-basic.con" );

  std::cout << "Built\n\nIdentifier = " << object["identifier"].asString();

  return 0;
}

