#include "LVRScanProjectOpenDialog.hpp"

namespace lvr2
{

LVRScanProjectOpenDialog::LVRScanProjectOpenDialog(QWidget* parent): 
    QDialog(parent),
    m_schema(nullptr),
    m_kernel(nullptr),
    m_projectType(NONE),
    m_projectScale(m),
    m_reductionPtr(nullptr),
    m_successful(false)
{
    m_parent = parent;
    m_ui = new LVRScanProjectOpenDialogUI;
    m_ui->setupUi(this);
    this->setFixedSize(this->size().width(), this->size().height());
    m_reductionPtr = ReductionAlgorithmPtr(new NoReductionAlgorithm());
    
    // displays the default reduction type when dialog is opened
    m_ui->pushButtonReduction->setText("Complete Point Buffer");

    initAvailableScales();
    connectSignalsAndSlots();
}

void LVRScanProjectOpenDialog::connectSignalsAndSlots()
{
    QObject::connect(m_ui->toolButtonPath, SIGNAL(pressed()), this, SLOT(openPathDialog()));    
    QObject::connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(acceptOpen()));
    QObject::connect(m_ui->comboBoxSchema, SIGNAL(currentIndexChanged(int)), this, SLOT(schemaSelectionChanged(int)));
    QObject::connect(m_ui->comboBoxScale, SIGNAL(currentIndexChanged(int)), this, SLOT(projectScaleSelectionChanged(int)));
    QObject::connect(m_ui->comboBoxProjectType, SIGNAL(currentIndexChanged(int)), this, SLOT(projectTypeSelectionChanged(int)));
    QObject::connect(m_ui->pushButtonReduction, SIGNAL(pressed()), this, SLOT(openReductionDialog()));
}

void LVRScanProjectOpenDialog::acceptOpen()
{
    m_successful = true;
}

bool LVRScanProjectOpenDialog::successful()
{
    return m_successful;
}

void LVRScanProjectOpenDialog::projectTypeSelectionChanged(int index)
{
    // clear dialog because all previously made selections
    // may be invalid with the change of the project type
    m_ui->lineEditPath->clear();
    m_ui->toolButtonPath->setDown(false);
    m_projectType = NONE;
    updateAvailableSchemas();
}

void LVRScanProjectOpenDialog::schemaSelectionChanged(int index)
{
    if(index != -1 && m_kernel)
    {
        switch(m_projectType)
        {
            case DIR:
                updateDirectorySchema(index);
                break;
            case HDF5:
                updateHDF5Schema(index);
                break;
            default:
                break;
        }
    }
}

void LVRScanProjectOpenDialog::projectScaleSelectionChanged(int index)
{
    // project scale will affect how big the scanner position cylinder will be rendered
    switch(index)
    {
        case 0:
            m_projectScale = mm;
            break;
        case 1:
            m_projectScale = cm;
            break;
        case 2:
            m_projectScale = m;
            break;
        default:
            break;
    }
}

void LVRScanProjectOpenDialog::updateDirectorySchema(int index)
{
    switch(index)
    {
        case 0:
            m_schema = ScanProjectSchemaPtr(new ScanProjectSchemaRaw(m_kernel->fileResource()));
            break;
        case 1:
            m_schema = ScanProjectSchemaPtr(new ScanProjectSchemaRawPly(m_kernel->fileResource()));
            break;
        case 2:
            m_schema = ScanProjectSchemaPtr(new ScanProjectSchemaSlam6D(m_kernel->fileResource()));
            break;
        case 3:
            m_schema = ScanProjectSchemaPtr(new ScanProjectSchemaEuRoC(m_kernel->fileResource()));
            break;
    }
}
void LVRScanProjectOpenDialog::updateHDF5Schema(int index)
{
    switch(index)
    {
        case 0:
            m_schema = ScanProjectSchemaPtr(new ScanProjectSchemaHDF5());
            break;
    }
}

void LVRScanProjectOpenDialog::initAvailableScales()
{
    // Clear all items in combo box
    QComboBox* b = m_ui->comboBoxScale;
    b->clear();
    // add available items to combobox
    b->addItem("mm");
    b->addItem("cm");
    b->addItem("m");
    // set meters as default
    b->setCurrentIndex(2);
}

void LVRScanProjectOpenDialog::updateAvailableSchemas()
{
    // Clear all schema items in combo box
    QComboBox* b = m_ui->comboBoxSchema;
    b->clear();

    // Check if kernel exists, than the current UI 
    // should be consistent. Otherwise, do not 
    // offer any schema
    if(!m_kernel)
    {
        b->addItem("None");
        if(m_schema)
        {
            m_schema.reset();
        }
        return;
    }

    // Add available schemas depending on selected 
    // project type
    switch(m_projectType)
    {
        case NONE:
            b->addItem("None");
            break;
        case HDF5:
            b->addItem("HDF5 Schema V2");
            break;
        case DIR:
            b->addItem("RAW");
            b->addItem("RAW PLY");
            b->addItem("SLAM 6D");
            b->addItem("EuRoC");
            break;    
        default:
            b->addItem("None");      
    }
    m_ui->pushButtonReduction->setEnabled(true);
}

void LVRScanProjectOpenDialog::openReductionDialog()
{
    LVRReductionAlgorithmDialog* dialog = new LVRReductionAlgorithmDialog(this);
    
    // Reduction Dialog
    dialog->setModal(true);
    dialog->raise();
    dialog->activateWindow();
    dialog->exec();

    if(!dialog->successful())
    {
        return;
    }

    m_reductionPtr = dialog->reductionPtr();
                    
    switch(dialog->reductionName())
    {
        case 0:
            m_ui->pushButtonReduction->setText("Complete Point Buffer");
            break;
        case 1:
            m_ui->pushButtonReduction->setText("Meta Only");
            break;
        case 2:
            m_ui->pushButtonReduction->setText("Octree Reduction");
            break;   
        case 3:
            m_ui->pushButtonReduction->setText("Fixed Size");
            break;
        case 4:
            m_ui->pushButtonReduction->setText("Percentage");
            break;
        default:
            break;     
    }
}

void LVRScanProjectOpenDialog::openPathDialog()
{   
    // Directory mode
    if(m_ui->comboBoxProjectType->currentIndex() == 0)
    {
        // Set dialog properties
        QFileDialog dialog(m_parent);
        dialog.setFileMode(QFileDialog::DirectoryOnly);

        // Get selected directory
        QStringList fileNames;
        if (dialog.exec())
        {
            fileNames = dialog.selectedFiles();
            if(fileNames.count() > 0)
            {
                QString selectedDir = fileNames[0];
                m_ui->lineEditPath->setText(selectedDir);

                // Check if path is valid
                QFileInfo info(selectedDir);
                if(info.exists())
                {
                    // Update project type flag
                    m_projectType = DIR;

                    // Update kernel
                    m_kernel = FileKernelPtr(new DirectoryKernel(selectedDir.toStdString()));
                }
            }
        }
    }
    // HDF5 mode
    else if(m_ui->comboBoxProjectType->currentIndex() == 1)
    {
        // Set dialog properties
        QFileDialog dialog(m_parent);
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setNameFilter(tr("HDF5 Files (*.h5 *.hdf5)"));
        
        // Get selected file
        QStringList fileNames;
        if (dialog.exec())
        {
            fileNames = dialog.selectedFiles();
            if(fileNames.count() > 0)
            {
                QString selectedFile = fileNames[0];
                m_ui->lineEditPath->setText(selectedFile);
             
                // Check if path is valid
                QFileInfo info(selectedFile);
                if(info.exists())
                {
                    // Update project type flag
                    m_projectType = HDF5;

                    // Update kernelelete m_kernel;
                    m_kernel = FileKernelPtr(new HDF5Kernel(selectedFile.toStdString()));
                }
            }
        }
    }
    else
    {
        std::cout << "LVRScanProjectDialog: Unknown item index: "
                  << m_ui->comboBoxProjectType->currentIndex() << std::endl;
    }

    // Update list of available schemas depending
    // on selected project type 
    updateAvailableSchemas();
}

} // namespace lvr2
