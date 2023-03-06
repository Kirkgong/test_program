#include "PraseTsFile.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char **argv)
{
    PraseTsFile prase_ts_file;
    if(argc < 2)
    {
        printf("Parameter error\n");
        return 1;
    }

    prase_ts_file.TsFileRead(argv[1], 3*30*3000, 6*30*3000);
}