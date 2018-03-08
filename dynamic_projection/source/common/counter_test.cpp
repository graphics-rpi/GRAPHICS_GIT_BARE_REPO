#include "counter.h"
#include <iostream>
  using namespace std;

int main()
{
  Counter counter("/home/nasmaj/test");
  while(1){
    counter.waitForNextCounter();
    cout<<"Current counter value "<<(int)counter.getCounterValue()<<endl;
  }
    
  
}
