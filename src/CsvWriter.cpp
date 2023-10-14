#include "CsvWriter.h"

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <actions/PluginTriggerAction.h>

#include <cmath>

#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

Q_PLUGIN_METADATA(IID "nl.BioVault.CsvSaverPlugin")

using namespace hdps;


CsvWriter::CsvWriter(const PluginFactory* factory) :
    WriterPlugin(factory)
{
}

void CsvWriter::writeData()
{

    auto inputDataset = getInputDataset<DatasetImpl>();
    if (!inputDataset.isValid())
        return;
    // Let the user chose the save path
    QSettings settings(QLatin1String{ "HDPS" }, QLatin1String{ "Plugins/" } + getKind());
    const QLatin1String directoryPathKey("directoryPath");
    const auto directoryPath = settings.value(directoryPathKey).toString() + "/";
    
    QString fileName = QFileDialog::getSaveFileName(
        nullptr, tr("Save data set"), directoryPath + inputDataset->getGuiName() + ".csv", tr("CSV file (*.csv);;All Files (*)"));

    // Only continue when the dialog has not been not canceled and the file name is non-empty.
    if (fileName.isNull() || fileName.isEmpty())
    {
        qDebug() << "CsvWriter: No data written to disk - File name empty";
        return;
    }
    else
    {
        // store the directory name
        settings.setValue(directoryPathKey, QFileInfo(fileName).absolutePath());
    }

    QVariantList sampleNames;
    // search for sampleNames
    {
        Dataset<DatasetImpl> d = inputDataset;
        while(d.isValid() && sampleNames.isEmpty())
        {
            if (d->hasProperty("Sample Names"))
                sampleNames = d->getProperty("Sample Names").toList();
            else
                d = d->getParent();
        }
    }
   

    
    

    
	if (inputDataset->getDataType() == PointType)
    {
		auto points = Dataset<Points>(inputDataset);
        // handle points dataset
        points->setLocked(true);
        {
        	QApplication::processEvents();

            auto& task = points->getTask();

            task.setName("Saving");
            task.setRunning();
            task.setProgressDescription(QString("Saving data"));


            QFile file(fileName);
            if (!file.open(QFile::WriteOnly | QFile::Truncate))
                return;
            QTextStream output(&file);
            QChar seperatorChar = ',';
            const QString replaceChar = "_";
            std::map<QString, QVariantList> dimensionProperties;
            auto dimensionNames = points->getDimensionNames();
           
            if (sampleNames.size() !=  points->getNumPoints())
                sampleNames.clear();

            QStringList propertyNames = points->propertyNames();
            for (auto propertyName : propertyNames)
            {
                QVariantList property_list = points->getProperty(propertyName).toList();
                if (propertyName == "Sample Names")
                {
                    if (property_list.size() == points->getNumPoints())
                        sampleNames = property_list;
                }
                else if (property_list.size() == points->getNumDimensions())
                {
                    dimensionProperties[propertyName] = property_list;;
                }
            }

            if(output.status() == QTextStream::Ok)
            {
                // first print header
                if(dimensionNames.size() && sampleNames.size())
                {
                    output << "" << seperatorChar;
                }

               
                if(dimensionNames.size())
                {
                    bool first = true;
	                for(auto name : dimensionNames)
	                {
                        if (!first)
                            output << seperatorChar;
                        output << name.replace(seperatorChar, replaceChar);
                        first = false;
	                }
                }

                output << "\n";
                points->visitData([this, &output, &points, &dimensionNames, &sampleNames, seperatorChar, replaceChar, &task](auto pointData) {
                    std::uint64_t pointIndex = 0;

                    for (auto point : pointData) {

                        if (sampleNames.size())
                            output << sampleNames[pointIndex].toString().replace(seperatorChar,replaceChar) << seperatorChar;
                        for (std::int32_t dimensionIndex = 0; dimensionIndex < points->getNumDimensions(); dimensionIndex++) {
                            if (dimensionIndex > 0)
                                output << seperatorChar;
                            output << point[dimensionIndex];
                        }


                        output << "\n";
                        ++pointIndex;

                        if (pointIndex % 10 == 0) {
                            task.setProgress(static_cast<float>(pointIndex) / static_cast<float>(points->getNumPoints()));

                            QApplication::processEvents();
                        }
                    }
                    });

                output.flush();
                file.close();
                task.setProgress(1.0f);
                task.setFinished();
            }

            if(dimensionProperties.size())
            {
                QFileInfo fileInfo(fileName);
                QString propertiesFileName = fileInfo.absolutePath() + "/" + fileInfo.baseName() + "_properties." + fileInfo.completeSuffix();
                if (QFileInfo::exists(propertiesFileName))
                {
                    QFileInfo propertiesFileInfo(propertiesFileName);
                    if (!propertiesFileInfo.isWritable())
                        propertiesFileName.clear();
                }
                if (propertiesFileName.size())
                {

                    QFile propertiesFile(propertiesFileName);
                    if (!propertiesFile.open(QFile::WriteOnly | QFile::Truncate))
                        return;
                    QTextStream output_properties(&propertiesFile);

                    if (output_properties.status() == QTextStream::Ok)
                    {
                        // write header
                        output_properties << "";
                        for(auto dimensionName : dimensionNames)
                        {
                            output_properties << seperatorChar << dimensionName.replace(seperatorChar, replaceChar);
                        }
                        output_properties << "\n";

                        //write data
                        for(auto p : dimensionProperties)
                        {
                            output_properties << p.first;
                            for(auto item : p.second)
                            {
                                output_properties << seperatorChar << item.toString();
                            }
                            output_properties << "\n";
                        }
                    }

                    output_properties.flush();
                    propertiesFile.close();
                }
            }

            points->setLocked(false);
            }
    }
    else if (inputDataset->getDataType() == ClusterType)
    {
		auto clusters = Dataset<Clusters>(inputDataset);
    	struct ClusterInfo
        {
            QString id;
            QString  clusterName;
            QString  clusterColor;
        } ;
		
        
        if(clusters.isValid())
        {
            clusters->setLocked(true);

            QFile file(fileName);
            if (!file.open(QFile::WriteOnly | QFile::Truncate))
                return;
            QTextStream output(&file);

            QChar seperatorChar = ',';
            const QString replaceChar = "_";

            
            if(output.status() == QTextStream::Ok)
            {
                std::vector<ClusterInfo> clusterInfo;
                auto clusterVector = clusters->getClusters();
                for (auto cluster : clusterVector)
                {
                    const auto& indices = cluster.getIndices();
                    for(auto index : indices)
                    {
                        if (index >= clusterInfo.size())
                            clusterInfo.resize(index + 1);
                        clusterInfo[index].id = sampleNames.isEmpty() ? QString::number(index) : sampleNames[index].toString();
                        clusterInfo[index].clusterName = cluster.getName();
                        clusterInfo[index].clusterColor = cluster.getColor().name();
                    }
                }

                if (sampleNames.size() != clusterInfo.size())
                    sampleNames.clear();

                // write header
                if (sampleNames.size())
                {
                    output << "Sample Name" << seperatorChar << "Cluster" << seperatorChar << "Color\n";
                }
                else
                {
                    output << "ID" << seperatorChar << "Cluster" << seperatorChar << "Color\n";
                }
                for (auto info : clusterInfo)
                {
                    output << info.id.replace(seperatorChar, replaceChar) << seperatorChar << info.clusterName.replace(seperatorChar, replaceChar) << seperatorChar << info.clusterColor << "\n";
                }
                output.flush();
                file.close();
            }
           
            clusters->setLocked(false);
        }

    }
    
}
       

// =============================================================================
// Factory
// =============================================================================

WriterPlugin* CsvWriterFactory::produce()
{
    return new CsvWriter(this);
}

QIcon CsvWriterFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return Application::getIconFont("FontAwesome").getIcon("file-csv", color);
}

DataTypes CsvWriterFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

PluginTriggerActions CsvWriterFactory::getPluginTriggerActions(const hdps::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this](const Dataset<Points>& dataset) -> CsvWriter* {
        return dynamic_cast<CsvWriter*>(plugins().requestPlugin(getKind(), { dataset }));
    };


        if (datasets.count() >= 1) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<CsvWriterFactory*>(this), this, "CsvWriter", "Export dataset to CSV file", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                for (auto dataset : datasets)
                    getPluginInstance(dataset);
                });

            pluginTriggerActions << pluginTriggerAction;
        }

    return pluginTriggerActions;
}