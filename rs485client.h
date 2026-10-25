#ifndef RS485CLIENT_H
#define RS485CLIENT_H

#include <QByteArray>
#include <QLocalSocket>
#include <QObject>
#include <QTimer>

// Values returned by the rs485d daemon for UI_CMD_GET_ANALOGUE_INPUTS.
// Order and units match sMeasurements_t in the daemon (services/measurements).
struct Measurements
{
    float vBatt = 0, vRect = 0, vLoad = 0;
    float iBatt = 0, iRect = 0, iLoad = 0;
    float tBatt = 0;
    float vEarth = 0, v24V = 0, tOnboard = 0, vRipple = 0;
    bool  complete = false;   // false if the daemon only sent vBatt
};

// Talks to the rs485d daemon over its Unix socket. The daemon handles the
// RS485 framing and CRC; this class only asks for data and decodes the reply.
class Rs485Client : public QObject
{
    Q_OBJECT

public:
    explicit Rs485Client(QObject *parent = nullptr,
                         const QString &socketPath = QStringLiteral("/tmp/rs485d.sock"));

    // Ask the daemon for the latest analogue inputs. Ignored while a
    // previous request is still in progress.
    void requestAnalogueInputs();

    bool busy() const { return m_busy; }

signals:
    void analogueInputsReceived(const Measurements &m);
    void failed(const QString &reason);

private:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void finish(bool ok, const QString &reason = QString());
    void parse();

    static constexpr char kCmdGetAnalogueInputs = 1;   // UI_CMD_GET_ANALOGUE_INPUTS
    static constexpr int  kFullSize = 44;              // 11 floats
    static constexpr int  kVBattOnlySize = 4;

    QLocalSocket m_socket;
    QTimer m_timeout;
    QString m_path;
    QByteArray m_buffer;
    bool m_busy = false;
};

#endif // RS485CLIENT_H
