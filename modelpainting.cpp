#include "modelpainting.h"

using namespace std;

//DRAWING_POINT_SIZE means the size of the painting points
#define DRAWING_POINT_SIZE 1.5
//LIMIT_NUM_OF_FARME means how much frame would be kept.
//TODO: make it can be changed in the main window.
#define LIMIT_NUM_OF_FARME 5

ModelPainting::ModelPainting(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers),parent)
{
    m_currPose.zoom = -2;
    m_currPose.rotx = 180;
    m_currPose.roty = 0;
    m_currPose.tx   = 0;
    m_currPose.ty   = 0;
    m_currPose.tz   = -1.5;
    m_backgroundFlag = true;
    m_showCamFlag = true;
    m_followCamFlag = false;
}

ModelPainting::~ModelPainting()
{
    clearAll();
}

void ModelPainting::clearAll()
{
    m_cams.clear();
    for (int32_t i=0; i<=(int32_t)m_glList.size(); i++)
    {
        glDeleteLists(m_glList[i],1);
    }
    m_glList.clear();
    m_poses.clear();
}

//=============================================================================//
//===========================Camera and points=================================//
//=============================================================================//

void ModelPainting::AddCamera(Matrix HomographyMatrix, float s)
{
    // Create list with points for this camera
    Matrix C(4,10);
    C._val[0][0] = -0.5*s; C._val[1][0] = -0.5*s; C._val[2][0] = +1.0*s;
    C._val[0][1] = +0.5*s; C._val[1][1] = -0.5*s; C._val[2][1] = +1.0*s;
    C._val[0][2] = +0.5*s; C._val[1][2] = +0.5*s; C._val[2][2] = +1.0*s;
    C._val[0][3] = -0.5*s; C._val[1][3] = +0.5*s; C._val[2][3] = +1.0*s;
    C._val[0][4] = -0.5*s; C._val[1][4] = -0.5*s; C._val[2][4] = +1.0*s;
    C._val[0][5] =      0; C._val[1][5] =      0; C._val[2][5] =      0;
    C._val[0][6] = +0.5*s; C._val[1][6] = -0.5*s; C._val[2][6] = +1.0*s;
    C._val[0][7] = +0.5*s; C._val[1][7] = +0.5*s; C._val[2][7] = +1.0*s;
    C._val[0][8] =      0; C._val[1][8] =      0; C._val[2][8] =      0;
    C._val[0][9] = -0.5*s; C._val[1][9] = +0.5*s; C._val[2][9] = +1.0*s;
    for (int32_t i=0; i<10; i++)
    {
        C._val[3][i] = 1;
    }

    // add camera to list of cameras
    Matrix C_ref = HomographyMatrix*C;
    Camera cam;
    for (int32_t i=0; i<10; i++)
    {
        for (int32_t j=0; j<3; j++)
        {
            cam.p[i][j] = C_ref._val[j][i];
        }
    }
    m_cams.push_back(cam);

    if(m_cams.size()>LIMIT_NUM_OF_FARME)
    {
        m_cams.pop_front();
    }
    updateGL();
}

void ModelPainting::AddPoints(std::vector<Point3D> p)
{
    // create display list
    int32_t gl_idx = glGenLists(1);
    m_glList.push_back(gl_idx);
    glNewList(gl_idx,GL_COMPILE);

    glPointSize(DRAWING_POINT_SIZE);
    glBegin(GL_POINTS);
    for (vector<Point3D>::iterator it=p.begin(); it!=p.end(); it++)
    {
        glColor3f(it->val,it->val,it->val);
        glVertex3f(it->x,it->y,it->z);
    }

    //If frame number bigger than the limit, then delete those old frame
    if(m_glList.size()>LIMIT_NUM_OF_FARME)
    {
        m_glList.pop_front();
    }

    // finish display list
    glEnd();
    glEnd();
    glEndList();

    updateGL();
}

void ModelPainting::FollowCamera(Matrix H)
{
    if(m_followCamFlag)
    {
        m_currPose.tx = -H._val[0][3];
        m_currPose.ty = -H._val[1][3]+0.5;
        m_currPose.tz = -H._val[2][3];
    }
    updateGL();
}

void ModelPainting::ResetView()
{
    m_currPose.zoom = -5;
    m_currPose.rotx = 180;
    m_currPose.roty = 0;
    m_currPose.tx   = 0;
    m_currPose.ty   = 0;
    m_currPose.tz   = -1.5;
    updateGL();
}

void ModelPainting::AddPose()
{
    m_poses.push_back(m_currPose);
    std::cout << "Add a pose, exsiting poses number: " << m_poses.size() << std::endl;
}

void ModelPainting::DelPose()
{
    if (m_poses.size()>0) m_poses.pop_back();
    std::cout << "Delete a pose, exsiting poses number: " << m_poses.size() << std::endl;
}

void ModelPainting::PlayPoses()
{
    float step_size = 0.04;
    for (int i=0; i<(int)m_poses.size()-1; i++)
    {
        Pose pose1 = m_poses[i];
        Pose pose2 = m_poses[i+1];
        for (float pos=0; pos<=1; pos+=step_size)
        {
            float pos2 = (1+sin(-M_PI/2+pos*M_PI))/2;
            m_currPose.zoom = pose1.zoom+(pose2.zoom-pose1.zoom)*pos2;
            m_currPose.rotx = pose1.rotx+(pose2.rotx-pose1.rotx)*pos2;
            m_currPose.roty = pose1.roty+(pose2.roty-pose1.roty)*pos2;
            m_currPose.tx   = pose1.tx  +(pose2.tx  -pose1.tx  )*pos2;
            m_currPose.ty   = pose1.ty  +(pose2.ty  -pose1.ty  )*pos2;
            m_currPose.tz   = pose1.tz  +(pose2.tz  -pose1.tz  )*pos2;
            updateGL();
        }
    }
}

//=============================================================================//
//===========================GL painting=======================================//
//=============================================================================//

void ModelPainting::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glClearColor(0,0,1,1);
}

void ModelPainting::paintGL()
{
    // clear screen & set matrices
    if (m_backgroundFlag)
    {
        glClearColor(1,1,1,1);
    }
    else
    {
        glClearColor(0,0,0,1);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // apply transformation
    glTranslatef(0.0, 0.0,m_currPose.zoom);
    glRotatef(m_currPose.rotx, 1.0, 0.0, 0.0);
    glRotatef(m_currPose.roty, 0.0, 1.0, 0.0);
    glTranslatef(m_currPose.tx,m_currPose.ty,m_currPose.tz);

    // draw 3d points
    for (deque<GLuint>::iterator it=m_glList.begin(); it!=m_glList.end(); it++)
    {
        glCallList(*it);
    }

    if (m_showCamFlag)
    {
        // draw cameras
        glDisable(GL_DEPTH_TEST);
        glLineWidth(1);
        for (deque<Camera>::iterator it = m_cams.begin(); it!=m_cams.end(); it++)
        {
            glColor3f(1.0,0.0,0.0);
            glBegin(GL_LINE_STRIP);
            for (int32_t i=0; i<10; i++)
            {
                glVertex3f(it->p[i][0],it->p[i][1],it->p[i][2]);
            }
            glEnd();
        }

        // draw connection line
        glBegin(GL_LINE_STRIP);
        for (deque<Camera>::iterator it = m_cams.begin(); it!=m_cams.end(); it++)
        {
            glVertex3f(it->p[5][0],it->p[5][1],it->p[5][2]);
        }
        glEnd();
        glEnable(GL_DEPTH_TEST);

    }

    if(m_followCamFlag)
    {
        //TODO: let the screen follow the camera.
    }

}

void ModelPainting::resizeGL(int width, int height)
{
    int side = qMax(width, height);
    glViewport((width-side)/2,(height-side)/2,side,side);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,1,0.1,10000);
    glMatrixMode(GL_MODELVIEW);

    updateGL();
}

//=============================================================================//
//===========================MouseEvent========================================//
//=============================================================================//

void ModelPainting::mousePressEvent(QMouseEvent *event)
{
    m_lastPose = event->pos();
}

void ModelPainting::mouseMoveEvent(QMouseEvent *event)
{
    // compute deltas
    float dx = -event->x() + m_lastPose.x();
    float dy = +event->y() - m_lastPose.y();

    if (event->buttons() & Qt::LeftButton)
    {
        m_currPose.rotx += dy;
        m_currPose.roty += dx;
    }
    // right button => translation
    // To study: this part is about the computer graphics and 3d points.
    else if (event->buttons() & Qt::RightButton )
    {
        float rotx2 = m_currPose.rotx;
        if (rotx2<170) rotx2 = 90;
        if (rotx2>190) rotx2 = 270;
        float rx = rotx2*M_PI/180.0;
        float ry = m_currPose.roty*M_PI/180.0;

        Matrix R = Matrix::rotMatY(-ry)*Matrix::rotMatX(-rx);

        Matrix v(3,1);
        v._val[0][0] = dx;
        v._val[1][0] = dy;

        v = R*v;
        m_currPose.tx += 0.0025*m_currPose.zoom*v._val[0][0];
        m_currPose.ty += 0.0025*m_currPose.zoom*v._val[1][0];
        m_currPose.tz += 0.0025*m_currPose.zoom*v._val[2][0];
    }

    m_lastPose = event->pos();
    updateGL();
}

void ModelPainting::wheelEvent(QWheelEvent *event)
{
    if (event->delta()>0)
    {
        m_currPose.zoom *= 1.15;
    }
    else
    {
        m_currPose.zoom /= 1.15;
    }
    updateGL();
}

void ModelPainting::keyPressEvent(QKeyEvent *event)
{
    float dx=0;
    float dy=0;

    switch (event->key())
    {
    case Qt::Key_W : dy=30; break;
    case Qt::Key_S : dy=-30; break;
    case Qt::Key_A : dx=-10; break;
    case Qt::Key_D : dx=10; break;
    }

    float rotx2 = m_currPose.rotx;
    if (rotx2<170) rotx2 = 90;
    if (rotx2>190) rotx2 = 270;
    float rx = rotx2*M_PI/180.0;
    float ry = m_currPose.roty*M_PI/180.0;

    Matrix R = Matrix::rotMatY(-ry)*Matrix::rotMatX(-rx);

    Matrix v(3,1);
    v._val[0][0] = dx;
    v._val[1][0] = dy;

    v = R*v;
    m_currPose.tx += 0.0025*m_currPose.zoom*v._val[0][0];
    m_currPose.ty += 0.0025*m_currPose.zoom*v._val[1][0];
    m_currPose.tz += 0.0025*m_currPose.zoom*v._val[2][0];

    updateGL();
}
