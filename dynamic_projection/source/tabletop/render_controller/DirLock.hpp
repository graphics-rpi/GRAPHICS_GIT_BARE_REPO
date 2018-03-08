#ifndef DIR_LOCK_HPP_INCLUDED_
#define DIR_LOCK_HPP_INCLUDED_

#include <cstdio>
#include <sys/file.h>
#include <string>
#include <cassert>

//
// simple class for exclusive locking on a directory
//
class DirLock {
public:

  DirLock(const char *dir_name){
    this->dir_name = dir_name;
    std::string filename = dir_name;
    //    filename += std::string("/lockfile");
    // THIS CHANGED FOR THE STANDALONE
    //filename = std::string("lockfile");
    // -------
    // this is what combined wants:
    printf("opening %s \r\n",dir_name);
    filename += std::string("/lockfile");
    // ------
    fp = fopen(filename.c_str(), "a");
    printf("opening %s \r\n",filename.c_str());
    fflush(stdout);
    assert(NULL != fp);
    fflush(stdout);
    fd = fileno(fp);
    locked = false;
  }

  ~DirLock(){
    Unlock();
    fclose(fp);
  }

  void Lock(){
    int ret=flock(fd, LOCK_EX);
    assert(0==ret);
    locked = true;
  }

  void Unlock(){
    int ret=flock(fd, LOCK_UN);
    assert(0==ret);
    locked = false;
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

