#ifndef DIR_LOCK_HPP_INCLUDED_
#define DIR_LOCK_HPP_INCLUDED_

#include <cstdio>
#ifndef _WIN32
#include <sys/file.h>
#endif
#include <string>
#include <cassert>

//
// simple class for exclusive locking on a directory
//
class DirLock {
public:

  DirLock(const char *dir_name){
#ifdef _WIN32
	//Directory locking not currently set up for windows
	assert(0);
#else
    this->dir_name = dir_name;
    std::string filename = dir_name;
    filename += std::string("/lockfile");
    fp = fopen(filename.c_str(), "a");
    assert(NULL != fp);
    fd = fileno(fp);
    locked = false;
#endif
  }

  ~DirLock(){
    Unlock();
    fclose(fp);
  }

  void Lock(){
#ifndef _WIN32
    int ret=flock(fd, LOCK_EX);
    assert(0==ret);
    locked = true;
#endif
  }

  void Unlock(){
#ifndef _WIN32
    int ret=flock(fd, LOCK_UN);
    assert(0==ret);
    locked = false;
#endif
  }

  bool IsLocked(){
    return locked;
  }

private:
  DirLock(const DirLock &);
  DirLock& operator=(const DirLock &);

  std::string dir_name;
  FILE *fp;
  int fd;
  bool locked;
};

#endif // #ifndef DIR_LOCK_HPP_INCLUDED_

