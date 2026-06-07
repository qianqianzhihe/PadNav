#ifndef PADNAV_SINGLEINSTANCEGUARD_H
#define PADNAV_SINGLEINSTANCEGUARD_H

#include <QString>

#include <memory>

class QLocalServer;

namespace pn::app {

class SingleInstanceGuard final {
public:
    explicit SingleInstanceGuard(QString serverName);
    ~SingleInstanceGuard();

    [[nodiscard]] auto tryAcquire() -> bool;

private:
    [[nodiscard]] auto hasRunningInstance() const -> bool;
    [[nodiscard]] auto listen() -> bool;

private:
    QString m_serverName;
    std::unique_ptr<QLocalServer> m_server;
};

} // namespace pn::app

#endif // PADNAV_SINGLEINSTANCEGUARD_H
