#include <string>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using std::ofstream;
using std::vector;
#define FILE_LOCATION "projector_config.txt"

//This function gets the next token (or word) and puts it into the word array.  This
//  array must already have space allocated.
//Returns the length of the token
int getToken(char* line,char* word)
{
  int position=0;
  while(line[position]!=0&&line[position]!=' '&&line[position]!='\n'&&line[position]!=26)
  {
    word[position]=line[position];
    //printf("position %i %c\n", position, line[position]);
    position++;
    
  }
  
  //Null terminate the string
  word[position]=0;
  while(line[position]==' '||line[position]=='\n')
    position++;
  return position;

} 

//A struct to hold the display information
struct Dsplay
{
  string host;
  int XServer;
  int XScreen;
  string display_name;
};

class ConfigParser
{

  private:
   

  public:
  vector<Dsplay> displays;
    //This is an example program for reading in from a config file
    ConfigParser(string filename)
    {
  
 
      int line_number;
      std::ifstream config_file(filename.c_str());
      char line[256];
      char word[256];
      std::string hostname;
      int XServer;
      int XScreen;
      int position_counter;
      std::string display_name;
  
    
      while(config_file.getline(line, 256))
      {
        position_counter=0;
        //"#" means that the line is a comment
        if(line[0]!='#')
        {
          Dsplay curr_display;
          assert(line[0]!='\n');
     
          //Parses the hostname
          position_counter+=getToken(&(line[position_counter]), word);
          curr_display.host=string(word);

          //Parses the XServer number
          position_counter+=getToken(&(line[position_counter]), word);
          curr_display.XServer=atoi(word);

          //Parse the XScreen number
          position_counter+=getToken(&(line[position_counter]), word);
          curr_display.XScreen=atoi(word);

          //Parses the display name
          position_counter+=getToken(&(line[position_counter]), word);
          curr_display.display_name=string(word);

          //Adds the current display to the displays vector
          displays.push_back(curr_display);
          
         
        }
      }
    }


    //Writes out the required hostfile for MPI
    // This file must be on each machine
    void write_mpi_hostfile(string filename)
    {
      //Opens the specified file
      ofstream mpi_config_file(filename.c_str());
      int display_count=0;

      //Writes out the hosts.
      string current_string=displays[0].host;
      for(int i=0; i < displays.size(); i++)
      {
        if(current_string.compare(displays[i].host)==0)
          display_count++;
        else
        {
          mpi_config_file<<"slots="<<display_count<<" "<<current_string<<endl;  
          display_count=1;
          current_string=displays[i].host;
        }
      }
      //Writes the last entry of the hostfile
      mpi_config_file<<current_string<<" slots="<<display_count<<endl;  
    
  
    }   
 
};



