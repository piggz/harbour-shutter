#ifndef FACEDETECTION_H
#define FACEDETECTION_H

#include <QRectF>
#include <QImage>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>

class FaceDetection
{
public:
    FaceDetection();
    QList<QRectF> detect(QImage image);

private:
    cv::CascadeClassifier classifier;
};

#endif // FACEDETECTION_H
