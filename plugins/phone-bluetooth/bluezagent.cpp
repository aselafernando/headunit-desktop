#include <QtDebug>
#include <QLoggingCategory>

#include "bluezagent.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT, "app.plugins.phone-bluetooth.bluezagent")

BluezAgent::BluezAgent(QObject *parent)
    : Agent(parent)
    , m_pinRequested(false)
    , m_passkeyRequested(false)
    , m_authorizationRequested(false)
    , m_cancelCalled(false)
    , m_releaseCalled(false)
{
}

QDBusObjectPath BluezAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/HUDBluezAgent"));
}

void BluezAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request)
{
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "requestPinCode";
    m_device = device;
    m_pinRequested = true;

    request.accept(QString());

}

void BluezAgent::displayPinCode(BluezQt::DevicePtr device, const QString &pinCode)
{
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "displayPinCode: " << pinCode;
    m_device = device;
    m_displayedPinCode = pinCode;
}

void BluezAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "requestPasskey";
    m_device = device;
    m_passkeyRequested = true;

    request.accept(0);
}

void BluezAgent::displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered)
{
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "displayPasskey passkey : " << passkey << " entered: " << entered;
    m_device = device;
    m_displayedPasskey = passkey;
    m_enteredPasskey = entered;
}

void BluezAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "requestConfirmation: " << device << " : " << passkey;
    m_device = device;
    m_requestedPasskey = passkey;

    request.accept();
}

void BluezAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request)
{
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "requestAuthorization";
    m_device = device;
    m_authorizationRequested = true;

    request.accept();
}

void BluezAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
  qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "authorizeService " << uuid;
    m_device = device;
    m_authorizedUuid = uuid;

    request.accept();
    device->setTrusted(true);
//    device->connectToDevice();
}

void BluezAgent::cancel()
{
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "cancel";
    m_cancelCalled = true;
}

void BluezAgent::release()
{
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZAGENT) << "release";
    m_releaseCalled = true;
}


BluezQt::Agent::Capability BluezAgent::capability() const{
  return DisplayOnly;
}
