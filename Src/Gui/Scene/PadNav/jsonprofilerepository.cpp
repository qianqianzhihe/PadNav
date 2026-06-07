#include "jsonprofilerepository.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <array>
#include <optional>

namespace pn::gui {

namespace {

constexpr auto configFileName = "config.json";
constexpr auto deadZonePercentKey = "deadZonePercent";
constexpr auto mouseSpeedKey = "mouseMaxSpeedPixelsPerSecond";
constexpr auto scrollSpeedKey = "scrollSpeedWheelUnitsPerSecond";
constexpr auto bindingsKey = "bindings";

struct ActionName final {
    pn::core::ChromeActionId id;
    const char* name;
};

constexpr auto actionNames = std::array{
    ActionName{pn::core::ChromeActionId::Back, "Back"},
    ActionName{pn::core::ChromeActionId::Forward, "Forward"},
    ActionName{pn::core::ChromeActionId::Reload, "Reload"},
    ActionName{pn::core::ChromeActionId::StopLoading, "StopLoading"},
    ActionName{pn::core::ChromeActionId::PageTop, "PageTop"},
    ActionName{pn::core::ChromeActionId::PageBottom, "PageBottom"},
    ActionName{pn::core::ChromeActionId::ZoomIn, "ZoomIn"},
    ActionName{pn::core::ChromeActionId::ZoomOut, "ZoomOut"},
    ActionName{pn::core::ChromeActionId::ResetZoom, "ResetZoom"},
    ActionName{pn::core::ChromeActionId::FocusAddressBar, "FocusAddressBar"},
    ActionName{pn::core::ChromeActionId::NewTab, "NewTab"},
    ActionName{pn::core::ChromeActionId::CloseTab, "CloseTab"},
    ActionName{pn::core::ChromeActionId::PreviousTab, "PreviousTab"},
    ActionName{pn::core::ChromeActionId::NextTab, "NextTab"},
    ActionName{pn::core::ChromeActionId::RestoreClosedTab, "RestoreClosedTab"},
    ActionName{pn::core::ChromeActionId::FindInPage, "FindInPage"},
};

void debugFallback(const QString& reason) {
#ifdef QT_DEBUG
    qDebug() << "JsonProfileRepository: fallback to default profile:" << reason;
#else
    Q_UNUSED(reason)
#endif
}

[[nodiscard]] auto readInt(const QJsonObject& object, const QString& key) -> std::optional<int> {
    const auto value = object.value(key);
    if (!value.isDouble()) {
        return std::nullopt;
    }

    const auto number = value.toDouble();
    const auto integer = static_cast<int>(number);
    if (static_cast<double>(integer) != number) {
        return std::nullopt;
    }

    return integer;
}

[[nodiscard]] auto readBindings(const QJsonObject& object)
    -> std::optional<std::array<pn::core::ActionBinding, 16>> {
    const auto value = object.value(QString::fromLatin1(bindingsKey));
    if (!value.isObject()) {
        return std::nullopt;
    }

    const auto bindingsObject = value.toObject();
    auto bindings = std::array<pn::core::ActionBinding, 16>{};
    for (auto index = std::size_t{}; index < actionNames.size(); ++index) {
        const auto& action = actionNames.at(index);
        const auto enabledValue = bindingsObject.value(QString::fromLatin1(action.name));
        if (!enabledValue.isBool()) {
            return std::nullopt;
        }

        bindings.at(index) = pn::core::ActionBinding{
            .id = action.id,
            .enabled = enabledValue.toBool(),
        };
    }

    return bindings;
}

[[nodiscard]] auto profileFromJson(const QJsonObject& object) -> std::optional<pn::core::ChromeProfile> {
    const auto deadZonePercent = readInt(object, QString::fromLatin1(deadZonePercentKey));
    const auto mouseSpeed = readInt(object, QString::fromLatin1(mouseSpeedKey));
    const auto scrollSpeed = readInt(object, QString::fromLatin1(scrollSpeedKey));
    const auto bindings = readBindings(object);
    if (!deadZonePercent || !mouseSpeed || !scrollSpeed || !bindings) {
        return std::nullopt;
    }

    auto profile = pn::core::ChromeProfile{
        .deadZonePercent = *deadZonePercent,
        .mouseMaxSpeedPixelsPerSecond = *mouseSpeed,
        .scrollSpeedWheelUnitsPerSecond = *scrollSpeed,
        .bindings = *bindings,
    };
    if (!pn::core::isValid(profile)) {
        return std::nullopt;
    }

    return profile;
}

[[nodiscard]] auto profileToJson(const pn::core::ChromeProfile& profile) -> QJsonObject {
    auto bindingsObject = QJsonObject{};
    for (const auto& action : actionNames) {
        bindingsObject.insert(QString::fromLatin1(action.name), pn::core::isEnabled(profile, action.id));
    }

    auto object = QJsonObject{};
    object.insert(QString::fromLatin1(deadZonePercentKey), profile.deadZonePercent);
    object.insert(QString::fromLatin1(mouseSpeedKey), profile.mouseMaxSpeedPixelsPerSecond);
    object.insert(QString::fromLatin1(scrollSpeedKey), profile.scrollSpeedWheelUnitsPerSecond);
    object.insert(QString::fromLatin1(bindingsKey), bindingsObject);
    return object;
}

} // namespace

auto JsonProfileRepository::load() const -> pn::core::ChromeProfile {
    auto file = QFile(filePath());
    if (!file.exists()) {
        return pn::core::makeDefaultChromeProfile();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        debugFallback(QStringLiteral("open failed"));
        return pn::core::makeDefaultChromeProfile();
    }

    const auto document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        debugFallback(QStringLiteral("root is not object"));
        return pn::core::makeDefaultChromeProfile();
    }

    const auto profile = profileFromJson(document.object());
    if (!profile) {
        debugFallback(QStringLiteral("invalid or incomplete json"));
        return pn::core::makeDefaultChromeProfile();
    }

    return *profile;
}

auto JsonProfileRepository::save(const pn::core::ChromeProfile& profile) const -> bool {
    if (!pn::core::isValid(profile)) {
        return false;
    }

    const auto path = filePath();
    auto directory = QDir{};
    if (!directory.mkpath(QFileInfo(path).absolutePath())) {
        return false;
    }

    auto file = QSaveFile(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    const auto document = QJsonDocument(profileToJson(profile));
    if (file.write(document.toJson(QJsonDocument::Indented)) < 0) {
        return false;
    }

    return file.commit();
}

auto JsonProfileRepository::filePath() const -> QString {
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (basePath.isEmpty()) {
        basePath = QDir::homePath() + QStringLiteral("/PadNav");
    }

    return QDir(basePath).filePath(QString::fromLatin1(configFileName));
}

} // namespace pn::gui
