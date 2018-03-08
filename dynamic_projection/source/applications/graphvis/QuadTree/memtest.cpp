/*
 * memtest.cpp
 *
 *  Created on: Jul 18, 2011
 *      Author: phipps
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "QuadTree.h"


int main()
{
	srand ( time(NULL) );

	int *o = new int(5);
	QuadTree<int> k = QuadTree<int>();
	k.Add(o, 20, 20);
	k.Add(o, 21, 25);
	k.Add(o, 50, 12);
	k.Add(o, 60, 54);
	k.Add(o, 23, 34);
	k.Add(o, 52, 23);

	for(int i = 0; i < 5000000; ++i)
	{
		k.Add(o, (rand() % 1920 + 1), (rand() % 1200 + 1));
	}

	k.Clear();
	delete o;

}
