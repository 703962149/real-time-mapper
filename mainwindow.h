#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QMutex>
#include <QFileDialog>
#include <QLabel>
#include <sys/time.h>
#include <opencv/highgui.h>

#include "calibrationparameters.h"
#include "stereoimage.h"
#include "readlocalfilethread.h"
#include "reconstructionthread.h"
#include "modelpainting.h"
#include "getcameraposethread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    Ui::MainWindow *ui;
    CalibrationParameters *m_calib;
    StereoImage *m_stereo;
    ReadLocalFileThread *m_readLocalFileThread;
    GetCameraPoseThread *m_getPoseThread;
    ReconstructionThread *m_3dReconstructionThread;

    bool m_readingLocalFile;

private slots:

    void on_BackgroundCheckBox_clicked();
    void on_ShowCameraCheckBox_clicked();
    void on_FollowCameraCheckBox_clicked();
    void on_AddPoseButton_clicked();
    void on_DeletePoseButton_clicked();
    void on_ShowPosesButton_clicked();
    void on_SaveVideoButton_clicked();
    void on_LocalImageModeButton_clicked();
    void on_RealTimeModeButton_clicked();
    void on_ExitButton_clicked();

    void DetectNewCalibParam();
    void DetectNewStereoImage();
    void DetectNewCameraPose();
    void DetectNewMap();

};

#endif // MAINWINDOW_H