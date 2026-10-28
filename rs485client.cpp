#include "rs485client.h"

#include <QDataStream>
#include <QIODevice>

Rs485Client::Rs485Client(QObject *parent, const QString &socketPath)
    : QObject(parent), m_path(socketPath)
{
    m_timeout.setSingleShot(true);
    m_timeout.setInterval(1500);
    connect(&m_timeout, &QTimer::timeout, this, [this]() {
        finishRequest(false, "daemon did not answer in time");
    });

    connect(&m_socket, &QLocalSocket::connected, this, &Rs485Client::onConnected);
    connect(&m_socket, &QLocalSocket::readyRead, this, &Rs485Client::onReadyRead);
    connect(&m_socket, &QLocalSocket::disconnected, this, &Rs485Client::onSocketClosed);
    connect(&m_socket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError err) {
        if (!m_busy)
            return;
        if (err == QLocalSocket::PeerClosedError)
            onSocketClosed();            // the daemon closes after replying: normal
        else
            finishRequest(false, m_socket.errorString());
    });
}

void Rs485Client::requestAnalogueInputs()
{
    if (m_pollPending)
        return;
    m_pollPending = true;
    enqueue(GetAnalogueInputs, QByteArray());
}

void Rs485Client::requestAnalogueCalibration()
{
    enqueue(GetAnaGains, QByteArray());
}

static QByteArray channelValuePayload(quint8 channel, float value)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    out << channel << value;               // U8 channel + float value = 5 bytes
    return payload;
}

void Rs485Client::setAnalogueGain(quint8 channel, float value)
{
    enqueue(SetAnaGain, channelValuePayload(channel, value));
}

void Rs485Client::setAnalogueOffset(quint8 channel, float value)
{
    enqueue(SetAnaOffset, channelValuePayload(channel, value));
}

void Rs485Client::sendCommand(quint16 command, const QByteArray &payload)
{
    enqueue(command, payload);
}

void Rs485Client::enqueue(quint16 command, const QByteArray &payload)
{
    Request r;
    r.command = command;
    r.transactionId = m_nextTransactionId++;
    if (m_nextTransactionId == 0)
        m_nextTransactionId = 1;
    r.payload = payload;
    m_queue.enqueue(r);
    startNext();
}

void Rs485Client::startNext()
{
    if (m_busy || m_queue.isEmpty())
        return;

    m_current = m_queue.dequeue();
    m_busy = true;
    m_buffer.clear();
    m_timeout.start();
    m_socket.abort();
    m_socket.connectToServer(m_path);
}

void Rs485Client::onConnected()
{
    // The daemon reads the header straight after accepting (it only waits
    // 200 ms), so send header and payload immediately in one go.
    QByteArray out;
    QDataStream s(&out, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << m_current.command << m_current.transactionId << quint16(m_current.payload.size());
    out.append(m_current.payload);
    m_socket.write(out);
    m_socket.flush();
}

void Rs485Client::onReadyRead()
{
    m_buffer.append(m_socket.readAll());
    tryComplete();
}

void Rs485Client::onSocketClosed()
{
    if (!m_busy)
        return;
    tryComplete();
    if (m_busy)                            // still waiting for more: it never will
        finishRequest(false, "daemon closed the connection without a full reply");
}

void Rs485Client::tryComplete()
{
    if (!m_busy || m_buffer.size() < HeaderSize)
        return;

    QDataStream in(m_buffer);
    in.setByteOrder(QDataStream::LittleEndian);
    quint16 command, transactionId, payloadLength;
    in >> command >> transactionId >> payloadLength;

    if (payloadLength > MaxPayload) {
        finishRequest(false, "malformed reply from the daemon");
        return;
    }
    if (m_buffer.size() < HeaderSize + payloadLength)
        return;                            // wait for the rest

    const QByteArray payload = m_buffer.mid(HeaderSize, payloadLength);

    if (transactionId != m_current.transactionId) {
        finishRequest(false, "reply does not match the request");
        return;
    }
    if (command & ErrorFlag) {
        const quint8 status = payload.isEmpty() ? quint8(3) : quint8(payload.at(0));
        finishRequest(false, statusText(status));
        return;
    }
    if (command != m_current.command) {
        finishRequest(false, "reply does not match the request");
        return;
    }
    finishRequest(true, QString(), payload);
}

QString Rs485Client::statusText(quint8 status)
{
    switch (status) {
    case 1: return "daemon rejected the request (invalid)";
    case 2: return "daemon is busy";
    case 3: return "the SP515 did not answer or rejected the value";
    case 4: return "command not supported by the daemon yet";
    default: return QString("daemon error %1").arg(status);
    }
}

void Rs485Client::finishRequest(bool ok, const QString &message, const QByteArray &payload)
{
    if (!m_busy)
        return;

    const Request done = m_current;
    m_busy = false;
    m_timeout.stop();
    m_socket.abort();
    if (done.command == GetAnalogueInputs)
        m_pollPending = false;

    if (ok) {
        QDataStream in(payload);
        in.setByteOrder(QDataStream::LittleEndian);
        in.setFloatingPointPrecision(QDataStream::SinglePrecision);

        if (done.command == GetAnalogueInputs) {
            if (payload.size() != 44) {
                emit failed(QString("unexpected reply length: %1 bytes").arg(payload.size()));
            } else {
                Measurements m;
                in >> m.vBatt >> m.vRect >> m.vLoad >> m.iBatt >> m.iRect >> m.iLoad
                   >> m.tBatt >> m.vEarth >> m.v24V >> m.tOnboard >> m.vRipple;
                m.complete = true;
                emit analogueInputsReceived(m);
            }
        } else if (done.command == GetAnaGains) {
            if (payload.size() != 2 * AnalogueCalibration::ChannelCount * int(sizeof(float))) {
                emit commandFinished(done.command, false,
                                     QString("unexpected reply length: %1 bytes").arg(payload.size()));
            } else {
                AnalogueCalibration c;
                for (int i = 0; i < AnalogueCalibration::ChannelCount; ++i) in >> c.gain[i];
                for (int i = 0; i < AnalogueCalibration::ChannelCount; ++i) in >> c.offset[i];
                emit analogueCalibrationReceived(c);
                emit commandFinished(done.command, true, QString());
            }
        } else {
            emit commandFinished(done.command, true, QString());
        }
        emit replyReceived(done.command, payload);
    } else if (done.command == GetAnalogueInputs) {
        emit failed(message);
    } else {
        emit commandFinished(done.command, false, message);
    }

    QTimer::singleShot(0, this, &Rs485Client::startNext);
}
