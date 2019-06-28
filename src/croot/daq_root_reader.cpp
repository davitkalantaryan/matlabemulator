/*
 *
 */

#include <TFile.h>
#include <TTree.h>
#include "TROOT.h"
#include "TPluginManager.h"
#include <TKey.h>
#include "TLeaf.h"
#include <daq_root_reader.hpp>

#define MAKE_ERROR_THIS(...)
#define MAKE_REPORT_THIS(...)
#define MAKE_WARNING_THIS(...)

#define TMP_FILE_NAME       "tmp.root.file.root"


using namespace pitz::daq;

typedef bool (*TypeContinue)(void* clbkData, const data::memory::ForClient&);

static bool GetDataTypeAndCountStatic(const TBranch* a_pBranch, ::std::list< BranchOutputForUserInfo* >::iterator a_pOutBranchItem);
static int GetMultipleBranchesFromFileStatic( const char* a_rootFileName,
                                              ::std::list< BranchUserInputInfo >::const_iterator a_pInpBranchItem,
                                              ::std::list< BranchOutputForUserInfo* >* a_pOutputIn,
                                              ::std::list< BranchOutputForUserInfo* >* a_pOutputOut,
                                              TypeContinue a_fpContinue, void* a_pClbkData);

namespace pitz{ namespace daq{

//static bool ShallContinue(void* clbkData, const data::memory::ForClient&)
//{
//    //
//}

int GetMultipleBranchesFromFile( const char* a_rootFileName, const ::std::list< BranchUserInputInfo >& a_Input, ::std::list< BranchOutputForUserInfo* >* a_pOutput)
{
    int nReturn;
    ::std::list< BranchUserInputInfo >::const_iterator pInpBranchItem, pInpBranchItemBegin(a_Input.begin()), pInpBranchItemEnd(a_Input.end());
    //::std::list< BranchOutputForUserInfo >::iterator pOutBranchItem;
    ::std::list< BranchOutputForUserInfo* > aOutputIn;
    BranchOutputForUserInfo* pOutData;

    //aOutputIn.resize(a_Input.size());
    for(pInpBranchItem = pInpBranchItemBegin;pInpBranchItem!=pInpBranchItemEnd;++pInpBranchItem){
        //(*pOutBranchItem).userClbk = &(*pInpBranchItem);
        pOutData = new BranchOutputForUserInfo;
        pOutData->userClbk = &(*pInpBranchItem);
        aOutputIn.push_back(pOutData);
    }

    nReturn = GetMultipleBranchesFromFileStatic(a_rootFileName,pInpBranchItemBegin,&aOutputIn,a_pOutput,
                                             [](void*,const data::memory::ForClient&){return true;},nullptr);

    a_pOutput->splice(a_pOutput->end(),aOutputIn,aOutputIn.begin(),aOutputIn.end());
    return nReturn;
}

}} // namespace pitz{ namespace daq{


static int GetMultipleBranchesFromFileStatic(
        const char* a_rootFileName, ::std::list< BranchUserInputInfo >::const_iterator a_pInpBranchItem,
        ::std::list< BranchOutputForUserInfo* >* a_pOutputIn,
        ::std::list< BranchOutputForUserInfo* >* a_pOutputOut,
        TypeContinue a_fpContinue, void* a_pClbkData)
{
    const char* cpcFileName(a_rootFileName);
    TBranch *pBranch;
    TTree* pTree;
    TFile* tFile  ;
    int64_t nIndexEntry;
    int64_t numberOfEntriesInTheFile;
    int nReturn(-1);
    int nDeleteFile = 0;
    //daq::callbackN::retType::Type clbkReturn(daq::callbackN::retType::Continue);
    //data::EntryInfo aBrInfo;
    //const char* cpcDataType;
    //::common::listN::ListItem<TBranchItemPrivate*> *pBranchItemNext, *pBranchItem;
    //memory::Base mem;
    ::std::list< BranchOutputForUserInfo* >::iterator pOutBranchItem, pOutBranchItemEnd, pOutBranchItemTmp;
    data::memory::ForClient aMemory(data::EntryInfoBase(),nullptr);
    data::memory::ForClient* pMemToAdd;

    //if(this->m_clbkType != callbackN::Type::MultiEntries)
    //{
    //    MAKE_ERROR_THIS("Proper callback to retrive multiple entries is not set");
    //    return -1;
    //}
    MAKE_REPORT_THIS(2,"NumberOfBranches = %d",(int)a_pBranches->size());
    //if((int)a_pBranches->count()==0){goto returnPoint;}

    nDeleteFile=0;
    if(strncmp(a_rootFileName,"/acs/",5)==0){
        char vcTmpBuffer[1024];
        cpcFileName = TMP_FILE_NAME;
        snprintf(vcTmpBuffer,1023,"dccp %s " TMP_FILE_NAME, a_rootFileName);
        if(system(vcTmpBuffer)!=0){
            MAKE_ERROR_THIS("Unable to stat the root file %s\n",a_rootFileName);
            return nReturn;
        }
        nDeleteFile = 1;
    }


    tFile = TFile::Open(cpcFileName);

    if(!tFile || !tFile->IsOpen()){
        MAKE_ERROR_THIS("Unable to open root file %s\n",a_rootFileName);
        if(tFile){
#ifdef CALL_DESTRUCTOR
            delete tFile;
#endif
            tFile=nullptr;
        }
        goto returnPoint;
    }

    MAKE_REPORT_THIS(1,"Root file (%s) open success ...!",a_rootFileName);


    pOutBranchItemEnd = a_pOutputIn->end();
    for(pOutBranchItem=a_pOutputIn->begin();pOutBranchItem!=pOutBranchItemEnd;++a_pInpBranchItem,++pOutBranchItem){

        pTree = static_cast<TTree *>(tFile->Get((*a_pInpBranchItem).branchName.c_str()));
        if(!pTree){
            MAKE_REPORT_THIS(2,"Tree \"%s\" is not found", (*a_pInpBranchItem).branchName.c_str());
            goto nextBranchItem;
        }

        pBranch = pTree->GetBranch((*a_pInpBranchItem).branchName.c_str());
        if(!pBranch){
            MAKE_WARNING_THIS("Branch \"%s\" is not found in the tree\n",(*a_pInpBranchItem).branchName.c_str());
            goto nextBranchItem;
        }

        numberOfEntriesInTheFile = pBranch->GetEntries();
        if( (numberOfEntriesInTheFile<1) || (!GetDataTypeAndCountStatic(pBranch,pOutBranchItem)) ){
            goto nextBranchItem;
        }
        aMemory.SetBranchInfo((*pOutBranchItem)->info);

        MAKE_REPORT_THIS(2,"nNumOfEntries[%s] = %d, dataType:%s, itemsCount=%d",
                         (*pInpBranchItem)->branchName.c_str(), (*pOutBranchItem).numberOfEntries,
                         cpcDataType,aBrInfo.itemsCount);

        for(nIndexEntry=0;nIndexEntry<numberOfEntriesInTheFile;++nIndexEntry){
            pBranch->SetAddress(aMemory.rawBuffer());
            pBranch->GetEntry(nIndexEntry);

            if(!a_fpContinue(a_pClbkData,aMemory)){
                pOutBranchItemTmp = pOutBranchItem++;
                a_pOutputOut->splice(a_pOutputOut->end(),*a_pOutputIn,pOutBranchItemTmp);
                break;
            }
            pMemToAdd = new data::memory::ForClient(aMemory,(*pOutBranchItem));
            (*pOutBranchItem)->data.push_back(pMemToAdd);

        }

nextBranchItem:
        ++pOutBranchItem;
    }

    nReturn = 0;
returnPoint:

    if(tFile){
        if(tFile->IsOpen()){tFile->Close();}
#ifdef CALL_DESTRUCTOR
        delete tFile;
#endif
    }

    if(nDeleteFile){
        remove(TMP_FILE_NAME);
    }

    return nReturn;
}


static bool GetDataTypeAndCountStatic(const TBranch* a_pBranch, ::std::list< BranchOutputForUserInfo* >::iterator a_pOutBranchItem)
{
    TLeaf* pLeaf ;
    const char* cpcTypeName="";

    pLeaf = a_pBranch->GetLeaf("data");  // new approach
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("float_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("int_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("array_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("char_array");
    if(pLeaf){cpcTypeName = "string";(*a_pOutBranchItem)->info.dataType = data::type::String;goto finalizeCharArrays;}

    pLeaf = a_pBranch->GetLeaf("IIII_array");
    if(pLeaf){cpcTypeName = "IIII_old";(*a_pOutBranchItem)->info.dataType = data::type::IIII_old;goto finalizeCharArrays;}

    pLeaf = a_pBranch->GetLeaf("IFFF_array");
    if(pLeaf){cpcTypeName = "IFFF_old";(*a_pOutBranchItem)->info.dataType = data::type::IFFF_old;goto finalizeCharArrays;}

    (*a_pOutBranchItem)->info.dataType = data::type::Error;
    (*a_pOutBranchItem)->info.itemsCountPerEntry = 0;
    (*a_pOutBranchItem)->info.dataTypeFromRoot = "unknown";
    return false;

finalizingInfo:
    cpcTypeName = pLeaf->GetTypeName();

    if(strcmp(cpcTypeName,"Float_t")==0){
        (*a_pOutBranchItem)->info.dataType = data::type::Float;
    }
    else if(strcmp(cpcTypeName,"Int_t")==0){
        (*a_pOutBranchItem)->info.dataType = data::type::Int;
    }
    else{
        (*a_pOutBranchItem)->info.dataType = data::type::Error;
    }

    (*a_pOutBranchItem)->info.itemsCountPerEntry =  pLeaf->GetLen();
    (*a_pOutBranchItem)->info.dataTypeFromRoot = cpcTypeName;

finalizeCharArrays:
    (*a_pOutBranchItem)->info.itemsCountPerEntry =  1;
    return true;
}
