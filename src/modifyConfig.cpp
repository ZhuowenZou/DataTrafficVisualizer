#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstddef>
#include <sstream>
using namespace std;

int main(int argc, char **argv)
{
	if (argc != 4){	
		cerr << "Please enter exactly three arguments for header, tick and size prefixes"<<endl;
		cerr << "You entered "<< argc-1 << " arguments" <<endl;
		return EXIT_FAILURE;
	}
	ofstream output;
	output.open("config");
	output<<"L="<<argv[1]<<endl;
	output<<"x="<<argv[2]<<endl;
	output<<"y="<<argv[3]<<endl;
	output.close();
	cout << "Configuration updated." <<endl;
	return EXIT_SUCCESS;
}
