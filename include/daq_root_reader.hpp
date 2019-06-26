
#ifndef PITZ_DAQ_DAQ_ROOT_READER_HPP
#define PITZ_DAQ_DAQ_ROOT_READER_HPP

#include <list>
#include <string>
#include <stdint.h>

#define INPUT_PD
#define OUTPUT_PD

namespace pitz{ namespace daq{ namespace data{

namespace type{enum Type{Error=-1,NoData=0,Int=1,Float=2,String=3,IIII_old=4,IFFF_old=5};}

}}}

namespace pitz{ namespace daq{

struct BranchUserInputInfo{
    ::std::string   branchName      INPUT_PD ;
};

struct BranchOutputForUserInfo{
    const BranchUserInputInfo*  userClbk;
    const char*                 dataTypeFromRoot;
    int64_t                     numberOfEntries OUTPUT_PD ;
    int                         itemsCountPerEntry;
    data::type::Type            dataType;
};

}}


#endif  // #ifndef PITZ_DAQ_DAQ_ROOT_READER_HPP
