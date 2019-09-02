//
// file:        main_prog_with_stdin_test.cpp
// created on:  2019 Aug 30
//

#include <iostream>
#include <stdio.h>
#include <string>

struct mxArray{};

//mexPrintf

void WriteToEmulatorMemory(mxArray*);

int main(int a_argc, char* a_argv[])
{
    ::std::string stringFromUser;
    ::std::cout << "argc="<< a_argc << ::std::endl;

    for(int i(0); i<a_argc;++i){
        ::std::cout << "\targv[" << i << "]=\"" << a_argv[i] << "\"\n";
    }

    ::std::cout << ::std::endl;
    fflush(stdout);

    ::std::cerr << "Some test error output";
    fflush(stderr);

    ::std::cout << "Print string to see the echo. String: "; fflush(stdout);
    ::std::cin >> stringFromUser;
    ::std::cout << ::std::endl << "UserString=\"" << stringFromUser << "\"\n";

    return 0;
}
