#include <iostream>
#include "Timer.h"

using namespace std;

int
main(int n, char **argv){
	miutil::Timer t(3);
	
	t.mark("1timer");
	t.mark("2timer");
	t.mark("3timer");
	t.mark("4timer");
	t.mark("5timer");
	cerr << t;
	
}