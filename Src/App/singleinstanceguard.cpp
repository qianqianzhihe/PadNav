#include "singleinstanceguard.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QDebug>

#include <memory>
#include <utility>

namespace pn::app {

namespace {

constexpr auto connectTimeoutMs = 100;
constexpr auto listenRetryCount = 2;

} // namespace

SingleInstanceGuard::SingleInstanceGuard(QString serverName)
    : m_serverName(std::move(serverName))
    , m_server(std::make_unique<QLocalServer>()) {}

SingleInstanceGuard::~SingleInstanceGuard() = default;

auto SingleInstanceGuard::tryAcquire() -> bool {
    if (hasRunningInstance()) {
        return false;
    }

    QLocalServer::removeServer(m_serverName);
    return listen();
}

auto SingleInstanceGuard::hasRunningInstance() const -> bool {
    auto socket = QLocalSocket{};
    socket.connectToServer(m_serverName, QIODeviceBase::ReadWrite);
    return socket.waitForConnected(connectTimeoutMs);
}

auto SingleInstanceGuard::listen() -> bool {
    if (!m_server) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: m_server";
        return false;
    }

    for (auto retryIndex = 0; retryIndex < listenRetryCount; ++retryIndex) {
        if (m_server->listen(m_serverName)) {
            return true;
        }

        QLocalServer::removeServer(m_serverName);
    }

    qWarning() << __FILE__ << __LINE__ << "single instance listen failed:" << m_server->errorString();
    return false;
}

} // namespace pn::app
