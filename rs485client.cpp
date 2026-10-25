#include "rs485client.h"

#include <QDataStream>

Rs485Client::Rs485Client(QObject *parent, const QString &socketPath)
    : QObject(parent), m_path(socketPath)
{
    m_timeout.setSingleShot(true);
    m_timeout.setInterval(1500);
    connect(&m_timeout, &QTimer::timeout, this, [this]() { finish(false, "daemon did not answer in time"); });

    connect(&m_socket, &QLocalSocket::connected, this, &Rs485Client::onConnected);
    connect(&m_socket, &QLocalSocket::readyRead, this, &Rs485Client::onReadyRead);
    connect(&m_socket, &QLocalSocket::disconnected, this, &Rs485Client::onDisconnected);
    connect(&m_socket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError err) {
        if (!m_busy)
            return;
        if (err == QLocalSocket::PeerClosedError)
            parse();                      // the daemon closes after replying: normal
        else
            finish(false, m_socket.errorString());
    });
}

void Rs485Client::requestAnalogueInputs()
{
    if (m_busy)
        return;
    m_busy = true;
    m_buffer.clear();
    m_timeout.start();
    m_socket.abort();
    m_socket.connectToServer(m_path);
}

void Rs485Client::onConnected()
{
    // The daemon reads exactly one command byte straight after accepting, so
    // send it immediately.
    m_socket.write(&kCmdGetAnalogueInputs, 1);
    m_socket.flush();
}

void Rs485Client::onReadyRead()
{
    m_buffer.append(m_socket.readAll());
    if (m_buffer.size() >= kFullSize)
        parse();                          // got everything, no need to wait for close
}

void Rs485Client::onDisconnected()
{
    if (m_busy)
        parse();                          // the daemon closes the socket after replying
}

void Rs485Client::parse()
{
    if (!m_busy)
        return;

    if (m_buffer.size() != kFullSize && m_buffer.size() != kVBattOnlySize) {
        finish(false, QString("unexpected reply length: %1 bytes").arg(m_buffer.size()));
        return;
    }

    QDataStream in(m_buffer);
    in.setByteOrder(QDataStream::LittleEndian);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);

    Measurements m;
    in >> m.vBatt;
    if (m_buffer.size() == kFullSize) {
        in >> m.vRect >> m.vLoad >> m.iBatt >> m.iRect >> m.iLoad >> m.tBatt
           >> m.vEarth >> m.v24V >> m.tOnboard >> m.vRipple;
        m.complete = true;
    }

    m_busy = false;
    m_timeout.stop();
    m_socket.abort();
    emit analogueInputsReceived(m);
}

void Rs485Client::finish(bool ok, const QString &reason)
{
    if (!m_busy)
        return;
    m_busy = false;
    m_timeout.stop();
    m_socket.abort();
    if (!ok)
        emit failed(reason);
}
