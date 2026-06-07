#include "WindowsPlatform/crashdump.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <DbgHelp.h>
#include <KnownFolders.h>
#include <ShlObj.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace pn::platform {

namespace {

constexpr auto dumpFileExtension = L".dmp";

[[nodiscard]] auto crashDumpDirectory() -> std::filesystem::path {
    auto* localAppDataPath = PWSTR{};
    const auto result = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppDataPath);
    if (FAILED(result) || !localAppDataPath) {
        return {};
    }

    auto path = std::filesystem::path{localAppDataPath};
    CoTaskMemFree(localAppDataPath);
    path /= L"PadNav";
    path /= L"CrashDumps";
    return path;
}

[[nodiscard]] auto timestampText() -> std::wstring {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    auto localTime = std::tm{};
    localtime_s(&localTime, &time);

    auto buffer = std::array<wchar_t, 32>{};
    const auto written = std::wcsftime(buffer.data(), buffer.size(), L"%Y%m%d-%H%M%S", &localTime);
    if (written == 0U) {
        return L"unknown-time";
    }
    return buffer.data();
}

[[nodiscard]] auto dumpFilePath() -> std::filesystem::path {
    auto directory = crashDumpDirectory();
    if (directory.empty()) {
        return {};
    }

    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) {
        return {};
    }

    auto filePath = directory / L"padnav-";
    filePath += timestampText();
    filePath += dumpFileExtension;
    return filePath;
}

auto writeMiniDump(EXCEPTION_POINTERS* exceptionPointers) -> LONG {
    const auto filePath = dumpFilePath();
    if (filePath.empty()) {
        return EXCEPTION_EXECUTE_HANDLER;
    }

    const auto fileHandle = CreateFileW(filePath.c_str(),
                                        GENERIC_WRITE,
                                        0,
                                        nullptr,
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        return EXCEPTION_EXECUTE_HANDLER;
    }

    auto exceptionInfo = MINIDUMP_EXCEPTION_INFORMATION{
        .ThreadId = GetCurrentThreadId(),
        .ExceptionPointers = exceptionPointers,
        .ClientPointers = FALSE,
    };
    MiniDumpWriteDump(GetCurrentProcess(),
                      GetCurrentProcessId(),
                      fileHandle,
                      MiniDumpNormal,
                      exceptionPointers ? &exceptionInfo : nullptr,
                      nullptr,
                      nullptr);
    CloseHandle(fileHandle);
    CrashDump::pruneOldDumps();
    return EXCEPTION_EXECUTE_HANDLER;
}

[[nodiscard]] auto dumpFiles() -> std::vector<std::filesystem::directory_entry> {
    auto directory = crashDumpDirectory();
    if (directory.empty()) {
        return {};
    }

    std::error_code error;
    if (!std::filesystem::exists(directory, error) || error) {
        return {};
    }

    auto files = std::vector<std::filesystem::directory_entry>{};
    for (const auto& entry : std::filesystem::directory_iterator{directory, error}) {
        if (error) {
            return files;
        }
        if (entry.is_regular_file(error) && entry.path().extension() == dumpFileExtension) {
            files.push_back(entry);
        }
    }
    return files;
}

} // namespace

void CrashDump::install() {
    SetUnhandledExceptionFilter(writeMiniDump);
}

void CrashDump::pruneOldDumps(std::size_t keepCount) {
    auto files = dumpFiles();
    std::ranges::sort(files, [](const auto& left, const auto& right) {
        std::error_code leftError;
        std::error_code rightError;
        const auto leftTime = left.last_write_time(leftError);
        const auto rightTime = right.last_write_time(rightError);
        if (leftError || rightError) {
            return left.path().filename() > right.path().filename();
        }
        return leftTime > rightTime;
    });

    for (auto index = keepCount; index < files.size(); ++index) {
        std::error_code error;
        std::filesystem::remove(files.at(index).path(), error);
    }
}

} // namespace pn::platform
