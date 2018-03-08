/*
This provides the functionality for having a counter file (in addition to a lockfile)
A counter file is a counter.txt file in the specified directory.  After using the 
constructor with a directory.  You can increment the timer using incrementCounter()
If you want to have the program block until the counter changes value 
use waitForNextCounter()
*/


#include "directory_locking.h"

//Utility functions necessary for counter
bool file_exists(const char *filename)
{
  FILE *fp = fopen(filename, "rt");
  if (NULL == fp)
  {
    return false;
  }
 
  fclose(fp);
  return true;
}

int read_int_from_file(const char * filename)
{
  int retval;
  if (FILE * file = fopen(filename, "r"))
  {
    fscanf(file,"%d", &retval);
    //fread(&retval, sizeof(int), 1, file);
    fclose(file);
    return retval;
  }
  else 
  {
    printf("cannot open %s for reading\n",filename);
    assert(0);
  }
}

void write_int_to_file(const char * filename, int num)
{
  int retval;
  if (FILE * file = fopen(filename, "w"))
  {
    fprintf(file, "%d", num);
    fclose(file);
  }
  else 
  {
    printf("Failed to open %s for writing int\n", filename);
    assert(0);
  }
}

class Counter
{
  public:
    //The counter constructor only takes the directory where counter.txt will reside
    Counter(string directory)
    {
      filename=directory+"/counter.txt";
      assert(file_exists(filename.c_str())&&"COUNTER FILE DOESN'T EXISTS");
      //A directory lock is used to ensure we don't read while the counter is being written
      lock=new DirLock(directory);   
      lock->Lock();
      counterNum=read_int_from_file(filename.c_str());
      lock->Unlock();
    }
    
    //Doesn't terminate until the counter changes
    void waitForNextCounter()
    {
      lock->Lock();
      int currVal=read_int_from_file(filename.c_str());
      lock->Unlock();
      while(currVal==counterNum)
      {
        usleep(100000);
        lock->Lock();
        currVal=read_int_from_file(filename.c_str());
        lock->Unlock();
      }
      counterNum=currVal;
    }
    
    //Increments the counter (assumes no one else is modifying counter)
    void incrementCounter()
    {
      counterNum++;
      lock->Lock();
      write_int_to_file(filename.c_str(),counterNum);
      lock->Unlock();
    }
    
    //Gets the current value (doesn't repoll file, just gives you last value
    int getCounterValue()
    {
      return counterNum;
    }
    
    private:
     string filename;
     DirLock* lock;
     int counterNum;
};
