#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <vector>
#include <string>
#include <sys/wait.h>
#include <fstream>
#include <iomanip>


#define FULLSCREEN_WIDTH  4096
#define FULLSCREEN_HEIGHT 2160
#define MY_WIDTH 4096
#define MY_HEIGHT 1080
#define MY_LEFT 0
#define MY_BOTTOM 0

#include "socket_helper.h"

std::vector<std::string> all_commands;

void error(const char *msg)
{
  std::cout << "CLIENT ERROR" << std::endl;

    perror(msg);
    exit(0);
}

void setup_commands() {
  all_commands.push_back("");

  // IMPORTANT!  The first string is the directory, the 2nd string is the executable name, the remaining strings are the arguments

  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/synenv/visualizations/MapView/    ./bin/mapview -with_lasers");

  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/synenv/visualizations/MapView/    ./bin/mapview -with_lasers -exec pyscripts/synenv-zero-damage.py");

  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./paint");

  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./puzzle -demo1");
  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./puzzle -demo2");
  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./puzzle -demo3");
  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./puzzle -demo4");

  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./animal_graph -animals_filename ../source/applications/multi_surface_graph/more_animals.txt");

  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./pong");
  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./volume_control");

  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ../../levees/laser_visualization/build/erosion_visualization -i /home/grfx/CDI_CHECKOUT/levees/Source/laser_pointer_interaction/Data/demo1.txt");

  //all_commands.push_back("/home/grfx//CDI_CHECKOUT/levees/Source/laser_pointer_interaction/        ./erosion_visualization -i Data/demo1.txt");

  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./planar_interpolation_calibration -collect_geometry");
  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./planar_interpolation_calibration -collect_intensity");
  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./planar_interpolation_calibration -collect_intensity -clear_all_intensities");
  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./planar_interpolation_calibration -test_data");
  all_commands.push_back("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/build/         ./planar_interpolation_calibration -test_data -visualize_intensity");

  all_commands.push_back("quit");
}




int main(int argc, char *argv[])
{

  setup_commands();

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char input_buffer[MAX_BUFFER_SIZE];
    char send_buffer[MAX_BUFFER_SIZE];
    char receive_buffer[MAX_BUFFER_SIZE];
    if (argc < 2) { //3) {
      //fprintf(stderr,"usage %s hostname port\n", argv[0]);
       fprintf(stderr,"usage %s port\n", argv[0]);
       exit(0);
    }
    //portno = atoi(argv[2]);
    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    //server = gethostbyname(argv[1]);
    server = gethostbyname("localhost"); 
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    std::cout << "connected to server" << std::endl;

    std::vector<std::vector<std::string> > command_list;
    for (unsigned int i = 0; i < all_commands.size(); i++) {
      command_list.push_back(split_command_into_vector(all_commands[i]));
      if (i != 0) 
	command_list[i][0] = remove_path(command_list[i][0]);
    }

    while (1) {

      for (unsigned int i = 1; i < command_list.size(); i++) {
	std::cout << "[" << std::setw(2) << i << "] ";
	for (unsigned int j = 0; j < command_list[i].size(); j++) {
	  std::cout << " " << command_list[i][j];
	}
	std::cout << std::endl;
      }

      printf("Enter the command number: ");

      bzero(input_buffer,MAX_BUFFER_SIZE);
      
      char *ret_val = fgets(input_buffer,MAX_BUFFER_SIZE-1,stdin);
      assert (ret_val != NULL);


      //if (!commandify(buffer)) break;
      
      int command_number = atoi(input_buffer);
      std::cout << " command number " << command_number << std::endl;
      if (command_number < 1 || command_number >= (int)all_commands.size()) {
	std::cout << "bad command" << std::endl;
	continue;
      }

      // clean out mouse & keyboard state directory before we start
      //system ("rm ../state/mouse_and_keyboard/*txt");
      int success = system ("rm ../state/applications/*state.txt");
      assert (success==0);

      strcpy(send_buffer,all_commands[command_number].c_str());
      n = write(sockfd,send_buffer,strlen(send_buffer));

      //std::string my_command = all_commands[command_number];

      if (n < 0) 
	error("ERROR writing to socket");
      bzero(receive_buffer,MAX_BUFFER_SIZE);
      n = read(sockfd,receive_buffer,MAX_BUFFER_SIZE-1);
      if (n < 0) 
	error("ERROR reading from socket");
      printf("RETURN MESSAGE %s\n",receive_buffer);

      std::string return_message = receive_buffer;
      int pos = return_message.find_last_of(' ');
      assert (pos > 0 && pos < (int)return_message.size());
      std::string server_child_pid = return_message.substr(pos+1,return_message.size()-pos-1);
      
      pid_t server_childPID = atoi(server_child_pid.c_str());

      //std::cout << "server_childPID = " << server_childPID << std::endl;

      // launch the client display
      pid_t client_childPID = commandify(send_buffer, true);
      
      //std::cout << "client_childPID = " << client_childPID << std::endl;



      //pid_t w;
      //int status;
      //w = waitpid(client_childPID, & status, WNOHANG | WEXITED);

      std::string myfile = "temporary_file_for_process_killing.txt";

      std::stringstream ss1; 
      std::stringstream ss2; 
      std::stringstream ss3; 
      ss1 << "ps -p " << server_childPID << " | grep defunct | wc > " << myfile;
      ss2 << "ps -p " << client_childPID << " | grep defunct | wc > " << myfile;
      
      ss3 << "rm " << myfile;
      

      //      int count = 1000;
      
      //while (count > 0) {
      //count--;
      while (1) {
	
	/*	std::cout << "IN CHECKING" << std::endl;
	int cstat = kill(client_childPID,0);
	//if (errno == EINVAL) { std::cout << "c1" << std::endl; break; }
	if (errno == EPERM)  { std::cout << "c2" << std::endl; break; }
	if (errno == ESRCH)  { std::cout << "c3" << std::endl; break; }
	int sstat = kill(server_childPID,0);
	//if (errno == EINVAL) { std::cout << "c4" << std::endl; break; }
	if (errno == EPERM)  { std::cout << "c5" << std::endl; break; }
	if (errno == ESRCH)  { std::cout << "c6" << std::endl; break; }

	std::cout << "running" << std::endl;
	//	std::cout << cstat << " " << sstat << std::endl;
	//if (cstat != sstat) break;
	*/

      //std::cout << "loop" << std::endl;

	int lines, success;

	//std::cout << "server_childPID = " << server_childPID << std::endl;
	//std::cout << "client_childPID = " << client_childPID << std::endl;
	//std::cout << ss1.str() << std::endl;
	//std::cout << ss2.str() << std::endl;




	success = system (ss1.str().c_str());
      assert (success==0);
	std::ifstream istr1(myfile.c_str()); 
	istr1 >> lines;
	istr1.close();
	success = system(ss3.str().c_str());
      assert (success==0);
	//std::cout << "LINESA = " << lines << std::endl;
	if (lines == 1) {
	  break;
	}
	istr1.close();


	success = system (ss2.str().c_str());
      assert (success==0);
	std::ifstream istr2(myfile.c_str()); 
	istr2 >> lines;
	istr2.close();
	success = system(ss3.str().c_str());
      assert (success==0);
	//std::cout << "LINESB = " << lines << std::endl;
	if (lines == 1) {
	  break;
	}
	istr2.close();

      }
      
      std::stringstream ss; 
      ss << "kill -9 " << server_childPID;
      success = system (ss.str().c_str());
      assert (success==0);

      ss.str("");
      ss << "kill -9 " << client_childPID;
      success = system (ss.str().c_str());
      assert (success==0);

      usleep(100);

    }

    close(sockfd);
    return 0;
}
