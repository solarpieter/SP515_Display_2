#ifndef RS485CLIENT_H
#define RS485CLIENT_H

#include <QByteArray>
#include <QLocalSocket>
#include <QObject>
#include <QQueue>
#include <QTimer>

// Values returned by the rs485d daemon for GET_ANALOGUE_INPUTS.
// Order and units match sUI_AnalogueInputs_t in the daemon.
struct Measurements
{
    float vBatt = 0, vRect = 0, vLoad = 0;
    float iBatt = 0, iRect = 0, iLoad = 0;
    float tBatt = 0;
    float vEarth = 0, v24V = 0, tOnboard = 0, vRipple = 0;
    bool  complete = false;   // true once all values were received
};

// Alarm flags from the SP515. Bit 0 is alarm 1; any bit set means an alarm is active.
struct Alarms
{
    quint32 bits = 0;
    bool any() const { return bits != 0; }
};

// Analogue calibration held by the SP515: one gain and one offset per channel.
struct AnalogueCalibration
{
    static constexpr int ChannelCount = 9;
    float gain[ChannelCount] = {};
    float offset[ChannelCount] = {};
};

// Talks to the rs485d daemon over its Unix socket. The daemon handles the
// RS485 framing and CRC; this class only sends commands and decodes replies.
//
// Socket protocol (little-endian), one request per connection:
//   request : U16 command, U16 transactionId, U16 payloadLength, payload
//   reply   : U16 command, U16 transactionId, U16 payloadLength, payload
//   error   : command has bit 0x8000 set, payload is one status byte
//
// Requests are queued and sent one at a time, so a poll and a button press
// can never get in each other's way.
class Rs485Client : public QObject
{
    Q_OBJECT

public:
    // Command numbers, as in the daemon's api.h (eUI_Command_t)
    enum Command : quint16
    {
        GetStatus = 0,
        GetAnalogueInputs = 1,
        GetDigitalInputs = 2,
        GetDigitalOutput = 3,
        GetAlarms = 4,
        ResetAlarms = 5,
        GetFloatConfig = 6,   SetFloatConfig = 7,
        GetBoostConfig = 8,   SetBoostConfig = 9,
        GetEqualizeConfig = 10, SetEqualizeConfig = 11,
        GetInitializeConfig = 12, SetInitializeConfig = 13,
        GetAnaGains = 20,
        SetAnaGain = 21,
        SetAnaOffset = 22,
        SetChargeSetpoint = 23,
    };

    explicit Rs485Client(QObject *parent = nullptr,
                         const QString &socketPath = QStringLiteral("/tmp/rs485d.sock"));

    // Ask for the analogue inputs. Ignored while one is already waiting, so a
    // slow daemon never builds up a pile of polls.
    void requestAnalogueInputs();

    // Ask for the alarm flags. Ignored while one is already waiting.
    void requestAlarms();

    // Gains and offsets for all channels
    void requestAnalogueCalibration();

    // Write one gain or offset to the SP515 (channel 0..8)
    void setAnalogueGain(quint8 channel, float value);
    void setAnalogueOffset(quint8 channel, float value);

    // Switch the charger on or off. The result arrives in commandFinished(): ok is true only
    // when the SP515 has acknowledged the new state (via the daemon).
    void setChargerSetpoint(bool enabled, float voltage, float currentLimit);

    // Any other command, for pages that need it (configs, alarms, ...)
    void sendCommand(quint16 command, const QByteArray &payload = QByteArray());

    bool busy() const { return m_busy; }

signals:
    void analogueInputsReceived(const Measurements &m);
    void analogueCalibrationReceived(const AnalogueCalibration &c);
    void alarmsReceived(const Alarms &a);

    // Raw payload of every successful reply
    void replyReceived(quint16 command, const QByteArray &payload);

    // Result of any command other than the analogue-input poll
    void commandFinished(quint16 command, bool ok, const QString &message);

    // The analogue-input poll failed (daemon not running, no answer, bad reply)
    void failed(const QString &reason);

private:
    struct Request
    {
        quint16 command = 0;
        quint16 transactionId = 0;
        QByteArray payload;
    };

    void enqueue(quint16 command, const QByteArray &payload);
    void startNext();
    void onConnected();
    void onReadyRead();
    void onSocketClosed();
    void tryComplete();
    void finishRequest(bool ok, const QString &message, const QByteArray &payload = QByteArray());
    static QString statusText(quint8 status);

    static constexpr int HeaderSize = 6;
    static constexpr int MaxPayload = 256;
    static constexpr quint16 ErrorFlag = 0x8000;

    QLocalSocket m_socket;
    QTimer m_timeout;
    QString m_path;
    QQueue<Request> m_queue;
    Request m_current;
    QByteArray m_buffer;
    quint16 m_nextTransactionId = 1;
    bool m_busy = false;
    bool m_pollPending = false;
    bool m_alarmsPending = false;
};

#endif // RS485CLIENT_H
