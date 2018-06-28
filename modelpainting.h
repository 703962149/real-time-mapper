#ifndef MODELPAINTING_H
#define MODELPAINTING_H

#include <GL/glew.h>
#include <QWidget>
#include <GL/glu.h>
#include <GL/glut.h>
#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <deque>
#include <iostream>
#include <math.h>
#include "libviso2/src/timer.h"
#include "libviso2/src/matrix.h"

class ModelPainting : public QGLWidget
{
    Q_OBJECT
public:

    ModelPainting(QWidget *parent = 0);
    ~ModelPainting();

    struct Point3D
    {
        float x,y,z;
        float val;
        Point3D (float x,float y,float z,float val) : x(x),y(y),z(z),val(val) {}
    };

    void clearAll ();

    void AddCamera(libviso2_Matrix HomographyMatrix, float s);
    void AddPoints(std::vector<Point3D> p);
    void FollowCamera(libviso2_Matrix H);

    void SetBackgroundFlag(bool background) { m_backgroundFlag = background; updateGL(); }
    void SetShowCamerasFlag(bool showCamFlag) { m_showCamFlag = showCamFlag; updateGL(); }
    void SetFollowCameraFlag(bool followCamFlag) { m_followCamFlag = followCamFlag; updateGL(); }

    void ResetView();

    void AddPose();
    void DelPose();
    void PlayPoses(bool saveVideo);

    std::string createNewRecordDirectory();

protected:

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:

    struct Camera
    {
        float p[10][3];
    };

    struct Pose
    {
        float  zoom;
        float  rotx,roty;
        float  tx,ty,tz;
    };
    Pose   m_currPose;

    std::deque<Camera> m_cams;
    std::deque<GLuint> m_glList;
    std::vector<Pose> m_poses;

    QPoint m_lastPose;

    bool   m_backgroundFlag;
    bool   m_showCamFlag;
    bool   m_followCamFlag;


signals:

public slots:
};

#endif // MODELPAINTING_H
