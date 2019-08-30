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
#include <common/system.hpp>
#include <string>

namespace matlab { namespace emulator {


class CalcThread : public QThread
{
public:
    CalcThread();
    ~CalcThread() OVERRIDE;
private:
    void run() OVERRIDE ;

private:
};



class Application : public QApplication
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

    bool RunCommand( QString& a_command );
    void RunCommand2( const QString&  ){}
    operator ::QSettings& ();

    void MainWindowClosedGui();

    bool IsSystemRunning()const;
    bool IsUsedAsStdin()const;
    void WriteToExeStdin(const QString& a_str);

private:
    //::common::system::TExecHandle IncreaseUsage();
    //void  DecreaseUsageOfSysHandle( ::common::system::TExecHandle handle );
    bool  IncreaseUsage();
    void  DecreaseUsageOfSysHandle(  );

private:
signals:
    void NewLoggingReadySignal(QtMsgType logType, const QString& logMsg);
    void MatlabOutputSignal2(const QString& logMsg);
    void ExeOutputSignal2(const QString& logMsg);
    void MatlabErrorOutputSignal2(const QString& logMsg);
    void ExeErrorOutputSignal2(const QString& logMsg);
    void UpdateSettingsSignal(QSettings& settings);

    void RunSystemSignal(const QString& systemLine);


private:
    static void noMessageOutputStatic(QtMsgType a_type, const QMessageLogContext &,const QString &a_message);
    void noMessageOutput(QtMsgType a_type, const QMessageLogContext &,const QString &a_message);
    void OpenOrReopenMatEngine();
    ssize_t  ReadMatlabErrorPipe(char* buffer, size_t bufferSize);
    mxArray*  GetMultipleBranchesFromFileCls(const QString& argumentsLine);
    mxArray*  GetMultipleBranchesForTimeInterval(const QString& a_argumentsLine);
    bool FindScriptFile(const QString& inputName,QString* scriptPath);
    void    HandleLastStoredData(::common::system::readCode::Type type, const char* buffer, size_t size);

private slots:
    void RunScript(const QString&,const QString&);

private:
    CalcThread                      m_calcThread;

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

    ::common::system::TExecHandle   m_pPrcHandle;
    QThread                         m_workerThread;
    QObject                         m_objectInWorkerThread;
    uint64_t                        m_isAllowedToUse : 1;
    uint64_t                        m_isSystemRunning : 1;
    uint64_t                        m_numberOfUsers : 10;
};

}} // namespace matlab { namespace  {

#define ThisAppPtr  static_cast< ::matlab::emulator::Application* >(qApp)
#define ThisApp     (*ThisAppPtr)


#endif   // #ifndef MATLAB_EMULATOR_APPLICATION_C_HPP
