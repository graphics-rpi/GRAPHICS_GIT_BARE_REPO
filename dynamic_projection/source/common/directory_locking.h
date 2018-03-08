#ifndef DIR_LOCK_HPP_INCLUDED_
#define DIR_LOCK_HPP_INCLUDED_

#include <cstdio>
#include <sys/file.h>
#include <string>
#include <cassert>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
//#include <iosfwd>
//#include <ext/stdio_filebuf.h>
#ifdef _WIN32
//Tyler's terrible hack for Human Paintbrush
int fileno(FILE *){
	return 0;
}
#endif

using std::string;

//
// simple class for exclusive locking on a directory
//
class DirLock {
public:

  DirLock(const char *dir_name){
    this->dir_name = dir_name;
    std::string filename = dir_name;
    filename += std::string("/lockfile");
    fp = fopen(filename.c_str(), "a");
    if (fp == NULL) {
      std::cout << "ERROR: file cannot be opened: " << filename << std::endl;
      exit(0);
    }
    assert(NULL != fp);
    fd = fileno(fp);
    locked = false;  /* THIS COULD BE A BUG!? */
  }

  DirLock(const string& dir_name)
  {
    string filename = dir_name;
    filename += std::string("/lockfile");
    fp = fopen(filename.c_str(), "a");
    if (fp == NULL) {
      std::cout << "ERROR: file cannot be opened: " << filename << std::endl;
      exit(0);
    }
    assert(NULL != fp);
    fd = fileno(fp);
    locked = false;  /* THIS COULD BE A BUG!? */
  } // end of constructor DirLock

  ~DirLock(){
    Unlock();
    fclose(fp);
  }

  void Lock(){
#if NDEBUG
    flock(fd, LOCK_EX);
#else
    int ret=flock(fd, LOCK_EX);
    assert(0==ret);
#endif
    locked = true;
  }

  bool TryLock(){
    int ret=flock(fd, LOCK_EX | LOCK_NB);
    if (ret == -1 && errno == EWOULDBLOCK) {
      return false;
    } else {
      assert(0==ret);
    }
    locked = true;
    return true;
  }

  void Unlock(){
#if NDEBUG
    flock(fd, LOCK_UN);
#else
    int ret=flock(fd, LOCK_UN);
    assert(0==ret);
#endif
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

