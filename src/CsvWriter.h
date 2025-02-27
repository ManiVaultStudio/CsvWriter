#pragma once

#include <WriterPlugin.h>

#include <Dataset.h>

using namespace mv::plugin;
using namespace mv::gui;
using namespace mv::util;



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
    Q_INTERFACES(mv::plugin::WriterPluginFactory mv::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.lumc.CsvWriter"
            FILE  "CsvWriter.json")

public:
    CsvWriterFactory();
    ~CsvWriterFactory(void) override {}

    WriterPlugin* produce() override;

    mv::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};