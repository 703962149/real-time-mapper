#include "mainwindow.h"
#include "ui_mainwindow.h"

//constructer: initialize all pointers.
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_calib = new CalibrationParameters();
    m_stereo = new StereoImage();
    m_readLocalFileThread = new ReadLocalFileThread(m_calib,m_stereo);
    m_getPoseThread = new GetCameraPoseThread(m_calib,m_stereo);
    m_3dReconstructionThread = new ReconstructionThread(m_calib,m_stereo);

    QObject::connect(m_stereo,SIGNAL(DetectNewStereoImage()),this,SLOT(DetectNewStereoImage()));
    QObject::connect(m_calib,SIGNAL(DetectNewCalibParam()),this,SLOT(DetectNewCalibParam()));

    m_readingLocalFile = false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_calib;
    delete m_stereo;
    delete m_readLocalFileThread;
    delete m_getPoseThread;
    delete m_3dReconstructionThread;
}

void MainWindow::on_BackgroundCheckBox_clicked()
{
    ui->Model3D->SetBackgroundFlag(ui->BackgroundCheckBox->isChecked());
}

void MainWindow::on_ShowCameraCheckBox_clicked()
{
    ui->Model3D->SetShowCamerasFlag(ui->ShowCameraCheckBox->isChecked());
}

void MainWindow::on_FollowCameraCheckBox_clicked()
{
    ui->Model3D->SetFollowCameraFlag(ui->FollowCameraCheckBox->isChecked());
}

void MainWindow::on_AddPoseButton_clicked()
{
    ui->Model3D->AddPose();
}

void MainWindow::on_DeletePoseButton_clicked()
{
    ui->Model3D->DelPose();
}

void MainWindow::on_ShowPosesButton_clicked()
{
    ui->Model3D->PlayPoses();
}

void MainWindow::on_SaveVideoButton_clicked()
{
    //TODO: the funtion to save those added poses as a video.
    std::cout<< "TODO: SaveVideo " << std::endl;
}

void MainWindow::on_LocalImageModeButton_clicked()
{
    if (m_readingLocalFile)
    {
        // set button text
        ui->LocalImageModeButton->setText("LocalImageMode");

        std::cout<<"Stop mapping."<<std::endl;

        // stop stereo scanning
        m_readingLocalFile = false;

        // terminate all processes
        m_readLocalFileThread->terminate();
        m_getPoseThread->terminate();
        m_3dReconstructionThread->terminate();
        while (m_readLocalFileThread->isRunning()||m_getPoseThread->isRunning()||m_3dReconstructionThread->isRunning());

    // starting ...
    }
    else
    {
        // set button text
        ui->LocalImageModeButton->setText("Stop");
        std::cout<<"Begin to read local file, please choose a directory."<<std::endl;

        // reset everything
        ui->Model3D->clearAll();
        ui->Model3D->ResetView();

        // start reading from files
        if (m_readLocalFileThread->isRunning())
        {
            m_readLocalFileThread->terminate();
        }

        QString inputDirectroy = QFileDialog::getExistingDirectory (this, tr("Choose Dataset Directory"), "../", QFileDialog::ShowDirsOnly);

        if(m_readLocalFileThread->SetInputDirectory(inputDirectroy))
        {
            m_readLocalFileThread->start();
            // start stereo scanning
            m_readingLocalFile = true;
        }
        else
        {
            ui->LocalImageModeButton->setText("LocalImageMode");
        }
    }

}

void MainWindow::on_RealTimeModeButton_clicked()
{
    std::cout<< "TODO: RealTimeMode " << std::endl;
}

void MainWindow::on_ExitButton_clicked()
{
    std::cout<< "Exit mapper " << std::endl;
    exit(0);
}

void MainWindow::DetectNewCalibParam()
{
    m_stereo->clear();

    if( m_getPoseThread->isRunning() )
    {
        m_getPoseThread->quit();
        while( m_getPoseThread->isRunning() );
    }

    QObject::disconnect(m_getPoseThread,SIGNAL(DetectNewCameraPose()),this,SLOT(DetectNewCameraPose()));
    delete m_getPoseThread;
    m_getPoseThread = 0;

    if( m_3dReconstructionThread->isRunning() )
    {
        m_3dReconstructionThread->quit();
        while( m_3dReconstructionThread->isRunning() );
    }

    QObject::disconnect(m_3dReconstructionThread,SIGNAL(DetectNewMap()),this,SLOT(DetectNewMap()));
    delete m_3dReconstructionThread;
    m_3dReconstructionThread = 0;

    m_getPoseThread = new GetCameraPoseThread(m_calib,m_stereo);
    m_3dReconstructionThread = new ReconstructionThread(m_calib,m_stereo);

    // connect to the objects for communication.
    QObject::connect(m_getPoseThread,SIGNAL(DetectNewCameraPose()),this,SLOT(DetectNewCameraPose()));
    QObject::connect(m_3dReconstructionThread,SIGNAL(DetectNewMap()),this,SLOT(DetectNewMap()));

    m_calib->PickedUpNewCalibParam();
}

void MainWindow::DetectNewStereoImage()
{
    //Show the current left image
    QImage newImg = m_stereo->m_displayLeftImage.scaled(ui->LeftImageLabel->width(), ui->LeftImageLabel->height());
    ui->LeftImageLabel->setPixmap(QPixmap::fromImage(newImg));
    //ui->RightImageLabel->setPixmap(QPixmap::fromImage(m_stereo->m_displayRightImage));

    //Show the status bar message
    //TODO: let the statusbar show more information
    QString msg = "The number of reading frame is:   " + QString::number(m_stereo->m_frameindex+1, 10) + m_readLocalFileThread->m_runningDataset;
    ui->statusBar->showMessage(msg);

    //Get the current stereo image
    StereoImage::StereoImageParameters stereoImg(*m_stereo->GetCurrStereoImage());

    //flag to read next stereo image
    m_stereo->PickUpNewStereo();

    //make sure the thread finished the last action
    if (m_readingLocalFile && !m_getPoseThread->isRunning())
    {
        //send image pointer and the pose file status to the POSE(VO) thread
        m_getPoseThread->SendImage(stereoImg, m_readLocalFileThread->CheckPoseFileStatus());
        m_getPoseThread->start();
    }

}

void MainWindow::DetectNewCameraPose()
{
    //get image and pose from the VO thread
    StereoImage::StereoImageParameters stereoImgWithPose(*m_getPoseThread->GetStereoImage());
    Matrix Homographytotal = m_getPoseThread->GetHomographyTotal();
    m_getPoseThread->PickUpNewCamPose();

    //make sure the thread finished the last action
    if (m_readingLocalFile && !m_3dReconstructionThread->isRunning())
    {
        m_3dReconstructionThread->SendImageAndPose(stereoImgWithPose, Homographytotal);
        m_3dReconstructionThread->start();
    }
}

void MainWindow::DetectNewMap()
{
    ui->Model3D->AddCamera(m_3dReconstructionThread->GetHomographyTotal(),0.1);
    ui->Model3D->AddPoints(m_3dReconstructionThread->GetMapPoints());
    ui->Model3D->FollowCamera(m_3dReconstructionThread->GetHomographyTotal());

    m_3dReconstructionThread->PickupNewMap();
}
