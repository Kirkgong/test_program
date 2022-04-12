#include "Mp4Repair.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv){
    Mp4Repair repair;

    if(argc < 2){
        cout << "Please input file name." << endl; 
        exit(0);
    }

    if(access(argv[1], F_OK) == -1){
        cout << "File " << argv[1] << " is not exist." << endl;
        exit(0);
    }

    FILE_STATUS status = repair.process(argv[1]); 
}