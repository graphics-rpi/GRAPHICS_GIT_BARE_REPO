#ifndef __WALLFILE_PARSER__
#define __WALLFILE_PARSER__
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include "person.hpp"

using std::ifstream;
using std::string;
using std::cout;
using std::endl;
using std::vector;


class WallfileParser{
  public:
  //A class to hold the position and direction of "people" on the tabletop

  //Given a file to parse a vector of Persons is returned (positions and directions).
  vector<Person> getPersonVector(string filename)
  {
    vector<Person> People;
    ifstream infile;
    infile.open(filename.c_str());
    int lineNum=1;
    string line;

    float a1,a2,b1,b2;
    int personCount=0;
    while(std::getline(infile, line))
    {
      //Person in parsed in a standard wallfile format
      if(4==sscanf(line.c_str(), "person %f %f %f %f", &a1,&a2,&b1,&b2))
      {
        printf("found person at %f %f %f %f \n", a1,a2,b1,b2);
        personCount++;
        People.push_back(Person(a1,a2,b1,b2));
      }
      //For all lines that arent persons
      else
        printf("no person here %d %s \n", lineNum++, line.c_str());
    }
    printf("found %d persons\n", personCount);
    
    //The vector of persons is returned to caller.
    return People;
  }
};
#endif
