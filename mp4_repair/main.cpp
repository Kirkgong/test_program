#include "Mp4Repair.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv){

    if(argc < 2){
        cout << "Please input file name." << endl; 
        exit(0);
    }

    if(access(argv[1], F_OK) == -1){
        cout << "File " << argv[1] << " is not exist." << endl;
        exit(0);
    }

    Mp4Repair repair;
    repair.open(argv[1]);
    FILE_STATUS status = repair.check(); 

    if(status == FILE_STATUR_NORMAL){
        cout << "File status is normal.Don't need repair." << endl;
        repair.close();
        exit(0);
    }else if(status == FILE_STATUR_DAMAGE){
        cout << "File status is undefine.Can't repair." << endl;
        repair.close();
        exit(0);
    }

    repair.close();
}