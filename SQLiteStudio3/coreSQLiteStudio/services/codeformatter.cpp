#include "codeformatter.h"
#include "parser/parser.h"
#include "plugins/codeformatterplugin.h"
#include "services/pluginmanager.h"
#include <QDebug>

void CodeFormatter::setFormatter(const QString& lang, CodeFormatterPlugin *formatterPlugin)
{
    currentFormatter[lang] = formatterPlugin;
}

CodeFormatterPlugin* CodeFormatter::getFormatter(const QString& lang)
{
    if (hasFormatter(lang))
        return currentFormatter[lang];

    return nullptr;
}

bool CodeFormatter::hasFormatter(const QString& lang)
{
    return currentFormatter.contains(lang);
}

void CodeFormatter::fullUpdate()
{
    availableFormatters.clear();
    QList<CodeFormatterPlugin*> formatterPlugins = PLUGINS->getLoadedPlugins<CodeFormatterPlugin>();
    for (CodeFormatterPlugin* plugin : formatterPlugins)
        availableFormatters[plugin->getLanguage()][plugin->getName()] = plugin;

    updateCurrent();
}

void CodeFormatter::updateCurrent()
{
    if (modifyingConfig)
        return;

    modifyingConfig = true;

    bool modified = false;
    currentFormatter.clear();
    QHash<QString,QVariant> config = CFG_CORE.General.ActiveCodeFormatter.get();
    QString name;
    QStringList names = availableFormatters.keys();
    qSort(names);
    for (const QString& lang : names)
    {
        name = config[lang].toString();
        if (config.contains(lang) && availableFormatters[lang].contains(name))
        {
            currentFormatter[lang] = availableFormatters[lang][name];
        }
        else
        {
            currentFormatter[lang] = availableFormatters[lang].begin().value();
            config[lang] = currentFormatter[lang]->getName();
            modified = true;
        }
    }

    if (modified)
        CFG_CORE.General.ActiveCodeFormatter.set(config);

    modifyingConfig = false;
}

QString CodeFormatter::format(const QString& lang, const QString& code, Db* contextDb)
{
    if (!hasFormatter(lang))
    {
        qWarning() << "No formatter plugin defined for CodeFormatter for language:" << lang;
        return code;
    }

    return currentFormatter[lang]->format(code, contextDb);
}