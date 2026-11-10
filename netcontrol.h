#ifndef NETCONTROL_H
#define NETCONTROL_H

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <functional>

// One Wi-Fi network found by a scan
struct WifiNetwork
{
    QString ssid;
    int signal = 0;                 // 0 - 100
    bool secured = false;           // needs a password
    bool inUse = false;             // the one we are connected to
};

struct WifiState
{
    bool adapterPresent = false;
    bool radioEnabled = false;      // Wi-Fi switched on
    QString device;                 // for example wlan0
    QString connection;             // name of the network we are connected to, empty when not connected
    QString ip;                     // our address on it
};

struct LanState
{
    bool present = false;           // a wired adapter was found
    bool full = false;              // the configured values below were read too (not only the live ones)
    QString device;                 // for example eth0
    QString profile;                // the NetworkManager profile that is used for it
    bool linkUp = false;

    QString method;                 // "auto" (DHCP) or "manual" (static)
    QString cfgAddress;
    int cfgPrefix = 0;
    QString cfgGateway;
    QString cfgDns;

    QString liveAddress;            // what the adapter has right now (from DHCP or static)
    int livePrefix = 0;
    QString liveGateway;
    QString liveDns;
};

// Controls Wi-Fi and the wired network port through NetworkManager's command line tool, nmcli.
// Everything runs in the background; the results come back as signals. nmcli is started without a
// shell, so nothing typed on the screen (a password, say) is ever interpreted as a command.
class NetControl : public QObject
{
    Q_OBJECT

public:
    explicit NetControl(QObject *parent = nullptr);

    void refreshWifi();                                     // wifiStateChanged()
    void scanWifi();                                        // wifiListReady()
    void setWifiEnabled(bool on);                           // actionFinished("wifi-radio")
    void connectWifi(const QString &ssid, const QString &password, bool secured);   // actionFinished("wifi-connect")

    void refreshLan(bool full);                             // lanStateChanged(); full also reads the configured values
    void applyLan(bool dhcp, const QString &address, int prefix,
                  const QString &gateway, const QString &dns);                      // actionFinished("lan-apply")

    // Split one line of nmcli's terse output at the colons (a colon inside a value is written as "\:")
    static QStringList splitTerse(const QString &line);

signals:
    void wifiStateChanged(const WifiState &state);
    void wifiListReady(const QList<WifiNetwork> &networks, const QString &error);
    void lanStateChanged(const LanState &state);
    void actionFinished(const QString &action, bool ok, const QString &message);

private:
    using Done = std::function<void(int exitCode, const QString &out, const QString &err)>;
    void run(const QStringList &args, int timeoutMs, Done done);
    static QString errorText(int exitCode, const QString &out, const QString &err);
    static void splitAddress(const QString &text, QString *address, int *prefix);

    QString m_wifiDevice;
    QString m_lanDevice;
    QString m_lanProfile;
};

#endif // NETCONTROL_H
