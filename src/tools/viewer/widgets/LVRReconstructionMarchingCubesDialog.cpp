#include <QFileDialog>
#include "LVRReconstructionMarchingCubesDialog.hpp"

namespace lvr
{

LVRReconstructViaMarchingCubesDialog::LVRReconstructViaMarchingCubesDialog(LVRPointCloudItem* parent, vtkRenderWindow* window) :
   m_parent(parent), m_renderWindow(window)
{
    // Setup DialogUI and events
    QDialog* dialog = new QDialog(parent->treeWidget());
    m_dialog = new ReconstructViaMarchingCubesDialog;
    m_dialog->setupUi(dialog);

    connectSignalsAndSlots();

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

LVRReconstructViaMarchingCubesDialog::~LVRReconstructViaMarchingCubesDialog()
{
    // TODO Auto-generated destructor stub
}

void LVRReconstructViaMarchingCubesDialog::connectSignalsAndSlots()
{
    QObject::connect(m_dialog->comboBox_pcm, SIGNAL(currentIndexChanged(const QString)), this, SLOT(toggleRANSACcheckBox(const QString)));
    QObject::connect(m_dialog->comboBox_gs, SIGNAL(currentIndexChanged(int)), this, SLOT(switchGridSizeDetermination(int)));
    QObject::connect(m_dialog->buttonBox, SIGNAL(accepted()), this, SLOT(printAllValues()));
}

void LVRReconstructViaMarchingCubesDialog::save()
{

}

void LVRReconstructViaMarchingCubesDialog::toggleRANSACcheckBox(const QString &text)
{
    QCheckBox* ransac_box = m_dialog->checkBox_RANSAC;
    if(text == "PCL")
    {
        ransac_box->setChecked(false);
        ransac_box->setCheckable(false);
    }
    else
    {
        ransac_box->setCheckable(true);
    }
}

void LVRReconstructViaMarchingCubesDialog::switchGridSizeDetermination(int index)
{
    QComboBox* gs_box = m_dialog->comboBox_gs;

    QLabel* label = m_dialog->label_below_gs;
    QSpinBox* spinBox = m_dialog->spinBox_below_gs;

    // TODO: Add reasonable default values
    if(index == 0)
    {
        label->setText("Voxel size");
        spinBox->setMinimum(1);
        spinBox->setMaximum(1000);
        spinBox->setSingleStep(1);
        spinBox->setValue(10);
    }
    else
    {
        label->setText("Number of intersections");
        spinBox->setMinimum(1);
        spinBox->setMaximum(1000);
        spinBox->setSingleStep(1);
        spinBox->setValue(10);
    }
}

void LVRReconstructViaMarchingCubesDialog::printAllValues()
{
    QComboBox* pcm_box = m_dialog->comboBox_pcm;
    string pcm = pcm_box->currentText().toStdString();
    QCheckBox* extrusion_box = m_dialog->checkBox_Extrusion;
    bool extrusion = extrusion_box->isChecked();
    QCheckBox* ransac_box = m_dialog->checkBox_RANSAC;
    bool ransac = ransac_box->isChecked();
    QSpinBox* kn_box = m_dialog->spinBox_kn;
    int kn = kn_box->value();
    QSpinBox* kd_box = m_dialog->spinBox_kd;
    int kd = kd_box->value();
    QSpinBox* ki_box = m_dialog->spinBox_ki;
    int ki = ki_box->value();
    QCheckBox* reNormals_box = m_dialog->checkBox_renormals;
    bool reestimateNormals = reNormals_box->isChecked();
    QComboBox* gridMode_box = m_dialog->comboBox_gs;
    bool useVoxelSize = (gridMode_box->currentIndex() == 0) ? true : false;
    QSpinBox* gridSize_box = m_dialog->spinBox_below_gs;
    int gridSize = gridSize_box->value();
    cout << "PCM: " << pcm << endl;
    cout << "Extrusion enabled? " << extrusion << endl;
    cout << "RANSAC enabled? " << ransac << endl;
    cout << "kn: " << kn << endl;
    cout << "kd: " << kd << endl;
    cout << "ki: " << ki << endl;
    cout << "(re-)estimate normals? " << reestimateNormals << endl;
    cout << "use voxel size? " << useVoxelSize << endl;
    cout << "grid size: " << gridSize << endl;

    PointBufferPtr pc_buffer = m_parent->getPointBuffer();
    psSurface::Ptr surface;

    if(pcm == "STANN" || pcm == "FLANN" || pcm == "NABO")
    {
        akSurface* aks = new akSurface(pc_buffer, pcm, kn, kd, ki);
        surface = psSurface::Ptr(aks);

        if(ransac) aks->useRansac(true);
    }

    surface->setKd(kd);
    surface->setKi(ki);
    surface->setKn(kn);

    if(!surface->pointBuffer()->hasPointNormals()
                    || (surface->pointBuffer()->hasPointNormals() && reestimateNormals))
        surface->calculateSurfaceNormals();

    // Create an empty mesh
    HalfEdgeMesh<cVertex, cNormal> mesh( surface );

    // Create a new reconstruction object
    FastReconstruction<cVertex, cNormal> reconstruction(
            surface,
            gridSize,
            useVoxelSize,
            "MC",
            extrusion);

    // Create mesh
    reconstruction.getMesh(mesh);
    mesh.setClassifier("PlaneSimpsons");
    mesh.getClassifier().setMinRegionSize(10);
    mesh.finalize();

    ModelPtr model(new Model(mesh.meshBuffer()));
    ModelBridgePtr bridge(new LVRModelBridge(model));
}

}
