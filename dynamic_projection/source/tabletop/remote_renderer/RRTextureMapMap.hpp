#ifndef RRTEXTURE_MAP_MAP_INCLUDED_
#define RRTEXTURE_MAP_MAP_INCLUDED_
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdio>
const char montage_string[256]="/home/grfx/Checkout/JOSH_EMPAC_2010/puzzle_textures/gary_hudson_montage/hudson_river_montage_";

class RRTextureMapMap {
public:
  RRTextureMapMap(){}
  void loadFile(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "unable to open %s\n", filename);
      exit(-1);
    }
    load(fp);
    fclose(fp);
  }
  char *walls[5];

  void load(SocketReader & sr){
    while(!sr.eof()){
      char textline[1024];
      char *material_name;
      char *texture_file;
      sr.Jgets(textline, 1024);
      if (!strcmp(textline, "END TEXTURE_MAP_MAP\n")){
        break;
      } else if (material_name = strtok(textline, "|")){
        if (texture_file = strtok(NULL, " \t\n")){
          texture_filenames.push_back(texture_file);
          material_map[material_name] = texture_filenames.size()-1;
        }
      } 
    }
  }
  

  void load_puzzle(SocketReader & sr, std::string* sides, bool* texture_changed){

    //These are the several textures needed for the puzzle
    walls[0]=(char*)"WALL0_0";
    walls[1]=(char*)"WALL1_0";
    walls[2]=(char*)"WALL2_0";
    walls[3]=(char*)"WALL3_0";
    walls[4]=(char*)"WALL4_0";

    int i=0;
    while(!sr.eof()){

      char textline[1024];
      sr.Jgets(textline, 1024);
      if (!strcmp(textline, "END PUZZLE_FILE\n")){
        break;
      }
      char puzzle_string[256];
      char full_filename[256];
      int val;

      assert (1 == sscanf(textline, "%1024s ", puzzle_string));
      //printf("Got %s, sides name:%s \n",puzzle_string, sides[i].c_str());
      if(strcmp(puzzle_string, sides[i].c_str()) ){
	      sides[i] = puzzle_string;
	      printf("sides[i]=%s\n", sides[i].c_str());
	      sprintf(full_filename,"%s%s.ppm",montage_string,puzzle_string);
	      //sprintf(full_filename,"/home/grfx/red.ppm");
	

	      val=getTextureID(walls[i]);
	
	      if(val==-1){
	        addTexture(walls[i],full_filename);
          printf("%s added\n",full_filename);
	      }
	      texture_changed[i] = true;
      } else {
	      texture_changed[i] = false;
      }
      i++;
    }
    
  }








     
                                                                                    
    
  

  void load(FILE *fp){
    while(!feof(fp)){
      char textline[1024];
      char *material_name;
      char *texture_file;
      fgets(textline, 1024, fp);
      if (!strcmp(textline, "END TEXTURE_MAP_MAP\n")){
        break;
      } else if (material_name = strtok(textline, "|")){
        if (texture_file = strtok(NULL, " \t\n")){
          texture_filenames.push_back(texture_file);
          material_map[material_name] = texture_filenames.size()-1;
        }
      } 
    }
  }

  void addTexture(char* tex_name, char* file_name)
  {
//      texture_filenames.push_back(file_name);
  //    material_map[tex_name] = texture_filenames.size()-1;
    //   printf("adding texture %s filename %s size %i \n", tex_name, file_name, texture_filenames.size());
       int index=0;
       for( ; index<texture_filenames.size();index++)
          if(!strcmp(texture_filenames[index].c_str(),file_name))
            break;
       if(index==texture_filenames.size()){
         texture_filenames.push_back(file_name);
         material_map[tex_name] = texture_filenames.size()-1;
         //printf("Material map %s is %i \r\n", tex_name, texture_filenames.size()-1 );
       }

//for(std::unordered_map<std::string, int>::const_iterator it=material_map.begin();	it!=material_map.end(); ++it) { 
//  lescout << it -> first << " ";
//}

  }

  int getNumTextures(){
    return texture_filenames.size();
  }
  std::string getTextureFilename(int idx){
    return texture_filenames.at(idx);
  }
  int getTextureID(std::string material_name){
    if (material_map.find(material_name) != material_map.end()){
      return material_map[material_name];
    } else {
      return -1;
    }
  }

  void print()
  {
//    printf("all the texture filename:\n");
 //   for(std::size_t i=0; i<texture_filenames.size(); ++i) {
 //     printf("texture %d: %s\n", i, texture_filenames[i].c_str());
  //  }
  //  printf("all the texture filename:\n");

    if(texture_filenames.empty())
      printf("texture map map empty\r\n");
    else
      for(std::unordered_map<std::string, int>::const_iterator it=material_map.begin();
	it!=material_map.end(); ++it) {      
        //printf("blah material name:%s, texture id:%d\n", it->first.c_str(), it->second);
      }
      for(int i=0; i < texture_filenames.size(); i++)
        printf("texture name: %s %d \r\n", texture_filenames.at(i).c_str(), i);
  }
private:
  std::vector<std::string> texture_filenames;
  std::unordered_map<std::string, int> material_map;
};

#endif // #ifndef TEXTURE_MAP_MAP_INCLUDED_
