#include "ModuleTestDispatcher.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QTextStream>

namespace {

QByteArray compact(const QJsonObject &object)
{
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QJsonObject parseAndDispatch(const QByteArray &line)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(line, &error);
    if (error.error != QJsonParseError::NoError) {
        return {{"ok", false},
                {"error", QString("invalid_json: %1").arg(error.errorString())}};
    }
    if (!document.isObject())
        return {{"ok", false}, {"error", "request_must_be_a_json_object"}};
    return ModuleTestDispatcher::dispatch(document.object());
}

void writeJson(QTextStream &out, const QJsonObject &object)
{
    out << QString::fromUtf8(compact(object)) << '\n';
    out.flush();
}

int verifyFile(const QString &path, QTextStream &out)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        writeJson(out, {{"ok", false}, {"error", "cannot_open_case_file"}, {"path", path}});
        return 2;
    }

    int passed = 0;
    int failed = 0;
    int lineNumber = 0;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            ++failed;
            writeJson(out, {{"ok", false}, {"line", lineNumber}, {"error", "invalid_case_json"}});
            continue;
        }

        const QJsonObject testCase = document.object();
        const QJsonObject request = testCase.value("request").toObject();
        const QJsonObject expected = testCase.value("expect").toObject();
        const QJsonObject actual = ModuleTestDispatcher::dispatch(request);
        if (actual == expected) {
            ++passed;
            continue;
        }

        ++failed;
        writeJson(out, {{"ok", false},
                        {"line", lineNumber},
                        {"name", testCase.value("name").toString()},
                        {"expected", expected},
                        {"actual", actual}});
    }

    writeJson(out, {{"ok", failed == 0}, {"passed", passed}, {"failed", failed}});
    return failed == 0 ? 0 : 1;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("ModuleTestCli");
    QCoreApplication::setApplicationName("IsolatedTestRun");

    QTemporaryDir settingsDirectory;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());

    QTextStream out(stdout);
    const QStringList arguments = app.arguments();
    if (arguments.size() == 2 && arguments.at(1) == "--describe") {
        writeJson(out, ModuleTestDispatcher::describe());
        return 0;
    }
    if (arguments.size() == 3 && arguments.at(1) == "--request") {
        writeJson(out, parseAndDispatch(arguments.at(2).toUtf8()));
        return 0;
    }
    if (arguments.size() == 3 && arguments.at(1) == "--verify")
        return verifyFile(arguments.at(2), out);
    if (arguments.size() != 1) {
        writeJson(out, {{"ok", false},
                        {"error", "usage: module_test_cli [--describe | --request JSON | --verify FILE]"}});
        return 2;
    }

    QFile input;
    if (!input.open(stdin, QIODevice::ReadOnly | QIODevice::Text)) {
        writeJson(out, {{"ok", false}, {"error", "cannot_open_stdin"}});
        return 2;
    }
    while (!input.atEnd()) {
        const QByteArray line = input.readLine().trimmed();
        if (!line.isEmpty())
            writeJson(out, parseAndDispatch(line));
    }
    return 0;
}
