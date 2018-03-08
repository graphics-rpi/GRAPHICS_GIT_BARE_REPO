#include "counter.h"
#include <iostream>
  using namespace std;

int main()
{
  Counter counter("/home/nasmaj/test");
  while(1)
  {
    char a;
    cin>>a;
    counter.incrementCounter();
  }
  
}
