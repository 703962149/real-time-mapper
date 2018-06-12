#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include <QObject>
#include <QThread>

#include "calibrationparameters.h"
#include "stereoimage.h"
#include "modelpainting.h"
#include "readlocalfilethread.h"

#include "libelas/src/elas.h"

class ReconstructionThread : public QThread
{
    Q_OBJECT

public:

    ReconstructionThread(CalibrationParameters *calib, StereoImage *stereo, QObject *parent = 0);
    ~ReconstructionThread();

    //Send image and pose from the mainwindow thread to this vo thread.
    void SendImageAndPose(StereoImage::StereoImageParameters &s, Matrix H);

    //The funtion for the mainwindow to get the 3d map points and the homographymatrix
    std::vector<ModelPainting::Point3D> GetMapPoints (){return m_mapPoints;}
    Matrix GetHomographyTotal() { return m_HomographyMatrix; }

    void PickupNewMap() { m_pickedNewMap = true; }

protected:

    void run();

private:

    //structruer: Map
    struct Map3d
    {
        float*   I;      // image
        float*   D;      // disparity map
        float*   X;      // 3d coordinates
        float*   Y;
        float*   Z;
        Matrix   H;      // extrinsics
        int32_t  width;  // image dimensions
        int32_t  height;
        int32_t  idx;    // index in point list

        Map3d()
        {
            I = 0;
            D = 0;
            X = 0;
            Y = 0;
            Z = 0;

            H = Matrix();
            width = 0;
            height = 0;
            idx = 0;
        }

        ~Map3d()
        {
            // Release the memory.
            if(I != 0)
            {
                free(I);
                I = 0;
            }
            if(D != 0)
            {
                free(D);
                D = 0;
            }
            if(X != 0)
            {
                free(X);
                X = 0;
            }
            if(Y != 0)
            {
                free(Y);
                Y = 0;
            }
            if(Z != 0)
            {
                free(Z);
                Z = 0;
            }

            H = Matrix();
            width = 0;
            height = 0;
            idx = 0;

        }

        inline Map3d& operator=(const Map3d& map)
        {
            if(height != map.height || width != map.width)
            {
                // Release the memory.
                if(I != 0)
                {
                    free(I);
                    I = 0;
                }
                if(D != 0)
                {
                    free(D);
                    D = 0;
                }
                if(X != 0)
                {
                    free(X);
                    X = 0;
                }
                if(Y != 0)
                {
                    free(Y);
                    Y = 0;
                }
                if(Z != 0)
                {
                    free(Z);
                    Z = 0;
                }

                // Alloc the memory
                I      = (float*)malloc(map.width * map.height * sizeof(float));
                D      = (float*)malloc(map.width * map.height * sizeof(float));
                X      = (float*)malloc(map.width * map.height * sizeof(float));
                Y      = (float*)malloc(map.width * map.height * sizeof(float));
                Z      = (float*)malloc(map.width * map.height * sizeof(float));
            }

            // Copy
            memcpy(I, map.I ,map.width * map.height * sizeof(float));
            memcpy(D, map.D ,map.width * map.height * sizeof(float));
            memcpy(X, map.X ,map.width * map.height * sizeof(float));
            memcpy(Y, map.Y ,map.width * map.height * sizeof(float));
            memcpy(Z, map.Z ,map.width * map.height * sizeof(float));
            H = map.H;
            width = map.width;
            height = map.height;
            idx = map.idx;

            return *this;
        }
    };

    Map3d CreateCurrentMap();
    void  AddDisparityMapToReconstruction();
    void  ReleaseMap(Map3d& m);

    CalibrationParameters *m_calib;
    StereoImage *m_stereo;
    StereoImage::StereoImageParameters *m_stereoImage;

    std::vector<ModelPainting::Point3D> m_mapPoints;

    bool m_pickedNewMap;
    float m_showingMinDistance;
    float m_showingMaxDistance;

    Matrix m_HomographyMatrix;

    double m_f; // x focal length in pixels.
    double m_cu; // principal point (u-coordinate) in pixels
    double m_cv; // principal point (v-coordinate) in pixels
    double m_baseline;// baseline in meters

signals:

    void DetectNewMap();

public slots:

};

#endif // RECONSTRUCTION_H
