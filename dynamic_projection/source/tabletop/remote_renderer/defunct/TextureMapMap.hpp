#ifndef TEXTURE_MAP_MAP_INCLUDED_
#define TEXTURE_MAP_MAP_INCLUDED_
#include <unordered_map>
#include <vector>
#include <string>
#include <stdio.h>

#include "SocketReader.h"

//char montage_string[256]="/home/grfx/Checkout/JOSH_EMPAC_2010/puzzle_textures/gary_hudson_montage/hudson_river_montage_";

class TextureMapMap {
public:
  TextureMapMap(){}
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
 //        printf("Material map size is %i \n", material_map.size() );
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
    std::unordered_map<std::string, int>::iterator p = material_map.find(material_name);
    if (p != material_map.end()){
      return p->second;
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
    for(std::unordered_map<std::string, int>::const_iterator it=material_map.begin();
	it!=material_map.end(); ++it) {      
      printf("blah material name:%s, texture id:%d\n", it->first.c_str(), it->second);
    }
  }
private:
  std::vector<std::string> texture_filenames;
  std::unordered_map<std::string, int> material_map;
};

#endif // #ifndef TEXTURE_MAP_MAP_INCLUDED_
