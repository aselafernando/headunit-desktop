#include <math.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <cstring>
#include <cmath>

#include "j2534plugin.h"

_PassThruConnect PassThruConnect = NULL;
_PassThruDisconnect PassThruDisconnect = NULL;
_PassThruReadMsgs PassThruReadMsgs = NULL;
_PassThruWriteMsgs PassThruWriteMsgs = NULL;
_PassThruStartPeriodicMsg PassThruStartPeriodicMsg = NULL;
_PassThruStopPeriodicMsg PassThruStopPeriodicMsg = NULL;
_PassThruStartMsgFilter PassThruStartMsgFilter = NULL;
_PassThruStopMsgFilter PassThruStopMsgFilter = NULL;
_PassThruSetProgrammingVoltage PassThruSetProgrammingVoltage = NULL;
_PassThruReadVersion PassThruReadVersion = NULL;
_PassThruGetLastError PassThruGetLastError = NULL;
_PassThruIoctl PassThruIoctl = NULL;
_PassThruOpen PassThruOpen = NULL;
_PassThruClose PassThruClose = NULL;
_PassThruReset PassThruReset = NULL;
_PassThruGetLastSocketError PassThruGetLastSocketError = NULL;
void* handle = NULL;

J2534Plugin::J2534Plugin(QObject* parent) : QObject(parent) {
    //m_pluginSettings.eventListeners = QStringList() << "MediaInput::position";
    //m_text = m_settings.value("text").toString() == "true";
}

void J2534Plugin::init() {
    handle = dlopen("/opt/hud/libJ2534.so", RTLD_LAZY);
    char* error = NULL;

    this->m_reversePageIndex = m_settings.value("reverse_page_index").toUInt();

    if (!handle) {
        qDebug() << "J2534: Cannot load libJ2534.so " << dlerror();
    } else {
        //clear existing errors
        dlerror();
        //map functions
        PassThruConnect = (_PassThruConnect)dlsym(handle, "PassThruConnect");
        if((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruConnect " << error;
        }
        PassThruDisconnect = (_PassThruDisconnect)dlsym(handle, "PassThruDisconnect");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruDisconnect " << error;
        }
        PassThruReadMsgs = (_PassThruReadMsgs)dlsym(handle, "PassThruReadMsgs");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruReadMsgs " << error;
        }
        PassThruWriteMsgs = (_PassThruWriteMsgs)dlsym(handle, "PassThruWriteMsgs");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruWriteMsgs " << error;
        }
        PassThruStartPeriodicMsg = (_PassThruStartPeriodicMsg)dlsym(handle, "PassThruStartPeriodicMsg");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruStartPeriodicMsg " << error;
        }
        PassThruStopPeriodicMsg = (_PassThruStopPeriodicMsg)dlsym(handle, "PassThruStopPeriodicMsg");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruStopPeriodicMsg " << error;
        }
        PassThruStartMsgFilter = (_PassThruStartMsgFilter)dlsym(handle, "PassThruStartMsgFilter");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruStartMsgFilter " << error;
        }
        PassThruStopMsgFilter = (_PassThruStopMsgFilter)dlsym(handle, "PassThruStopMsgFilter");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruStopMsgFilter " << error;
        }
        PassThruSetProgrammingVoltage = (_PassThruSetProgrammingVoltage)dlsym(handle, "PassThruSetProgrammingVoltage");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruSetProgrammingVoltage " << error;
        }
        PassThruReadVersion = (_PassThruReadVersion)dlsym(handle, "PassThruReadVersion");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruReadVersion " << error;
        }
        PassThruGetLastError = (_PassThruGetLastError)dlsym(handle, "PassThruGetLastError");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruGetLastError " << error;
        }
        PassThruIoctl = (_PassThruIoctl)dlsym(handle, "PassThruIoctl");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruIoctl " << error;
        }
        PassThruOpen = (_PassThruOpen)dlsym(handle, "PassThruOpen");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruOpen " << error;
        }
        PassThruClose = (_PassThruClose)dlsym(handle, "PassThruClose");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruClose " << error;
        }
        PassThruReset = (_PassThruReset)dlsym(handle, "PassThruReset");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruReset " << error;
        }
        PassThruGetLastSocketError = (_PassThruGetLastSocketError)dlsym(handle, "PassThruGetLastSocketError");
        if ((error = dlerror()) != NULL) {
            qDebug() << "J2534: PassThruGetLastSocketError " << error;
        }
        qDebug() << "J2534: Loaded functions";

        startWorker();
    }
}

void J2534Plugin::startWorker() {
    if(handle) {
        //Run the data colleciton in another thread
        J2534Worker* worker = new J2534Worker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &J2534Plugin::operate, worker, &J2534Worker::j2534Connect);
        connect(worker, &J2534Worker::iat, this, &J2534Plugin::handleIAT);
        connect(worker, &J2534Worker::maf, this, &J2534Plugin::handleMAF);
        connect(worker, &J2534Worker::ect, this, &J2534Plugin::handleECT);
        connect(worker, &J2534Worker::vss, this, &J2534Plugin::handleVSS);
        connect(worker, &J2534Worker::rpm, this, &J2534Plugin::handleRPM);
        connect(worker, &J2534Worker::accPedal, this, &J2534Plugin::handleACCPEDAL);
        connect(worker, &J2534Worker::gear, this, &J2534Plugin::handleGEAR);
        connect(worker, &J2534Worker::stftb1, this, &J2534Plugin::handleSTFTB1);
        connect(worker, &J2534Worker::stftb2, this, &J2534Plugin::handleSTFTB2);
        workerThread.start();
        operate();
        qDebug() << "J2534: Started worker thread: " << workerThread.isRunning();
    }
}

void J2534Plugin::stopWorker() {
    workerThread.quit();
    workerThread.requestInterruption();
    workerThread.wait();
}

J2534Plugin::~J2534Plugin() {
    if (handle) {
        stopWorker();
        dlclose(handle);
        handle = NULL;
    }
}

QObject *J2534Plugin::getContextProperty(){
    return this;
}


void J2534Plugin::handleIAT(const double& result) {
    if(this->m_iat != result) {
        this->m_iat = result;
        emit iatUpdated();
        emit message("IAT", result);
    }
}

void J2534Plugin::handleMAF(const double& result) {
    if(this->m_maf != result) {
        this->m_maf = result;
        emit mafUpdated();
        emit message("MAF", result);
    }
}

void J2534Plugin::handleSTFTB1(const double& result) {
    if(this->m_stftb1 != result) {
        this->m_stftb1 = result;
        emit stftb1Updated();
        emit message("STFTB1", result);
    }
}

void J2534Plugin::handleSTFTB2(const double& result) {
    if(this->m_stftb2 != result) {
        this->m_stftb2 = result;
        emit stftb2Updated();
        emit message("STFTB2", result);
    }
}

void J2534Plugin::handleECT(const double& result) {
    if(this->m_ect != result) {
        this->m_ect = result;
        emit ectUpdated();
        emit message("ECT", result);
    }
}

void J2534Plugin::handleVSS(const int& result) {
    if(this->m_vss != result) {
        this->m_vss = result;
        emit vssUpdated();
        emit message("VSS", result);
    }
}

void J2534Plugin::handleRPM(const double& result) {
    if(this->m_rpm != result) {
        this->m_rpm = result;
        emit rpmUpdated();
        emit message("RPM", result);
    }
}

void J2534Plugin::handleACCPEDAL(const double& result) {
    if(this->m_accPedal != result) {
        this->m_accPedal = result;
        emit accPedalUpdated();
        emit message("TPS", result);
    }
}

void J2534Plugin::handleGEAR(const int& result) {
    if(this->m_gear != result) {
        //Change to the reverse camera page, -2 disables this
        if(result == 240) {
            if(this->m_reversePageIndex > -2)
                emit action("GUI::ChangePageIndex", this->m_reversePageIndex);
        } else if(this->m_gear == 240) {
            if(this->m_reversePageIndex > -2)
                emit action("GUI::ChangePagePrevIndex", this->m_reversePageIndex);
        }
        this->m_gear = result;
        emit gearUpdated();
        emit message("Gear", result);
    }
}

void J2534Plugin::eventMessage(QString id, QVariant message) {
}

void J2534Plugin::settingsChanged(const QString &key, const QVariant &){
    if(key == "reverse_page_index") {
        this->m_reversePageIndex = m_settings.value("reverse_page_index").toUInt();
    }
}

void J2534Plugin::PrintString(char *message) {
    qDebug() << "J2534 DEBUG : " << message;
}

bool J2534Worker::startMsgFilters() {
    PASSTHRU_MSG maskMsg;
    PASSTHRU_MSG patternMsg;
    uint32_t tmpMsgID = 0;

    memset(&maskMsg, 0, sizeof(PASSTHRU_MSG));
    memset(&patternMsg, 0, sizeof(PASSTHRU_MSG));

    //Pass Everything
    /*maskMsg.protocolID = ProtocolID::ISO14230;
    maskMsg.RxStatus = 0;
    maskMsg.TxFlags = 0;
    maskMsg.Timestamp = 0;
    maskMsg.DataSize = 1;
    maskMsg.ExtraDataIndex = 1;
    maskMsg.Data[0] = 0;

    patternMsg.protocolID = ProtocolID::ISO14230;
    patternMsg.RxStatus = 0;
    patternMsg.TxFlags = 0;
    patternMsg.Timestamp = 0;
    patternMsg.DataSize = 1;
    patternMsg.ExtraDataIndex = 1;
    patternMsg.Data[0] = 0;

    bool retVal = PassThruStartMsgFilter(channelID, Filter::PASS_FILTER, ref maskMsg, ref patternMsg, IntPtr.Zero, ref tmpMsgID) == RETURN_STATUS::STATUS_NOerror << std::endl;

    msgID[0] = tmpMsgID;

    return retVal;*/

    maskMsg.protocolID = ProtocolID::ISO14230;
    maskMsg.RxStatus = 0;
    maskMsg.TxFlags = 0;
    maskMsg.Timestamp = 0;
    maskMsg.DataSize = 1;
    maskMsg.ExtraDataIndex = 1;
    maskMsg.Data[0] = 192;

    patternMsg.protocolID = ProtocolID::ISO14230;
    patternMsg.RxStatus = 0;
    patternMsg.TxFlags = 0;
    patternMsg.Timestamp = 0;
    patternMsg.DataSize = 1;
    patternMsg.ExtraDataIndex = 1;
    patternMsg.Data[0] = 192;

    if (PassThruStartMsgFilter(channelID, Filter::PASS_FILTER, &maskMsg, &patternMsg, NULL, &tmpMsgID) != RETURN_STATUS::STATUS_NOERROR)
        return false;

    msgID[0] = tmpMsgID;

    patternMsg.protocolID = ProtocolID::ISO14230;
    patternMsg.RxStatus = 0;
    patternMsg.TxFlags = 0;
    patternMsg.Timestamp = 0;
    patternMsg.DataSize = 1;
    patternMsg.ExtraDataIndex = 1;
    patternMsg.Data[0] = 128;

    if (PassThruStartMsgFilter(channelID, Filter::PASS_FILTER, &maskMsg, &patternMsg, NULL, &tmpMsgID) != RETURN_STATUS::STATUS_NOERROR)
        return false;

    msgID[1] = tmpMsgID;

    patternMsg.protocolID = ProtocolID::ISO14230;
    patternMsg.RxStatus = 0;
    patternMsg.TxFlags = 0;
    patternMsg.Timestamp = 0;
    patternMsg.DataSize = 1;
    patternMsg.ExtraDataIndex = 1;
    patternMsg.Data[0] = 64;

    if (PassThruStartMsgFilter(channelID, Filter::PASS_FILTER, &maskMsg, &patternMsg, NULL, &tmpMsgID) != RETURN_STATUS::STATUS_NOERROR)
        return false;

    msgID[2] = tmpMsgID;

    return true;
}

bool J2534Worker::setConfig()
{
    bool retVal = false;
    SCONFIG_LIST list;
    SCONFIG config[12];

    memset(&list, 0, sizeof(SCONFIG_LIST));
    memset(config, 0, sizeof(SCONFIG) * 12);

    list.NumOfParams = 12;
    list.ConfigPtr = config;

    //DATA_RATE
    config[0].Parameter = 0x01;
    config[0].Value = 9600;
    //config[0].Value = 10400;

    //P1_MAX
    config[1].Parameter = 0x07;
    config[1].Value = 40;

    //P3_MIN
    //Min delay between response and new request
    config[2].Parameter = 0x0A;
    config[2].Value = 200; //ECU
    //config[2].Value = 110; //SMT
    config[2].Value = 10; //55

    //P4_MIN
    config[3].Parameter = 0x0B;
    config[3].Value = 10;

    //TIDLE
    config[4].Parameter = 0x13;
    config[4].Value = 300;

    //TINIL
    config[5].Parameter = 0x14;
    config[5].Value = 35;

    //TWUP
    config[6].Parameter = 0x15;
    config[6].Value = 50;

    //W1
    config[7].Parameter = 0x0E;
    config[7].Value = 105;
    config[7].Value = 25;

    //W2
    config[8].Parameter = 0x0F;
    config[8].Value = 20;

    //W3
    //Max time between key bytes
    config[9].Parameter = 0x10;
    config[9].Value = 20;

    //W4
    config[10].Parameter = 0x11;
    config[10].Value = 50;
    config[10].Value = 25;

    //W5
    //Min time before transmitting the address byte
    config[11].Parameter = 0x12;
    config[11].Value = 330;
    config[11].Value = 10;

    //call
    retVal = PassThruIoctl(channelID, IOCTL_ID::SET_CONFIG, &list, NULL) == RETURN_STATUS::STATUS_NOERROR;
    PassThruIoctl(channelID, IOCTL_ID::CLEAR_PERIODIC_MSGS, NULL, NULL);

    return retVal;
}

bool J2534Worker::fastInit(uint8_t address)
{
    PASSTHRU_MSG input;
    PASSTHRU_MSG output;
    bool retVal = false;

    memset(&input, 0, sizeof(PASSTHRU_MSG));
    memset(&output, 0, sizeof(PASSTHRU_MSG));

    input.protocolID = ProtocolID::ISO14230;
    input.RxStatus = 0;
    input.TxFlags = 0;
    input.Timestamp = 0;
    input.DataSize = 4;
    input.ExtraDataIndex = 4;

    if (address == 0x33 || address == 0xFE)
    {
        input.Data[0] = 0xC1;  //Functional Addressing
        input.Data[1] = address;
        input.Data[2] = 240;
        input.Data[3] = 0x81;
    }
    else
    {
        input.Data[0] = 0x81; //Physical Addressing
        input.Data[1] = address;
        input.Data[2] = 240;
        input.Data[3] = 0x81;
    }

    PassThruIoctl(channelID, IOCTL_ID::FAST_INIT, &input, &output);

    if (output.DataSize == 7)
    {
        retVal = true;
    }

    keepReading();

    PassThruIoctl(channelID, IOCTL_ID::CLEAR_PERIODIC_MSGS, NULL, NULL);

    return retVal;
}

void J2534Worker::keepReading()
{
    PASSTHRU_MSG input;
    uint32_t numMsgs = 1;

    memset(&input, 0, sizeof(PASSTHRU_MSG));

    QThread::sleep(1);
    while (PassThruReadMsgs(channelID, &input, &numMsgs, 1000) == RETURN_STATUS::STATUS_NOERROR && QThread::currentThread()->isInterruptionRequested() == false)
    {
        if (numMsgs == 0) break;
        QThread::sleep(1);
    }
}

void J2534Worker::j2534Connect() {
    qDebug() << "J2534: Connecting to J2534 Server";
    QThread::sleep(3);

    while (QThread::currentThread()->isInterruptionRequested() == false) {
        deviceID = 0;

        if (PassThruOpen(NULL, &deviceID) == RETURN_STATUS::STATUS_NOERROR) {
            qDebug() << "J2534: PassThru opened";
            if (PassThruConnect(deviceID, ProtocolID::ISO14230, 4096, 10400, &channelID) == RETURN_STATUS::STATUS_NOERROR) {
                qDebug() << "J2534: PassThru connected";
                if (startMsgFilters()) {
                    qDebug() << "J2534: Set message filters";
                    if (setConfig()) {
                        qDebug() << "J2534: Set config";
                        if (fastInit(0x19)) {
                            qDebug() << "J2534: Fast Init Done";
                            getData();
                            qDebug() << "J2534: getData finished";
                            PassThruDisconnect(channelID);
                            PassThruClose(deviceID);
                        } else {
                            qDebug() << "J2534: PassThru fast init failed";
                            PassThruDisconnect(channelID);
                            PassThruClose(deviceID);
                        }
                    } else {
                        qDebug() << "J2534: PassThru couldn't set config";
                        PassThruDisconnect(channelID);
                        PassThruClose(deviceID);
                    }
                } else {
                     qDebug() << "J2534: PassThru can't start message filters";
                    PassThruDisconnect(channelID);
                    PassThruClose(deviceID);
                }
            } else {
                qDebug() << "J2534: PassThru couldn't connect";
                PassThruClose(deviceID);
            }
        }

        if(QThread::currentThread()->isInterruptionRequested() == false) {
            qDebug() << "J2534: Looping awaiting to reconnect";
            QThread::sleep(15);
        } else {
            break;
        }
    }
    qDebug() << "J2534: Stopped connecting to J2534 server";
}

void J2534Worker::getData()
{
    uint32_t i = 0;

    //SMT 0x19
    //fastInit(0x13); //ECU
    //fastInit(0x29); //VSC

    while (QThread::currentThread()->isInterruptionRequested() == false)
    {
        //emit stftb1(requestPID(0x19, 0x01, 0x06));
        //emit stftb2(requestPID(0x19, 0x01, 0x07));
        //emit maf(requestPID(0x19, 0x01, 0x10));
        emit vss((int)requestPID(0x19, 0x01, 0x0D));

        if ((i % 2) == 0) {
            emit gear((int)requestPID(0x19, 0x01, 0xBA));
        } else {
            emit iat(requestPID(0x19, 0x01, 0x0F));
            emit ect(requestPID(0x19, 0x01, 0x05));
        }

        i++;
        if(i > 3000) break; //force disconnection due to DLL memory leaks in some closed source windows DLLs
    }
}

double J2534Worker::requestPID(uint8_t address, uint8_t mode, uint8_t pid)
{
    PASSTHRU_MSG pidMSG;
    uint32_t numMsgs = 1;
    double retVal = 0;
    RETURN_STATUS ret;

    memset(&pidMSG, 0, sizeof(PASSTHRU_MSG));

    pidMSG.RxStatus = 0;
    pidMSG.TxFlags = 0;
    pidMSG.protocolID = ProtocolID::ISO14230;
    pidMSG.DataSize = pidMSG.ExtraDataIndex = 5;
    pidMSG.Data[0] = 130;
    pidMSG.Data[1] = address;
    pidMSG.Data[2] = 240;
    pidMSG.Data[3] = mode;
    pidMSG.Data[4] = pid;

    PassThruWriteMsgs(channelID, &pidMSG, &numMsgs, 1000);

    numMsgs = 1;
    memset(&pidMSG, 0, sizeof(PASSTHRU_MSG));

    while ((ret = PassThruReadMsgs(channelID, &pidMSG, &numMsgs, 5000)) == RETURN_STATUS::STATUS_NOERROR)
    {
        if (numMsgs == 1)
        {
            if (pidMSG.RxStatus == 0)
            {
                break;
            }
        }
        QThread::msleep(100);
    }

    if (ret == RETURN_STATUS::ERR_TIMEOUT)
    {
        return 0;
    }

    if (address == 0x13)
    {
        //ECU
        switch (pid)
        {
        case 0x05:
            //ECT
            retVal = pidMSG.Data[5] - 40;
            break;
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
            //Fuel Trims
            retVal = pidMSG.Data[5] / 1.28 - 100;
            break;
        case 0x0C:
            //RPM
            retVal = pidMSG.Data[5] * 64 + (pidMSG.Data[6] / 4.0);
            break;
        case 0x10:
            //MAF
            retVal = (pidMSG.Data[5] * 256 + pidMSG.Data[6]) / 100.0;
            break;
        case 0x11:
            //TPS
            retVal = (100.0 / 255.0) * pidMSG.Data[5];
            break;
        case 0x0F:
            //IAT
            retVal = pidMSG.Data[5] - 40;
            break;
        case 0x0D:
            //VSS
            retVal = pidMSG.Data[5];
            break;
        }
    }
    else if (address == 0x19)
    {
        //SMT
        switch (pid)
        {
        case 0x05:
            //ECT
            retVal = pidMSG.Data[5] - 40;
            break;
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
            //Fuel Trims
            retVal = pidMSG.Data[5] / 1.28 - 100;
            break;
        case 0x0C:
            //RPM
            retVal = pidMSG.Data[5] * 64 + (pidMSG.Data[6] / 4.0);
            break;
        case 0x0D:
            //VSS
            retVal = pidMSG.Data[5];
            break;
        case 0x0F:
            //IAT
            retVal = pidMSG.Data[5] - 40;
            break;
        case 0x10:
            //MAF
            retVal = (pidMSG.Data[5] * 256 + pidMSG.Data[6]) / 100.0;
            break;
        case 0x11:
            //Absolute TPS %
            retVal = pidMSG.Data[5] * (100.0 / 255.0);
            break;
        case 0xA2:
            //Acc pedal angle %
            retVal = ((uint16_t)pidMSG.Data[6] | (uint16_t)pidMSG.Data[5] << 8) / 10.0;
            break;
        case 0xBA:
            //Gear
            // 240 = R 1 = 1st 0 = N
            retVal = pidMSG.Data[5]; // Data[6] is the requested Gear
            break;
        }
    }
    else if (address == 0x29)
    {
        //VSC
        switch (pid)
        {
        case 0x09:
            //throttle position degrees
            retVal = pidMSG.Data[5] * (124.0 / 255.0);
            break;
        case 0x0B:
            //RPM
            retVal = ((uint16_t)pidMSG.Data[6] | (uint16_t)pidMSG.Data[5] << 8);
            break;
        case 0x0E:
            //Steering Angle
            retVal = ((uint16_t)pidMSG.Data[6] | (uint16_t)pidMSG.Data[5] << 8) * (73729.0 / 65535.0) - 36864;
            break;
        case 0x22:
            //Vehicle Speed
            retVal = pidMSG.Data[9];
            break;
        case 0x24:
            //Master Cylinder
            retVal = pidMSG.Data[5] * (5.0 / 255.0);
            break;
        }
    }

    //pidMSG.Data = null;
    PassThruIoctl(channelID, IOCTL_ID::CLEAR_PERIODIC_MSGS, NULL, NULL);

    return roundf(retVal * 100) / 100;
}
