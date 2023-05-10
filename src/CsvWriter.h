#pragma once

#include <WriterPlugin.h>

#include <Dataset.h>

using namespace hdps::plugin;
using namespace hdps::gui;
using namespace hdps::util;



/**
 * Csv Exporter plugin class
 *
 * @author Jeroen Eggermont
 */

class CsvWriter : public WriterPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param factory Pointer to the plugin factory
     */
    CsvWriter(const PluginFactory* factory);

    /** Destructor */
    ~CsvWriter() override = default;

    /** Initialization is called when the plugin is first instantiated. */
    void init() override {};

    /** Saved the data */
    void writeData() override;

    /** calls saveData */
    
};

/// =============================================================================
// Factory
// =============================================================================

class CsvWriterFactory : public WriterPluginFactory
{
    Q_INTERFACES(hdps::plugin::WriterPluginFactory hdps::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.lumc.ExtCsvExporter"
            FILE  "CsvWriter.json")

public:
    CsvWriterFactory(void) {}
    ~CsvWriterFactory(void) override {}

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    WriterPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const hdps::Datasets& datasets) const override;
};