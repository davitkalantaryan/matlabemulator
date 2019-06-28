
#ifndef PITZ_DAQ_DAQ_ROOT_READER_HPP
#define PITZ_DAQ_DAQ_ROOT_READER_HPP

#include <list>
#include <string>
#include <stdint.h>
#include <pitz/daq/data/memory/forclient.hpp>

#define INPUT_PD
#define OUTPUT_PD



namespace pitz{ namespace daq{

struct BranchUserInputInfo{
    BranchUserInputInfo(const ::std::string& a_branchName):branchName(a_branchName){}
    ::std::string   branchName      INPUT_PD ;
};

struct BranchOutputForUserInfo{
    const BranchUserInputInfo*                  userClbk;
    data::EntryInfoBase                         info;
    ::std::list< data::memory::ForClient* >     data;
};

int Initialize();
void Cleanup();
int GetMultipleBranchesFromFile( const char* a_rootFileName, const ::std::list< BranchUserInputInfo >& a_pInput, ::std::list< BranchOutputForUserInfo* >* a_pOutput);

}}



#endif  // #ifndef PITZ_DAQ_DAQ_ROOT_READER_HPP
