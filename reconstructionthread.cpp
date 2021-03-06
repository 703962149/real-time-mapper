#include "reconstructionthread.h"

ReconstructionThread::ReconstructionThread(CalibrationParameters *calib, StereoImage *stereo, QObject *parent)
    : QThread(parent)
    ,m_calib(calib)
    ,m_stereo(stereo)
{
    m_stereoImage = 0;

    //it is the max map point distance
    //if any point is far from the camera over this number, it would not be shown
    //TODO: make it could be changed in the mainwindow
    m_showingMinDistance = 0.1;
    m_showingMaxDistance = 25;

    if (m_calib->CheckRunDataType() == m_calib->DADAO_DATASET_FLAG)
    {
        m_f = m_calib->m_dadaoCalibParam.m_f; // x focal length in pixels.
        m_cu = m_calib->m_dadaoCalibParam.m_cu; // principal point (u-coordinate) in pixels
        m_cv = m_calib->m_dadaoCalibParam.m_cv; // principal point (v-coordinate) in pixels
        m_baseline = m_calib->m_dadaoCalibParam.m_baseline; // baseline in meters
    }
    else if (m_calib->CheckRunDataType() == m_calib->KITTI_DATASET_FLAG)
    {
        m_f = m_calib->m_KittiCalibParam.m_f; // x focal length in pixels.
        m_cu = m_calib->m_KittiCalibParam.m_cu; // principal point (u-coordinate) in pixels
        m_cv = m_calib->m_KittiCalibParam.m_cv; // principal point (v-coordinate) in pixels
        m_baseline = m_calib->m_KittiCalibParam.m_baseline; // baseline in meters
    }
    else if (m_calib->CheckRunDataType() == m_calib->LOITOR_DATASET_FLAG)
    {
        m_f = m_calib->m_loitorCalibParam.m_f; // x focal length in pixels.
        m_cu = m_calib->m_loitorCalibParam.m_cu; // principal point (u-coordinate) in pixels
        m_cv = m_calib->m_loitorCalibParam.m_cv; // principal point (v-coordinate) in pixels
        m_baseline = m_calib->m_loitorCalibParam.m_baseline; // baseline in meters
    }
}

ReconstructionThread::~ReconstructionThread()
{
    if (m_stereoImage!=0)
    {
        delete m_stereoImage;
        m_stereoImage = 0;
    }
    m_mapPoints.clear();

}

void ReconstructionThread::SendImageAndPose(StereoImage::StereoImageParameters &s, libviso2_Matrix H)
{
    if (m_stereoImage!=0)
    {
        delete m_stereoImage;
        m_stereoImage = 0;
    }

    m_stereoImage = new StereoImage::StereoImageParameters(s);
    m_HomographyMatrix = H;
}

void ReconstructionThread::run()
{

    Elas::parameters param(Elas::ROBOTICS);
    param.postprocess_only_left = true;
    param.filter_adaptive_mean = true;
    param.support_texture = 30;

    int32_t d_width  = m_stereoImage->width;
    int32_t d_height = m_stereoImage->height;

    if(m_stereoImage->D1 != 0)
    {
        free(m_stereoImage->D1);
        m_stereoImage->D1 = 0;
    }
    m_stereoImage->D1 = (float*)malloc(d_width*d_height*sizeof(float));

    if(m_stereoImage->D2 != 0)
    {
        free(m_stereoImage->D2);
        m_stereoImage->D2 = 0;
    }
    m_stereoImage->D2 = (float*)malloc(d_width*d_height*sizeof(float));

    const int32_t dims[3] = {m_stereoImage->width,m_stereoImage->height,m_stereoImage->step};

    Elas elas(param);
    elas.process(m_stereoImage->I1,m_stereoImage->I2,m_stereoImage->D1,m_stereoImage->D2,dims);

    AddDisparityMapToReconstruction();

    m_pickedNewMap = false;
    emit DetectNewMap();
    while (!m_pickedNewMap) usleep(1000);
}

ReconstructionThread::Map3d ReconstructionThread::CreateCurrentMap()
{
    Map3d m;
    m.I      = (float*)malloc(m_stereoImage->width*m_stereoImage->height*sizeof(float));
    m.D      = (float*)malloc(m_stereoImage->width*m_stereoImage->height*sizeof(float));
    m.X      = (float*)malloc(m_stereoImage->width*m_stereoImage->height*sizeof(float));
    m.Y      = (float*)malloc(m_stereoImage->width*m_stereoImage->height*sizeof(float));
    m.Z      = (float*)malloc(m_stereoImage->width*m_stereoImage->height*sizeof(float));
    m.H      = m_HomographyMatrix;
    m.width  = m_stereoImage->width;
    m.height = m_stereoImage->height;

    // copy disparities and intensities
    memcpy(m.D,m_stereoImage->D1,m_stereoImage->width*m_stereoImage->height*sizeof(float));
    for (int32_t v=0; v<m.height; v++)
    {
        for (int32_t u=0; u<m.width; u++)
        {
            m.I[v*m.width+u] = ((float)m_stereoImage->I1[v*m_stereoImage->step+u])/255.0;
        }
    }

    // compute 3d coordinates in first coordinate system
    float hcf00=0,hcf01=0,hcf02=0,hcf03=0,hcf10=0,hcf11=0,hcf12=0,hcf13=0,hcf20=0,hcf21=0,hcf22=0,hcf23=0;

    hcf00 = m.H._val[0][0]; hcf01 = m.H._val[0][1]; hcf02 = m.H._val[0][2]; hcf03 = m.H._val[0][3];
    hcf10 = m.H._val[1][0]; hcf11 = m.H._val[1][1]; hcf12 = m.H._val[1][2]; hcf13 = m.H._val[1][3];
    hcf20 = m.H._val[2][0]; hcf21 = m.H._val[2][1]; hcf22 = m.H._val[2][2]; hcf23 = m.H._val[2][3];

    for (int32_t u=0; u<m.width; u++)
    {
        for (int32_t v=0; v<m.height; v++)
        {
            int32_t addr = v*m.width+u;
            float   d    = m.D[addr];
            if (d>0)
            {
                float z = (m_f * m_baseline)/d;
                if ((z>m_showingMinDistance) && (z < m_showingMaxDistance))
                {
                    float x = ((float)u - m_cu)*m_baseline/d;
                    float y = ((float)v - m_cv)*m_baseline/d;
                    m.X[addr] = hcf00*x+hcf01*y+hcf02*z+hcf03;
                    m.Y[addr] = hcf10*x+hcf11*y+hcf12*z+hcf13;
                    m.Z[addr] = hcf20*x+hcf21*y+hcf22*z+hcf23;
                }
                else
                {
                    m.D[addr] = -1;
                }
            }
        }
    }

    return m;
}

void ReconstructionThread::AddDisparityMapToReconstruction()
{

    Map3d currentMap3d = CreateCurrentMap();

    m_mapPoints.clear();

    // for all pixels do
    for (int32_t u = 0; u < currentMap3d.width; u++)
    {
        for (int32_t v = 0; v < currentMap3d.height; v++)
        {
            // get pixel address and disparity
            int32_t addr = v * currentMap3d.width + u;
            float   d    = currentMap3d.D[addr];

            // if disparity is valid: add 3d coordinates to point cloud
            if (d>0)
            {
                m_mapPoints.push_back( ModelPainting::Point3D(currentMap3d.X[addr],
                                                              currentMap3d.Y[addr],
                                                              currentMap3d.Z[addr],
                                                              currentMap3d.I[addr]) );
            }
        }
    }

    ReleaseMap(currentMap3d);
}

void ReconstructionThread::ReleaseMap(ReconstructionThread::Map3d &m)
{
    if(m.I != 0)
    {
        free(m.I);
        m.I = 0;
    }
    if(m.D != 0)
    {
        free(m.D);
        m.D = 0;
    }
    if(m.X != 0)
    {
        free(m.X);
        m.X = 0;
    }
    if(m.Y != 0)
    {
        free(m.Y);
        m.Y = 0;
    }
    if(m.Z != 0)
    {
        free(m.Z);
        m.Z = 0;
    }
}
