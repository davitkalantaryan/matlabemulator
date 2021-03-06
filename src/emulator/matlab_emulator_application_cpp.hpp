//
// file:        matlab_emulator_application.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_APPLICATION_CPP_HPP
#define MATLAB_EMULATOR_APPLICATION_CPP_HPP

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QApplication>
#include <QThread>
#include <stdint.h>
#include <QSettings>
#include <QDebug>
#include <QTimer>
#include <matrix.h>
#include <QMap>
#include <functional>
#include <MatlabEngine.hpp>
#include <cpp11+/common_defination.h>

namespace matlab { namespace emulator {


class CalcThread : public ::QThread
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
public:
    Application(int& argc, char** argv);
    ~Application() OVERRIDE ;

    bool RunCommand( QString& a_command );
    void RunCommand2( const QString&  ){}
    operator ::QSettings& ();

private:
signals:
    void NewLoggingReadySignal(QtMsgType logType, const QString& logMsg);
    void MatlabOutputSignal(const QString& logMsg);
    void MatlabErrorOutputSignal(const QString& logMsg);
    void UpdateSettingsSignal(QSettings& settings);

private:
    static void noMessageOutputStatic(QtMsgType a_type, const QMessageLogContext &,const QString &a_message);
    void noMessageOutput(QtMsgType a_type, const QMessageLogContext &,const QString &a_message);
    void OpenOrReopenMatEngine();
    ssize_t  ReadMatlabErrorPipe(char* buffer, size_t bufferSize);
    mxArray*  GetMultipleBranchesFromFileCls(const QString& argumentsLine);
    mxArray*  GetMultipleBranchesFromFileCls2(const QString& a_argumentsLine);

private:
    CalcThread                  m_calcThread;

    QSettings*                  m_pSettings;
    ::std::unique_ptr< matlab::engine::MATLABEngine > m_pEngineCpp;
    //Engine*                     m_pEngine;
    uint64_t                    m_isEngineVisible : 1;
    uint64_t                    m_bitwise64Reserved : 63;

    QtMessageHandler            m_originalMessageeHandler;
    //char                        m_vcErrorBuffer[1024];
    int                         m_vErrorPipes[2];
    QTimer                      m_settingsUpdateTimer;
    QMap< QString, mxArray* >   m_variablesMap;
    //QMap< QString, void (Application::*)(const QString&) >   m_functionsMap;//QMetaMethod
    QMap< QString, void (*)(Application*,const QString&,const QString&) >   m_functionsMap;//QMetaMethod
    //QMap< QString, QMetaMethod >   m_functionsMap;
    //QMap< QString, void (*)(const QString&) >   m_functionsMap;//QMetaMethod
    //QMap< QString,::std::function< void(Application&, const QString&) > > m_functionsMap;
    //QMap< QString,decltype ([this](){}) > m_functionsMap;
};

}} // namespace matlab { namespace  {

#define ThisAppPtr  static_cast< ::matlab::emulator::Application* >(qApp)
#define ThisApp     (*ThisAppPtr)


#endif   // #ifndef MATLAB_EMULATOR_APPLICATION_CPP_HPP
