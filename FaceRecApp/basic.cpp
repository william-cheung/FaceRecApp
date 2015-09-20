#include "stdafx.h"
#include "facerec.h"

void cvText(IplImage* img, 
		const char* text, 
		const CvPoint pos, 
		const double hscale, 
		const double vscale, 
		const CvScalar color,
		const int width)
{
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, hscale, vscale, 0, width);
    cvPutText(img, text, pos, &font, color);
}

IplImage* cropImage(const IplImage *img, const CvRect region)
{
	IplImage *imageTmp = NULL;
	IplImage *imageRGB = NULL;
	
	if (img == NULL || img->depth != IPL_DEPTH_8U) 
		return NULL;

	// First create a new (color or greyscale) IPL Image and copy contents of img into it.
	imageTmp = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, img->nChannels);
	cvCopy(img, imageTmp, NULL);

	// Create a new image of the detected region
	// Set region of interest to that surrounding the face
	cvSetImageROI(imageTmp, region);

	// Copy region of interest (i.e. face) into a new iplImage (imageRGB) and return it
	CvSize size = cvSize(region.width, region.height);
	imageRGB = cvCreateImage(size, IPL_DEPTH_8U, img->nChannels);
	cvCopy(imageTmp, imageRGB, NULL);	// Copy just the region.

    cvReleaseImage(&imageTmp);

	return imageRGB;		
}

IplImage* cropImage(const IplImage *src, const CvRect region, IplImage** pdst)
{
	if (*pdst != NULL)
		cvReleaseImage(pdst);
	*pdst = cropImage(src, region);
	return *pdst;
}

IplImage* convertImageToGreyscale(const IplImage *imageSrc)
{
	IplImage *imageGrey = NULL;
	// Either convert the image to greyscale, or make a copy of the existing greyscale image.
	// This is to make sure that the user can always call cvReleaseImage() on the output, whether it was greyscale or not.
	if (imageSrc == NULL)
		return NULL;
	else if (imageSrc->nChannels == 3)  
	{
		imageGrey = cvCreateImage(cvGetSize(imageSrc), IPL_DEPTH_8U, 1);
		cvCvtColor(imageSrc, imageGrey, CV_BGR2GRAY);
	}
	else 
		imageGrey = cvCloneImage(imageSrc);

	return imageGrey;
}

IplImage* convertFloat32ToGreyscale(const IplImage *imageSrc)
{
	typedef unsigned char UINT8;

	IplImage* imageDst = NULL;
	if (imageSrc != NULL && imageSrc->nChannels == 1)
	{	
		imageDst = cvCreateImage(cvGetSize(imageSrc), 
			IPL_DEPTH_8U, imageSrc->nChannels);
		if (imageDst == NULL)
			return NULL;
		
		int width = imageSrc->width;
		int height = imageSrc->height;
		if (imageSrc->depth == IPL_DEPTH_32F)
		{
			for (int i = 0; i < height; i++)
				for (int j = 0; j < width; j++)
				{
					int k = i * width + j;
					
					int pix = (int)
						((float*)(imageSrc->imageData))[k];
					if (pix > 255) pix = 255;
					else if (pix < 0) pix = 0;
					imageDst->imageData[k] = (UINT8)pix;
				}
		}
		else if (imageSrc->depth == IPL_DEPTH_8U)
			cvCopy(imageSrc, imageDst);
		else
			cvReleaseImage(&imageDst);
	}

	return imageDst;
}

IplImage* resizeImage(const IplImage *origImg, const CvSize newSize)
{
	IplImage *outImg	= 0;

	int origWidth		= 0;
	int origHeight	= 0;

	if (origImg != NULL) 
	{
		origWidth = origImg->width;
		origHeight = origImg->height;
	}
	else
		return NULL;

	int newWidth = newSize.width;
	int newHeight = newSize.height;

	// Processing illegal parameters
	if (newWidth <= 0 || newHeight <= 0 || origImg == 0 || origWidth <= 0 || origHeight <= 0) 
		return NULL;

	// Scale the image to the new dimensions, even if the aspect ratio will be changed.
	outImg = cvCreateImage(cvSize(newWidth, newHeight), origImg->depth, origImg->nChannels);
	if (newWidth > origImg->width && newHeight > origImg->height) {
		// Make the image larger
		cvResetImageROI((IplImage*)origImg);
		cvResize(origImg, outImg, CV_INTER_LINEAR);	// CV_INTER_CUBIC or CV_INTER_LINEAR is good for enlarging
	}
	else {
		// Make the image smaller
		cvResetImageROI((IplImage*)origImg);
		cvResize(origImg, outImg, CV_INTER_AREA);	// CV_INTER_AREA is good for shrinking / decimation, but bad at enlarging.
	}

	return outImg;
}

IplImage* resizeImage(const IplImage *src, const CvSize newSize, IplImage** pdst)
{
	if (*pdst != NULL)
		cvReleaseImage(pdst);
	*pdst = resizeImage(src, newSize);
	return *pdst; 
}

void releaseImages(int nImages, ...)
{
	va_list ap;
	va_start(ap, nImages);
	for (int i = 0; i < nImages; i++)
	{
		IplImage** image = va_arg(ap, IplImage**);
		if (*image != NULL)
			cvReleaseImage(image);
	}
	va_end(ap);
}

void releaseImageArray(IplImage* imageArray[], int nImages)
{
	for (int i = 0; i < nImages; i++)
		if (imageArray[i] != NULL)
			cvReleaseImage(&imageArray[i]);
}

// Perform face detection on the input image, using the given Haar Cascade. 
// Returns a rectangle for the detected region in the given image.
CvRect detectFaceInImage(IplImage* inputImg, CvHaarClassifierCascade* cascade, CvSize faceSize)
{
	// Only search for 1 face.
	int flags = CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
	// How detailed should the search be.
	float search_scale_factor = 1.1f;
	IplImage *detectImg = NULL;
	IplImage *greyImg = NULL;
	CvMemStorage* storage = NULL;

	CvSeq* rects = NULL;
	CvSize srcSize;
	CvRect rc;
	int nFaces;

	storage = cvCreateMemStorage(0); // Allocate 64KB memory space
	cvClearMemStorage(storage);

	// If the image is color, use a greyscale copy of the image.
	detectImg = (IplImage*)inputImg;
	if (inputImg->nChannels > 1) {
		srcSize = cvSize(inputImg->width, inputImg->height);
		greyImg = cvCreateImage(srcSize, IPL_DEPTH_8U, 1);
		cvCvtColor(inputImg, greyImg, CV_BGR2GRAY);
		detectImg = greyImg;	// Use the greyscale image.
	}

	// Detect all the faces in the greyscale image.
	rects = cvHaarDetectObjects(detectImg, cascade, storage,
			search_scale_factor, 3, flags, faceSize);
	nFaces = rects->total;

	// Get the first detected face (the biggest).
	if (nFaces > 0)
		rc = *(CvRect*)cvGetSeqElem(rects, 0);
	else
		rc = cvRect(-1, -1, -1, -1);	// Couldn't find the face.

	if (greyImg != NULL)
		cvReleaseImage(&greyImg);
	cvReleaseMemStorage(&storage);

	return rc;	// Return the biggest face found, or (-1,-1,-1,-1).
}