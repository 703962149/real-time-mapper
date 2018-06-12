#ifndef GETCAMERAPOSETHREAD_H
#define GETCAMERAPOSETHREAD_H

#include <QObject>
#include <QThread>
#include <math.h>

#include "calibrationparameters.h"
#include "stereoimage.h"
#include "readlocalfilethread.h"

#include "libviso2/src/viso_stereo.h"
#include "libviso2/src/timer.h"

class GetCameraPoseThread : public QThread
{
    Q_OBJECT

public:

    GetCameraPoseThread(CalibrationParameters *calib, StereoImage *stereo, QObject *parent = 0);
    ~GetCameraPoseThread();

    StereoImage::StereoImageParameters* GetStereoImage() { return m_stereoImage; }
    Matrix GetHomographyTotal() { return m_HomographyMatrix; }
    void ResetHomographyTotal() { m_HomographyMatrix.eye(); }
    void PickUpNewCamPose() { m_pickedNewCamPose = true; }

    //Send image from the mainwindow thread to this vo thread.
    void SendImage(StereoImage::StereoImageParameters &s, bool poseFileStatus);

    //The Homography matrix here is from the first coordination to the current coordination
    Matrix m_HomographyMatrix;

protected:

    void run();

private:

    CalibrationParameters *m_calib;
    StereoImage *m_stereo;
    StereoImage::StereoImageParameters *m_stereoImage;
    VisualOdometryStereo*  m_visualOdomStereo;

    bool m_poseFileStatus;
    bool m_pickedNewCamPose;

signals:

    void DetectNewCameraPose();

public slots:
};

#endif // GETCAMERAPOSETHREAD_H