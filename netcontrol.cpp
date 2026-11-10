#include "netcontrol.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>
#include <QVariant>
#include <algorithm>

namespace {
const int quickMs = 8000;
const int radioMs = 15000;
const int scanMs = 30000;
const int connectMs = 60000;
const int applyMs = 60000;
}

NetControl::NetControl(QObject *parent)
    : QObject(parent)
{
}

// ---- helpers ---------------------------------------------------------------

QStringList NetControl::splitTerse(const QString &line)
{
    QStringList parts;
    QString current;
    for (int i = 0; i < line.size(); ++i) {
        const QChar c = line.at(i);
        if (c == QLatin1Char('\\') && i + 1 < line.size())
            current += line.at(++i);                // an escaped colon or backslash
        else if (c == QLatin1Char(':')) {
            parts << current;
            current.clear();
        } else
            current += c;
    }
    parts << current;
    return parts;
}

void NetControl::splitAddress(const QString &text, QString *address, int *prefix)
{
    const QString first = text.section(',', 0, 0).trimmed();        // only the first address
    *address = first.section('/', 0, 0);
    bool ok = false;
    const int p = first.section('/', 1, 1).toInt(&ok);
    *prefix = ok ? p : 0;
}

QString NetControl::errorText(int exitCode, const QString &out, const QString &err)
{
    QString text = err.trimmed();
    if (text.isEmpty())
        text = out.trimmed();
    if (text.isEmpty())
        text = QString("nmcli failed (code %1)").arg(exitCode);
    if (text.startsWith("Error: "))
        text = text.mid(7);
    return text.section('\n', 0, 0);                // the first line is the useful one
}

// Runs nmcli with the given arguments (no shell) and calls done() when it has finished.
void NetControl::run(const QStringList &args, int timeoutMs, Done done)
{
    auto *p = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C");                      // English, parseable output
    p->setProcessEnvironment(env);

    auto *timer = new QTimer(p);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, p, [p]() {
        p->setProperty("timedOut", true);
        p->kill();
    });

    connect(p, &QProcess::errorOccurred, this, [p, done](QProcess::ProcessError e) {
        if (e == QProcess::FailedToStart) {
            done(-1, QString(), "nmcli could not be started (is NetworkManager installed?)");
            p->deleteLater();
        }
    });
    connect(p, &QProcess::finished, this, [p, done](int code, QProcess::ExitStatus) {
        const QString out = QString::fromUtf8(p->readAllStandardOutput());
        QString err = QString::fromUtf8(p->readAllStandardError());
        if (p->property("timedOut").toBool())
            err = "Timed out";
        done(code, out, err);
        p->deleteLater();
    });

    p->start("nmcli", args);
    timer->start(timeoutMs);
}

// ---- Wi-Fi ----------------------------------------------------------------

void NetControl::refreshWifi()
{
    run({"-t", "-f", "DEVICE,TYPE", "device"}, quickMs, [this](int code, const QString &out, const QString &) {
        WifiState state;
        if (code == 0) {
            for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                const QStringList f = splitTerse(line);
                if (f.size() >= 2 && f.at(1) == "wifi") {
                    state.adapterPresent = true;
                    state.device = f.at(0);
                    break;
                }
            }
        }
        m_wifiDevice = state.device;
        if (!state.adapterPresent) {
            emit wifiStateChanged(state);
            return;
        }
        run({"-t", "-f", "WIFI", "radio"}, quickMs, [this, state](int code2, const QString &out2, const QString &) mutable {
            state.radioEnabled = code2 == 0 && out2.trimmed() == "enabled";
            if (!state.radioEnabled) {
                emit wifiStateChanged(state);
                return;
            }
            run({"-t", "-f", "GENERAL.CONNECTION,IP4.ADDRESS", "device", "show", state.device}, quickMs,
                [this, state](int code3, const QString &out3, const QString &) mutable {
                    if (code3 == 0) {
                        for (const QString &line : out3.split('\n', Qt::SkipEmptyParts)) {
                            const QStringList f = splitTerse(line);
                            if (f.size() < 2)
                                continue;
                            if (f.at(0) == "GENERAL.CONNECTION" && f.at(1) != "--")
                                state.connection = f.at(1);
                            else if (f.at(0).startsWith("IP4.ADDRESS") && state.ip.isEmpty()) {
                                int prefix = 0;
                                splitAddress(f.at(1), &state.ip, &prefix);
                            }
                        }
                    }
                    emit wifiStateChanged(state);
                });
        });
    });
}

void NetControl::scanWifi()
{
    run({"-t", "-f", "IN-USE,SSID,SIGNAL,SECURITY", "device", "wifi", "list", "--rescan", "yes"}, scanMs,
        [this](int code, const QString &out, const QString &err) {
            QList<WifiNetwork> list;
            if (code != 0) {
                emit wifiListReady(list, errorText(code, out, err));
                return;
            }
            for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                const QStringList f = splitTerse(line);
                if (f.size() < 4 || f.at(1).isEmpty())
                    continue;                           // hidden network: no name to show
                WifiNetwork n;
                n.inUse = f.at(0).trimmed() == "*";
                n.ssid = f.at(1);
                n.signal = f.at(2).toInt();
                n.secured = !f.at(3).trimmed().isEmpty() && f.at(3).trimmed() != "--";

                // The same network is often seen several times (several access points): keep the strongest
                bool merged = false;
                for (WifiNetwork &existing : list) {
                    if (existing.ssid == n.ssid) {
                        existing.signal = std::max(existing.signal, n.signal);
                        existing.inUse = existing.inUse || n.inUse;
                        merged = true;
                        break;
                    }
                }
                if (!merged)
                    list.append(n);
            }
            std::sort(list.begin(), list.end(), [](const WifiNetwork &a, const WifiNetwork &b) {
                if (a.inUse != b.inUse)
                    return a.inUse;                     // the connected network first
                return a.signal > b.signal;             // then the strongest
            });
            emit wifiListReady(list, QString());
        });
}

void NetControl::setWifiEnabled(bool on)
{
    run({"radio", "wifi", on ? "on" : "off"}, radioMs, [this](int code, const QString &out, const QString &err) {
        emit actionFinished("wifi-radio", code == 0, code == 0 ? QString() : errorText(code, out, err));
    });
}

void NetControl::connectWifi(const QString &ssid, const QString &password, bool secured)
{
    const QString ifname = m_wifiDevice;
    auto finish = [this](int code, const QString &out, const QString &err) {
        emit actionFinished("wifi-connect", code == 0, code == 0 ? QString() : errorText(code, out, err));
    };
    auto connectNew = [this, ssid, password, secured, ifname, finish]() {
        QStringList args = {"device", "wifi", "connect", ssid};
        if (secured && !password.isEmpty())
            args << "password" << password;
        if (!ifname.isEmpty())
            args << "ifname" << ifname;
        run(args, connectMs, finish);
    };

    // A network that was connected to before already has a saved profile: give it the new password and
    // bring it up, instead of making a second profile with the same name.
    run({"-t", "-f", "NAME,TYPE", "connection", "show"}, quickMs,
        [this, ssid, password, secured, ifname, finish, connectNew](int code, const QString &out, const QString &) {
            bool known = false;
            if (code == 0) {
                for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                    const QStringList f = splitTerse(line);
                    if (f.size() >= 2 && f.at(0) == ssid && f.at(1) == "802-11-wireless") {
                        known = true;
                        break;
                    }
                }
            }
            if (!known) {
                connectNew();
                return;
            }
            auto bringUp = [this, ssid, ifname, finish]() {
                QStringList args = {"connection", "up", ssid};
                if (!ifname.isEmpty())
                    args << "ifname" << ifname;
                run(args, connectMs, finish);
            };
            if (secured && !password.isEmpty()) {
                run({"connection", "modify", ssid, "wifi-sec.psk", password}, quickMs,
                    [finish, bringUp](int code2, const QString &out2, const QString &err2) {
                        if (code2 != 0)
                            finish(code2, out2, err2);
                        else
                            bringUp();
                    });
            } else {
                bringUp();
            }
        });
}

// ---- LAN ------------------------------------------------------------------

void NetControl::refreshLan(bool full)
{
    run({"-t", "-f", "DEVICE,TYPE,STATE,CONNECTION", "device"}, quickMs,
        [this, full](int code, const QString &out, const QString &) {
            LanState state;
            state.full = full;
            QString connectionName;
            if (code == 0) {
                for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                    const QStringList f = splitTerse(line);
                    if (f.size() >= 4 && f.at(1) == "ethernet") {
                        state.present = true;
                        state.device = f.at(0);
                        state.linkUp = f.at(2) == "connected";
                        connectionName = f.at(3) == "--" ? QString() : f.at(3);
                        break;
                    }
                }
            }
            m_lanDevice = state.device;
            if (!state.present) {
                emit lanStateChanged(state);
                return;
            }

            run({"-t", "-f", "IP4.ADDRESS,IP4.GATEWAY,IP4.DNS", "device", "show", state.device}, quickMs,
                [this, state, connectionName, full](int code2, const QString &out2, const QString &) mutable {
                    if (code2 == 0) {
                        for (const QString &line : out2.split('\n', Qt::SkipEmptyParts)) {
                            const QStringList f = splitTerse(line);
                            if (f.size() < 2 || f.at(1).isEmpty() || f.at(1) == "--")
                                continue;
                            if (f.at(0).startsWith("IP4.ADDRESS") && state.liveAddress.isEmpty())
                                splitAddress(f.at(1), &state.liveAddress, &state.livePrefix);
                            else if (f.at(0) == "IP4.GATEWAY")
                                state.liveGateway = f.at(1);
                            else if (f.at(0).startsWith("IP4.DNS") && state.liveDns.isEmpty())
                                state.liveDns = f.at(1);
                        }
                    }
                    if (!full) {
                        state.profile = m_lanProfile;
                        emit lanStateChanged(state);
                        return;
                    }

                    // Which profile belongs to this port? The active one, else the first wired one.
                    auto readProfile = [this, state](const QString &profile) mutable {
                        m_lanProfile = profile;
                        state.profile = profile;
                        if (profile.isEmpty()) {
                            emit lanStateChanged(state);
                            return;
                        }
                        run({"-t", "-f", "ipv4.method,ipv4.addresses,ipv4.gateway,ipv4.dns", "connection", "show", profile}, quickMs,
                            [this, state](int code3, const QString &out3, const QString &) mutable {
                                if (code3 == 0) {
                                    for (const QString &line : out3.split('\n', Qt::SkipEmptyParts)) {
                                        const QStringList f = splitTerse(line);
                                        if (f.size() < 2 || f.at(1) == "--")
                                            continue;
                                        if (f.at(0) == "ipv4.method")
                                            state.method = f.at(1);
                                        else if (f.at(0) == "ipv4.addresses")
                                            splitAddress(f.at(1), &state.cfgAddress, &state.cfgPrefix);
                                        else if (f.at(0) == "ipv4.gateway")
                                            state.cfgGateway = f.at(1);
                                        else if (f.at(0) == "ipv4.dns")
                                            state.cfgDns = f.at(1).section(',', 0, 0).trimmed();
                                    }
                                }
                                emit lanStateChanged(state);
                            });
                    };
                    if (!connectionName.isEmpty()) {
                        readProfile(connectionName);
                        return;
                    }
                    run({"-t", "-f", "NAME,TYPE,DEVICE", "connection", "show"}, quickMs,
                        [state, readProfile](int code4, const QString &out4, const QString &) mutable {
                            QString chosen;
                            if (code4 == 0) {
                                for (const QString &line : out4.split('\n', Qt::SkipEmptyParts)) {
                                    const QStringList f = splitTerse(line);
                                    if (f.size() >= 3 && f.at(1) == "802-3-ethernet") {
                                        if (f.at(2) == state.device) {
                                            chosen = f.at(0);
                                            break;
                                        }
                                        if (chosen.isEmpty())
                                            chosen = f.at(0);
                                    }
                                }
                            }
                            readProfile(chosen);
                        });
                });
        });
}

void NetControl::applyLan(bool dhcp, const QString &address, int prefix, const QString &gateway, const QString &dns)
{
    const QString device = m_lanDevice;
    const QString profile = m_lanProfile;

    QStringList settings;
    if (dhcp) {
        settings << "ipv4.method" << "auto" << "ipv4.addresses" << "" << "ipv4.gateway" << "" << "ipv4.dns" << "";
    } else {
        settings << "ipv4.method" << "manual"
                 << "ipv4.addresses" << QString("%1/%2").arg(address).arg(prefix)
                 << "ipv4.gateway" << gateway
                 << "ipv4.dns" << dns;
    }

    auto activate = [this, device](const QString &name) {
        QStringList args = {"connection", "up", name};
        if (!device.isEmpty())
            args << "ifname" << device;
        run(args, applyMs, [this, name](int code, const QString &out, const QString &err) {
            m_lanProfile = name;
            if (code == 0)
                emit actionFinished("lan-apply", true, QString());
            else
                emit actionFinished("lan-apply", false,
                                    QString("The settings were saved, but could not be applied now: %1").arg(errorText(code, out, err)));
        });
    };

    if (profile.isEmpty()) {
        // No profile for this port yet: make one
        QStringList args = {"connection", "add", "type", "ethernet", "con-name", "SP515 LAN"};
        if (!device.isEmpty())
            args << "ifname" << device;
        args << settings;
        run(args, quickMs, [this, activate](int code, const QString &out, const QString &err) {
            if (code != 0) {
                emit actionFinished("lan-apply", false, errorText(code, out, err));
                return;
            }
            activate("SP515 LAN");
        });
        return;
    }

    QStringList args = {"connection", "modify", profile};
    args << settings;
    run(args, quickMs, [this, activate, profile](int code, const QString &out, const QString &err) {
        if (code != 0) {
            emit actionFinished("lan-apply", false, errorText(code, out, err));
            return;
        }
        activate(profile);
    });
}
