#include "facedetection.h"
#include <QFile>
#include <QDebug>
#include <QTemporaryFile>
#include <QImage>

FaceDetection::FaceDetection()
{
    QFile xml(":/assets/classifiers/lbpcascade_frontalface.xml");

    if(xml.open(QFile::ReadOnly | QFile::Text))
    {
        QTemporaryFile temp;
        if(temp.open())
        {
            temp.write(xml.readAll());
            if(classifier.load(temp.fileName().toStdString()))
            {
                qDebug() << "Successfully loaded classifier!";
            }
            else
            {
                qDebug() << "Could not load classifier.";
            }
            temp.close();
        }
        else
        {
            qDebug() << "Can't open temp file.";
        }
    }
    else
    {
        qDebug() << "Can't open XML.";
    }
}

QList<QRectF> FaceDetection::detect(QImage image)
{
    //qDebug() << Q_FUNC_INFO;

    if (image.isNull()) {
         QList<QRectF> r;
            return r;
    }

    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat frame(image.height(),
                  image.width(),
                  CV_8UC3,
                  image.bits(),
                  image.bytesPerLine());

    //cv::flip(frame, frame, 0);

    cv::Mat frameGray;

    if(frame.channels() == 3){
        cvtColor( frame, frameGray, cv::COLOR_BGR2GRAY );
    }else if(frame.channels() == 4) {
        cvtColor( frame, frameGray, cv::COLOR_BGRA2GRAY );
    }

    cv::equalizeHist( frameGray, frameGray );

    std::vector<cv::Rect> detected;

    //resize the frame
    double imageWidth = image.size().width();
    double imageHeight = image.size().height();

    double resizedWidth = 320;
    double resizedHeight = (imageHeight/imageWidth) * resizedWidth;

    cv::resize(frameGray, frameGray, cv::Size((int)resizedWidth, (int)resizedHeight));

    classifier.detectMultiScale(frameGray, detected, 1.1, 2, 0|cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

    QList<QRectF> rects;
    QRectF rect;

    double rX, rY, rWidth, rHeight;
    cv::Size frameSize = frameGray.size();

    for(size_t i = 0; i < detected.size(); i++){

        rX = double(detected[i].x) / double(frameSize.width);
        rY = double(detected[i].y) / double(frameSize.height);
        rWidth = double(detected[i].width) / double(frameSize.width);
        rHeight = double(detected[i].height) / double(frameSize.height);

        qDebug() << "Face:" << rX << rY << rWidth << rHeight;

        rect = QRectF(rX, rY, rWidth, rHeight);
        rects.append(rect);
    }

    return rects;
}
