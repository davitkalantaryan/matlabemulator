//
// file:        matlab_emulator_application.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_APPLICATION_C_HPP
#define MATLAB_EMULATOR_APPLICATION_C_HPP

#include <QApplication>
#include <QString>
#include <engine.h>
#include <QThread>
#include <stdint.h>
#include <QSettings>
#include <QDebug>
#include <QTimer>
#include <matrix.h>
#include <QMap>
#include <functional>
#include <cpp11+/common_defination.h>
#include <vector>
#include "../qt_code_editor/codeeditor.hpp"
#include <common/system/runexe.hpp>
#include <string>
#include <exception>
#include <typeinfo>
#include <stddef.h>
#include <map>
#include <matlab/emulator/extend1bylib.hpp>
#include <list>
#include <QFile>
#include <QFileInfoList>
#include <utility>
#include <extendbylib_functions_private.hpp>

#ifdef _WIN32
#if !defined(ssize_t) && !defined(ssize_t_defined)
typedef int ssize_t;
#endif
typedef unsigned int rdtype_t;
#else
typedef size_t rdtype_t;
#endif

namespace matlab { namespace emulator {


//typedef const TEmTableEntry* TypeMatlabEmulatorTable;
struct Extend1LibraryStruct;

struct ExtendedFunction2
{
    FncTypeEmExtension                        function;
    const QString                             functionName;
    const QString                             helpString;
    //
    ExtendedFunction2( FncTypeEmExtension a_function, QString&& a_funcName,QString&& a_helpString ):function(a_function),functionName(a_funcName),helpString(a_helpString){}
};

struct Extend1Function
{
    ExtendedFunction2                           exFunc;
    ::std::list<Extend1Function* >::iterator    thisIter;
    Extend1LibraryStruct*                       parentLibHandle;
    uint64_t                                    isMapped : 1;
    //
    Extend1Function(FncTypeEmExtension a_function, QString&& a_funcName,QString&& a_helpString,Extend1LibraryStruct* a_parentLibHandle)
        :exFunc(a_function,::std::move(a_funcName),::std::move(a_helpString)),parentLibHandle(a_parentLibHandle)
    {this->isMapped = 0;}
    ~Extend1Function();
};

struct Extend2Function
{
    ExtendedFunction2           exFunc;
    void*                       parentLibHandle;
    //
    Extend2Function(FncTypeEmExtension a_function, QString&& a_funcName,QString&& a_helpString,void* a_parentLibHandle)
        :exFunc(a_function,::std::move(a_funcName),::std::move(a_helpString)),parentLibHandle(a_parentLibHandle)
    {}
    ~Extend2Function();
};


struct Extend1LibraryStruct
{
    void*                           libraryHandle;
    uint64_t                        notDestructing : 1;
    const TEmTableEntry*            tableEntry;
    const QString                   libraryPath;
    ::std::list<Extend1Function* >  functions;
    //
    Extend1LibraryStruct(void* a_libHandle,const TEmTableEntry* a_tableEntry,QString&& a_libraryPath)
        :libraryHandle(a_libHandle),tableEntry(a_tableEntry),libraryPath(a_libraryPath)
    {this->notDestructing=1;}
    ~Extend1LibraryStruct();
};


class BadCommandException : public ::std::exception
{
public:
    BadCommandException(const ::std::string& msg);

    const char* what()const noexcept OVERRIDE ;
private:
    const ::std::string     m_what;

};

class WorkerThread : public QThread
{
public:
    WorkerThread();
    ~WorkerThread() OVERRIDE;
private:
    void run() OVERRIDE ;

private:
};


class Application : public QApplication, private ApplicationBase
{
    Q_OBJECT
    static void CommandDefaultFunction(Application*,const QString&,const QString&){}
    typedef void (*TypeCommand)(Application*,const QString&,const QString&);
    struct CommandStruct{
        TypeCommand clbk;
        QString  help;
        CommandStruct():clbk(&CommandDefaultFunction){}
        CommandStruct(const CommandStruct& a_cM):clbk(a_cM.clbk),help(a_cM.help){}
        CommandStruct(const QString& a_help, TypeCommand a_clbk):clbk(a_clbk),help(a_help){}
    };
public:
    Application(int& argc, char** argv);
    ~Application() OVERRIDE ;

    void RunCommand( QString& a_command, bool bThrowException );
    operator ::QSettings& ();

    void MainWindowClosedGui();

    //bool IsCommandRunning()const;

    bool IsUsedAsStdin()const;
    void WriteToExeStdin(const QString& a_str);

private:
    //::common::system::TExecHandle IncreaseUsage();
    //void  DecreaseUsageOfSysHandle( ::common::system::TExecHandle handle );
    bool  IncreaseUsage();
    void  DecreaseUsageOfSysHandle(  );

    void  KeepSystemOutputIntoVarSysThread(const QString& a_systemLine, const QString& a_retArgumetName, int returnsToMask); // 0x1 -> stdout, 0x10 -> stderr, 0x100 -> data

    //
    void ExtendMethod1(const QString& a_inputArgumentsLine,const QString& a_retArgumetName);
    void ClearExtends1();
    void ClearExtends2();
    bool TryToRunExt1Function(const QString& coreCommand,const QString& inputArgumentsLine,const QString& retArgumetName);
    void RunExtendedFunction(const ExtendedFunction2& func, const QString& inputArgumentsLine,const QString& retArgumetName);

    //
    void ShowVariableIfImplemented(const QString& a_inputArgumentsLine);
    void ClearAllVariables();

    void noMessageOutput(QtMsgType a_type, const QMessageLogContext &,const QString &a_message);
    void OpenOrReopenMatEngine();
    ssize_t  ReadMatlabErrorPipe(char* buffer, rdtype_t bufferSize);

#ifdef ROOT_APP
    mxArray*  GetMultipleBranchesFromFileCls(const QString& argumentsLine);
    mxArray*  GetMultipleBranchesForTimeInterval(const QString& a_argumentsLine);
#endif
    bool FindAnyFileInKnownDirs(const QString& inputName,QString* scriptPath);
    int  FindScriptorExt2File(const QString& inputName,QString* scriptPath);

    bool RunScriptOrExt2(const QString& coreCommand,const QString& inputArgumentsLine,const QString& retArgumetName);
    void RunScriptByPath(const QString& a_scriptPath, const QString& a_inputArgumentsLine,const QString& a_retArgumetName);
    void RunScriptByFile(QFile& a_scriptFile, const QString& a_inputArgumentsLine,const QString& a_retArgumetName);
    void RunExt2File(const QString& coreCommand, const QString& ext2Path, const QString& a_inputArgumentsLine,const QString& a_retArgumetName);
    void PrintScriptsAndExtsListForDir(const QFileInfoList& a_entryInfoList);


    /*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
    int MatEmPrintStandard(const char* format, va_list argList) OVERRIDE ;
    int MatEmPrintError(const char* format, va_list argList) OVERRIDE ;
    int MatEmPrintWarning(const char* format, va_list argList) OVERRIDE ;
    int MatEmPrintColored(TMatEmRGB color, const char* format, va_list argList) OVERRIDE ;
    int PutVariableToEmulatorWorkspace(const char* variableName, mxArray* var) OVERRIDE ;
    int PutVariableToMatlabWorkspace(const char* variableName, mxArray* var) OVERRIDE ;
    /*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

private:
signals:
    // command prompt signals
    void InsertOutputSignal(const QString& logMsg);
    void InsertErrorSignal(const QString& logMsg);
    void InsertWarningSignal(const QString& logMsg);
    void InsertColoredTextSignal(int rd, int gr, int bl, const QString& logMsg);
    void AppendNewPromptSignal();
    void PrintCommandsHistSignal();
    // other signals
    void NewLoggingReadySignal(QtMsgType logType, const QString& logMsg);
    void UpdateSettingsSignal(QSettings& settings);
    void ClearPromptSignal();
    // this is not for main thread
    void RunSystemOutSignal(const QString& systemLine, const QString& reurnVarName);
    void RunSystemErrSignal(const QString& systemLine, const QString& reurnVarName);
    void RunSystemBothSignal(const QString& systemLine, const QString& reurnVarName);


    // static functions
private:
    static void noMessageOutputStatic(QtMsgType a_type, const QMessageLogContext &,const QString &a_message);

    // slots (if necessary)
private slots:
    void RunAnyScript(const QString&,const QString&);  // todo: delete this

private:
    //CalcThread                      m_calcThread;

    QSettings*                      m_pSettings;
    Engine*                         m_pEngine;
    uint64_t                        m_isEngineVisible : 1;
    uint64_t                        m_bitwise64Reserved : 63;

    QtMessageHandler                m_originalMessageeHandler;
    //char                          m_vcErrorBuffer[1024];
    int                             m_vErrorPipes[2];
    QTimer                          m_settingsUpdateTimer;
    QMap< QString, mxArray* >       m_variablesMap;
    //QMap< QString, void (Application::*)(const QString&) >   m_functionsMap;//QMetaMethod
    QMap< QString, CommandStruct >  m_functionsMap;//QMetaMethod
    //QMap< QString, QMetaMethod >   m_functionsMap;
    //QMap< QString, void (*)(const QString&) >   m_functionsMap;//QMetaMethod
    //QMap< QString,::std::function< void(Application&, const QString&) > > m_functionsMap;
    //QMap< QString,decltype ([this](){}) > m_functionsMap;
    ::std::vector< QString >        m_knownPaths;
    CodeEditor                      *m_first,*m_last;

    uint64_t                        m_isCommandRunning2 : 4;

    ::common::system::TExecHandle   m_pPrcHandle;
    WorkerThread                    m_workerThread;
    QObject                         m_objectInWorkerThread;
    uint64_t                        m_isAllowedToUse : 1;
    uint64_t                        m_numberOfUsers : 10;

    //
    ::std::map< QString,Extend1LibraryStruct* >   m_librariesExt1;
    ::std::map< QString,Extend1Function* >        m_ext1Functions;
    ::std::map< QString,Extend2Function* >        m_ext2Functions;
    int                                           m_nNumberUnknownExtend1Functions;
};

}} // namespace matlab { namespace  {

#define ThisAppPtr  static_cast< ::matlab::emulator::Application* >(qApp)
#define ThisApp     (*ThisAppPtr)


#endif   // #ifndef MATLAB_EMULATOR_APPLICATION_C_HPP
