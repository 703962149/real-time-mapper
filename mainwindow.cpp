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
    m_realTimeCamThread = new RealTimeCamThread(m_stereo);

    QObject::connect(m_stereo,SIGNAL(DetectNewStereoImage()),this,SLOT(DetectNewStereoImage()));
    QObject::connect(m_calib,SIGNAL(DetectNewCalibParam()),this,SLOT(DetectNewCalibParam()));
    QObject::connect(m_realTimeCamThread,SIGNAL(DetectNewCamImage()),this,SLOT(DetectNewCamImage()));

    m_readingLocalFileFlag = false;
    m_runningRealTimeModeFlag = false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_calib;
    delete m_stereo;
    delete m_readLocalFileThread;
    delete m_getPoseThread;
    delete m_3dReconstructionThread;
    delete m_realTimeCamThread;
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
    if(m_runningRealTimeModeFlag)
    {
        std::cout<<"Please stop other mode first."<<std::endl;
        return;
    }
    if (m_readingLocalFileFlag)
    {
        // set button text
        ui->LocalImageModeButton->setText("LocalImageMode");

        std::cout<<"Stop mapping."<<std::endl;

        // stop stereo scanning
        m_readingLocalFileFlag = false;

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
        std::cout<<"Begin to read local file, please choose a dataset directory."<<std::endl;

        std::cout<<"TODO:Fix the existing memory problem, something does not be released when the program running."<<std::endl;

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
            m_readingLocalFileFlag = true;
        }
        else
        {
            ui->LocalImageModeButton->setText("LocalImageMode");
        }
    }
}

void MainWindow::on_RealTimeModeButton_clicked()
{
    if(m_readingLocalFileFlag)
    {
        std::cout<<"Please stop other mode first."<<std::endl;
        return;
    }
    if (m_runningRealTimeModeFlag)
    {
        m_runningRealTimeModeFlag = false;
    }
    else
    {
        ui->tabWidget->setCurrentIndex(1);
        //TODO: fix this problem
        std::cout<< "Have to use RealTimeMode without using other mode first." << std::endl;

        // set button text
        ui->RealTimeModeButton->setText("Stop");
        std::cout<<"Begin the real time mode, please choose the setting directory."<<std::endl;
        std::cout<<"TODO:Fix the existing memory problem, something does not be released when the program running."<<std::endl;

        //QString settingDirectory = QFileDialog::getExistingDirectory (this, tr("Choose camera setting Directory"), "../", QFileDialog::ShowDirsOnly);
        /*
        if(m_realTimeCamThread->SetInputDirectory(settingDirectory))
        {
            //have to start getting camera images first, if get calib params before getting camera images, the mapper will crash
            m_realTimeCamThread->start();
            m_runningRealTimeModeFlag = true;
        }
        else
        {
            std::cout<<"Can not get the right directory."<<std::endl;
            ui->RealTimeModeButton->setText("RealTimeMode");
        }
        */

        //have to start getting camera images first, if get calib params before getting camera images, the mapper will crash
        m_realTimeCamThread->start();
        m_runningRealTimeModeFlag = true;
    }
}

void MainWindow::on_ReadCalibOfRealTimeModeButton_clicked()
{
    if(!m_realTimeCamThread->isRunning())
    {
        std::cout<<"Please get the camera images first, click the RealTimeMode button."<<std::endl;
    }
    else
    {
        //TODO
        m_realTimeCamThread->GetLoitorCalibParam("/home/dadaoii/camtest/StereoCalib.yml");
        //m_realTimeCamThread->GetLoitorCalibParam("/home/dadaoii/orbslam2/Examples/ROS/ORB_SLAM2/EuRoC.yaml");
        m_realTimeCamThread->SetRemapFlag();

        m_calib->GetLoitorCalibParam("/home/dadaoii/camtest/StereoCalib.yml");
        //m_calib->GetLoitorCalibParam("/home/dadaoii/orbslam2/Examples/ROS/ORB_SLAM2/EuRoC.yaml");


        //QStringList arguments = qApp->arguments();
        //m_slam = new ORB_SLAM2::System(qPrintable(arguments.at(1)),qPrintable(arguments.at(2)),ORB_SLAM2::System::STEREO,true);
        //m_realTimeCamThread->GetORBSLAM(m_slam);
    }
}

void MainWindow::on_ReconstructionButton_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
    if(!m_realTimeCamThread->isRunning())
    {
        std::cout<<"Please get the camera images first, click the RealTimeMode button."<<std::endl;
    }
    else if(!m_realTimeCamThread->CheckCalibFlagStatus())
    {
        std::cout<<"Please get the camera calibration parameters first."<<std::endl;
    }
    else
    {
        m_realTimeCamThread->SetReconstructionFlag();
    }
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
    if (m_readingLocalFileFlag)
    {
        //Show the current left image
        QImage newImg = m_stereo->m_displayLeftImage.scaled(ui->LeftImageLabel->width(), ui->LeftImageLabel->height());
        ui->LeftImageLabel->setPixmap(QPixmap::fromImage(newImg));

        //Show the status bar message
        //TODO: let the statusbar show more information
        QString msg = "The number of reading frame is:   " + QString::number(m_stereo->m_frameindex+1, 10) + m_readLocalFileThread->m_runningDataset;
        ui->statusBar->showMessage(msg);

    }
    else if (m_runningRealTimeModeFlag)
    {
        QImage rawImgL = m_realTimeCamThread->m_displayLeftImage.scaled(ui->LeftImageLabel->width(),ui->LeftImageLabel->height());
        ui->LeftImageLabel->setPixmap(QPixmap::fromImage(rawImgL));

        QString msg = "Now running real time mode.";
        ui->statusBar->showMessage(msg);
    }

    //Get the current stereo image
    StereoImage::StereoImageParameters stereoImg(*m_stereo->GetCurrStereoImage());

    //flag to read next stereo image
    m_stereo->PickUpNewStereo();

    //make sure the thread finished the last action
    if ((m_readingLocalFileFlag || m_runningRealTimeModeFlag) && !m_getPoseThread->isRunning())
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
    libviso2_Matrix Homographytotal = m_getPoseThread->GetHomographyTotal();
    m_getPoseThread->PickUpNewCamPose();

    //make sure the thread finished the last action
    if ((m_readingLocalFileFlag || m_runningRealTimeModeFlag) && !m_3dReconstructionThread->isRunning())
    {
        m_3dReconstructionThread->SendImageAndPose(stereoImgWithPose, Homographytotal);
        m_3dReconstructionThread->start();
    }
}

void MainWindow::DetectNewMap()
{
    if(ui->tabWidget->currentIndex()==0)
    {
        ui->Model3D->AddCamera(m_3dReconstructionThread->GetHomographyTotal(),0.1);
        ui->Model3D->AddPoints(m_3dReconstructionThread->GetMapPoints());
        ui->Model3D->FollowCamera(m_3dReconstructionThread->GetHomographyTotal());
    }

    m_3dReconstructionThread->PickupNewMap();
}

void MainWindow::DetectNewCamImage()
{
    if(ui->tabWidget->currentIndex()==1)
    {
        QImage rawImgL = m_realTimeCamThread->m_displayLeftImage.scaled(ui->rawLeftImageLabel->width(),ui->rawLeftImageLabel->height());
        QImage rawImgR = m_realTimeCamThread->m_displayRightImage.scaled(ui->rawRightImageLabel->width(),ui->rawRightImageLabel->height());
        ui->rawLeftImageLabel->setPixmap(QPixmap::fromImage(rawImgL));
        ui->rawRightImageLabel->setPixmap(QPixmap::fromImage(rawImgR));
        if(m_realTimeCamThread->CheckRemapFlagStatus())
        {
            QImage rImgL = m_realTimeCamThread->m_remapLeftImage.scaled(ui->remapLeftLabel->width(),ui->remapLeftLabel->height());
            QImage rImgR = m_realTimeCamThread->m_remapRightImage.scaled(ui->remapRightLabel->width(),ui->remapRightLabel->height());
            ui->remapLeftLabel->setPixmap(QPixmap::fromImage(rImgL));
            ui->remapRightLabel->setPixmap(QPixmap::fromImage(rImgR));
        }
    }

    m_realTimeCamThread->PickedUpNewCamImage();
}
