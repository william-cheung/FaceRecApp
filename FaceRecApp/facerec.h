#ifndef __FACEREC_H__
#define __FACEREC_H__

#include		<cstdio>
#include		<cstdlib>
#include		<iostream>
#include		<string>
#include		<vector>

#include		<opencv2/opencv.hpp>
#include		<cvaux.h>

using namespace std;
using namespace cv;

#define	FILE_SCOPE	static

#define	NAMESIZ		256

typedef	IplImage*	ImagePtr;


// basic.cpp
void			cvText					(IplImage* img, const char* text, const CvPoint pos, const double hscale, const double vscale, const CvScalar color, const int width);

IplImage*	convertImageToGreyscale	(const IplImage *imageSrc);
IplImage*	convertFloat32ToGreyscale	(const IplImage *imageSrc);
IplImage*	cropImage				(const IplImage *img, const CvRect region);
IplImage*	cropImage				(const IplImage *src, const CvRect region, IplImage** pdst);
IplImage*	resizeImage				(const IplImage *origImg, const CvSize newSize);
IplImage*	resizeImage				(const IplImage *src, const CvSize newSize, IplImage** pdst);
void			releaseImages				(int nImages, ...);
void			releaseImageArray			(IplImage* imageArray[], int nImages);

CvRect		detectFaceInImage			(IplImage* inputImg, CvHaarClassifierCascade* cascade, CvSize size);
// end basic.cpp

// PCA.cpp
int			loadTrainingData			(void);
void			storeTrainingData			(void);
void			releaseTrainingData		(void);
void			showRecords				(const char* filename);
int			train					(const IplImage* face);
int			facerec					(const IplImage* face, float *pConfidence);
int			facerec					(const IplImage* face, const float threshold = 0.60f);
// end PCA.cpp

#endif // end facerec.h