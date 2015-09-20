#include "stdafx.h"
#include "facerec.h"
#include "Person.h"
#include "Html.h"

/* ----------------------------------------------------------------------- 
					Face Recognizition Based On PCA

						Variables and Functions 
   ----------------------------------------------------------------------- */

#define MAX_N_FACE	400
#define MIN_N_FACE	2
#define MAX_N_EIGEN	40		

FILE_SCOPE int		nTrainFaces;						// number of sample faces
FILE_SCOPE int		nEigenFaces;						// number of eigenfaces

FILE_SCOPE CvSize		libFaceSize;						// face image size in database

FILE_SCOPE IplImage*	faceImageArray[MAX_N_FACE] = {0};	// sample faces
FILE_SCOPE IplImage*	eigenFaceArray[MAX_N_EIGEN] = {0};	// eigenfaces
FILE_SCOPE IplImage*	pAvgTrainImg = NULL;				// average face

FILE_SCOPE CvMat*		eigenValueMat = 0;				// eigenvalues
FILE_SCOPE CvMat*		projectedTrainFaceMat = 0;			// eigen coefficients

extern vector<CPerson>		persons;						// infos of persons

// Open the training data from the file 'facedata.xml'.
int loadTrainingData()
{
	CvFileStorage * fileStorage;

	//printf("loading training data from training database file 'facedata.xml'.\n");

	// create a file-storage interface
	fileStorage = cvOpenFileStorage("facedata.xml", 0, CV_STORAGE_READ);
	if( !fileStorage ) 
	{
	//	printf("error : can't open training database file 'facedata.xml'.\n");
		return 0;
	}

	// Load the person names. Added by Shervin.
	persons.clear();	// Make sure it starts as empty.
	int nPersons = cvReadIntByName(fileStorage, 0, "nPersons", 0);
	if (nPersons == 0) {
	//	printf("note : no people found in the training database 'facedata.xml'.\n");
		//libFaceSize.width = libFaceSize.height = 60; // default size
		return 1;
	}
	// Load each person's name.
	for (int i = 0; i < nPersons; i++) {
		char varname[NAMESIZ];
		sprintf(varname, "personName_%d", i);
		CPerson person(cvReadStringByName(fileStorage, 0, varname));
		persons.push_back(person);
	}

	// Load the data
	nTrainFaces = cvReadIntByName(fileStorage, 0, "nTrainFaces", 0);
	nEigenFaces = cvReadIntByName(fileStorage, 0, "nEigenFaces", 0);

	libFaceSize.width = cvReadIntByName(fileStorage, 0, "libFaceSize_width", 0);
	libFaceSize.height = cvReadIntByName(fileStorage, 0, "libFaceSize_height", 0);
	//eigenFaceArray = (IplImage **)cvAlloc(nTrainFaces*sizeof(IplImage *));
	for(int i = 0; i < nTrainFaces; i++)
	{
		char varname[NAMESIZ];
		sprintf(varname, "trainingFace_%d", i);
		faceImageArray[i] = (IplImage *)cvReadByName(fileStorage, 0, varname, 0);
	}
	if (nTrainFaces < MIN_N_FACE)
	{
		cvReleaseFileStorage(&fileStorage);
		return 1;
	} 

	eigenValueMat = (CvMat *)cvReadByName(fileStorage, 0, "eigenValueMat", 0);
	projectedTrainFaceMat = (CvMat *)cvReadByName(fileStorage, 0, "projectedTrainFaceMat", 0);
	pAvgTrainImg = (IplImage *)cvReadByName(fileStorage, 0, "avgTrainImg", 0);
	for(int i = 0; i < nEigenFaces; i++)
	{
		char varname[NAMESIZ];
		sprintf(varname, "eigenVect_%d", i);
		eigenFaceArray[i] = (IplImage *)cvReadByName(fileStorage, 0, varname, 0);
	}

	// release the file-storage interface
	cvReleaseFileStorage(&fileStorage);

	//printf("training data loaded (%d training images of %d people):\n", nTrainFaces, nPersons);

	return 1;
}

void storeTrainingData()
{
	CvFileStorage * fileStorage;

	//printf("storing training data to training database file 'facedata.xml'.\n");

	// create a file-storage interface
	fileStorage = cvOpenFileStorage("facedata.xml", 0, CV_STORAGE_WRITE );

	// Store the person names.
	int nPersons = persons.size();
	cvWriteInt(fileStorage, "nPersons", nPersons);
	for (int i = 0; i < nPersons; i++) {
		char varname[NAMESIZ];
		sprintf(varname, "personName_%d", i);
		cvWriteString(fileStorage, varname, persons[i].getName());
	}

	// store all the data
	cvWriteInt(fileStorage, "nTrainFaces", nTrainFaces);
	cvWriteInt(fileStorage, "nEigenFaces", nEigenFaces );
	cvWriteInt(fileStorage, "libFaceSize_width", libFaceSize.width);
	cvWriteInt(fileStorage, "libFaceSize_height", libFaceSize.height);
	for(int i = 0; i < nTrainFaces; i++)
	{
		char varname[NAMESIZ];
		sprintf(varname, "trainingFace_%d", i);
		cvWrite(fileStorage, varname, faceImageArray[i], cvAttrList(0,0));
	}
	if (nTrainFaces < MIN_N_FACE)
	{
		cvReleaseFileStorage(&fileStorage);
		return;
	}

	cvWrite(fileStorage, "eigenValueMat", eigenValueMat, cvAttrList(0,0));
	cvWrite(fileStorage, "projectedTrainFaceMat", projectedTrainFaceMat, cvAttrList(0,0));
	cvWrite(fileStorage, "avgTrainImg", pAvgTrainImg, cvAttrList(0,0));
	for(int i = 0; i < nEigenFaces; i++)
	{
		char varname[NAMESIZ];
		sprintf(varname, "eigenVect_%d", i);
		cvWrite(fileStorage, varname, eigenFaceArray[i], cvAttrList(0,0));
	}
	// release the file-storage interface
	cvReleaseFileStorage(&fileStorage);
}

void releaseTrainingData()
{	// Release memory resourse used by PCA
	cvReleaseMat(&eigenValueMat);
	cvReleaseMat(&projectedTrainFaceMat);
	cvReleaseImage(&pAvgTrainImg);
	for(int i = 0; i < nTrainFaces; i++)
		cvReleaseImage(&faceImageArray[i]);
	for(int i = 0; i < nEigenFaces; i++)
		cvReleaseImage(&eigenFaceArray[i]);
}

FILE_SCOPE void saveImages(const char* directory, const char* label, vector<string> &picnames,
	IplImage* imageArray[], int nImages)
{
	char cName[256];
	string dirStr(directory);
	dirStr += "\\";

	picnames.clear();
	for (int i = 0; i < nImages; i++)
	{
		sprintf(cName, "%s_%d.jpg", label, i); 
		string name(cName);
		name = dirStr + name;
		picnames.push_back(name);

		if (imageArray[i]->depth == IPL_DEPTH_32F 
			&& imageArray[i]->nChannels == 1)
		{
			IplImage* imageTmp = convertFloat32ToGreyscale(imageArray[i]);
			cvSaveImage(name.c_str(), imageTmp);
			cvReleaseImage(&imageTmp);
		}
		else
			cvSaveImage(name.c_str(), imageArray[i]);
	}
}

FILE_SCOPE IplImage* reconstruct(IplImage*, IplImage*[], int, float[]);
void showRecords(const char* filename)
{
	// Create a HTML file named (filename), and write records into it
	CHtml file(filename, "Records");
	
	char varstr[256];
	
	file.ShowText("Records", 1);
	
	vector<string> personNames;
	for (int i = 0, len = persons.size(); i < len; i++)
		personNames.push_back(persons[i].getName());

	vector<string> picnames;
	sprintf(varstr, "Number of training faces : %d", nTrainFaces);
	file.ShowText(varstr);
	file.EndLine();

	if (nTrainFaces > 0)
	{
		file.ShowText("Training faces:");
		file.EndLine();
		saveImages("records", "trainingFace", picnames, faceImageArray, nTrainFaces);
		file.ShowImageTable(picnames, personNames, libFaceSize.width, libFaceSize.height, 10);
		file.EndLine();
	}

	if (nTrainFaces < MIN_N_FACE)
	{
		sprintf(varstr, "Number of eigen faces : %d", nEigenFaces);
		file.ShowText(varstr);
		file.EndLine();
		return;
	}

	file.ShowText("Reconstruction:");
	file.EndLine();
	IplImage* reconstructFaces[MAX_N_FACE];
	for (int i = 0; i < nTrainFaces; i++)
	{
		reconstructFaces[i] = reconstruct(pAvgTrainImg, eigenFaceArray, nEigenFaces, 
			projectedTrainFaceMat->data.fl + i * nEigenFaces);
	}
	saveImages("records", "reconstruct", picnames, reconstructFaces, nTrainFaces);
	file.ShowImageTable(picnames, personNames, libFaceSize.width, libFaceSize.height, 10);
	file.EndLine();
	releaseImageArray(reconstructFaces, nTrainFaces);

	sprintf(varstr, "Number of eigen faces : %d", nEigenFaces);
	file.ShowText(varstr);
	file.EndLine();
	file.ShowText("Eigenfaces(EigenVectors):");
	file.EndLine();
	saveImages("records", "eigenFace", picnames, eigenFaceArray, nEigenFaces);
	file.ShowImageTable(picnames, libFaceSize.width, libFaceSize.height, 10);
	file.EndLine();
	
	file.ShowText("EigenValues:");
	file.EndLine();
	for (int i = 1; i <= nEigenFaces; i++)
	{
		sprintf(varstr, "%16.4f ", eigenValueMat->data.fl[i-1]); 
		file.ShowText(varstr);
		if (i % 6 == 0)
			file.EndLine();
	}
	file.EndLine();
}

FILE_SCOPE int calcnEigens(int nTrains)
{
	if (nTrains < MIN_N_FACE)
		return 0;
	else if (nTrains <= MAX_N_EIGEN)
		return nTrains - 1;
	else
		return MAX_N_EIGEN;
}


// Do the Principal Component Analysis, finding the average image
// and the eigenfaces that represent any image in the given dataset.
FILE_SCOPE void doPCA()
{
	// set the number of eigenvalues to use
	nEigenFaces = calcnEigens(nTrainFaces);

	// allocate the eigenvector images	
	//eigenFaceArray = (IplImage**)cvAlloc(sizeof(IplImage*) * nEigenFaces);
	for(int i = 0; i < nEigenFaces; i++)
	{
		cvReleaseImage(&eigenFaceArray[i]);
		eigenFaceArray[i] = cvCreateImage(libFaceSize, IPL_DEPTH_32F, 1);
	}

	// allocate the eigenvalue array
	cvReleaseMat(&eigenValueMat);
	eigenValueMat = cvCreateMat(1, nEigenFaces, CV_32FC1);

	// allocate the averaged image
	cvReleaseImage(&pAvgTrainImg);
	pAvgTrainImg = cvCreateImage(libFaceSize, IPL_DEPTH_32F, 1);

	// set the PCA termination criterion
	CvTermCriteria calcLimit = cvTermCriteria(CV_TERMCRIT_ITER, nEigenFaces, 1);

	// compute average image, eigenvalues, and eigenvectors
	cvCalcEigenObjects(
		nTrainFaces,
		(void*)faceImageArray,
		(void*)eigenFaceArray,
		CV_EIGOBJ_NO_CALLBACK,
		0,
		0,
		&calcLimit,
		pAvgTrainImg,
		eigenValueMat->data.fl);
}

FILE_SCOPE IplImage* reconstruct(IplImage* averageface, IplImage* eigenfaces[], int neigens, float eigencoeffs[])
{
	IplImage* imageDst = NULL;
	IplImage* imageTmp = NULL; 

	if (neigens > 0 
		&& eigenfaces[0]->depth == IPL_DEPTH_32F && eigenfaces[0]->nChannels == 1
		&& averageface->depth == IPL_DEPTH_32F && averageface->nChannels == 1)
	{
		int width = eigenfaces[0]->width;
		int height = eigenfaces[0]->height;
		
		imageTmp = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
		if (imageTmp == NULL)
		{
			cvReleaseImage(&imageDst);
			return NULL;
		}
		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
			{
				int k = i * width + j;
				float* sum = &((float*)imageTmp->imageData)[k];
				*sum = ((float*)averageface->imageData)[k];
				for (int s = 0; s < neigens; s++)
				{
					float temp = ((float*)(eigenfaces[s]->imageData))[k];
					*sum += temp * eigencoeffs[s];
				}
			}
		imageDst = imageTmp;
		/*imageDst = convertFloat32ToGreyscale(imageTmp);
		cvReleaseImage(&imageTmp);*/
	}

	return imageDst;
}

FILE_SCOPE float calcEuclideanDist(float v1[], float v2[], int n)
{
	double ans = 0.0;
	for (int i = 0; i < n; i++)
	{
		double diff = v2[i] - v1[i];
		ans += diff * diff;
	}
	return (float) ans;
}

FILE_SCOPE int findNearest(float faceEigenVector[], float *pConfidence)
{
	double leastDistSq = DBL_MAX;
	int iTrain, iNearest = 0;

	for(iTrain = 0; iTrain < nTrainFaces; iTrain++)
	{
		float* projectedTrainFaceVec = projectedTrainFaceMat->data.fl + iTrain * nEigenFaces;
		
		double distSq = calcEuclideanDist(faceEigenVector, projectedTrainFaceVec, nEigenFaces); 

		if(distSq < leastDistSq)
		{
			leastDistSq = distSq;
			iNearest = iTrain;
		}
	}
	//cout << "Least square of distance: "<< leastDistSq << endl;
	//cout << "value : " << sqrt(leastDistSq / (double)(nTrainFaces * nEigenFaces)) / 255.0 << endl;
	// Return the confidence level based on the Euclidean distance,
	// so that similar images should give a confidence between 0.5 to 1.0,
	// and very different images should give a confidence between 0.0 to 0.5.
	*pConfidence = (float)(1.0 - sqrt(leastDistSq / (double)(nTrainFaces * nEigenFaces)) / 255.0);

	// Return the found index.
	return iNearest;
}

int train(const IplImage* face)
{
	if (face == NULL)
		return 0;

	if (nTrainFaces == 0)
	{
		// Original face size. Once nTrainfaces > 0, this block cannot be changed. 
		libFaceSize.height = 60;
		libFaceSize.width = 60;
	}

	IplImage* resiz = resizeImage(face, libFaceSize);
	IplImage* grey = convertImageToGreyscale(resiz);
	cvEqualizeHist(grey, grey);

	cvReleaseImage(&resiz);

	if (grey == NULL)
		return 0;

	faceImageArray[nTrainFaces] = grey;
	nTrainFaces += 1;
	
	if (nTrainFaces < MIN_N_FACE)
	{
		nEigenFaces = 0;
		storeTrainingData();
		return 1;
	}

	doPCA();

	// project the training images onto the PCA subspace
	cvReleaseMat(&projectedTrainFaceMat);
	projectedTrainFaceMat = cvCreateMat(nTrainFaces, nEigenFaces, CV_32FC1);
	int offset = projectedTrainFaceMat->step / sizeof(float);
	for(int i = 0; i < nTrainFaces; i++)
	{
		cvEigenDecomposite(
			faceImageArray[i],
			nEigenFaces,
			eigenFaceArray,
			0, 0,
			pAvgTrainImg,
			projectedTrainFaceMat->data.fl + i*offset);
	}
	storeTrainingData();

	return 1;
}

int facerec(const IplImage* face, float *pConfidence)
{
	int index = -1; // defualt return value
	
	if (pConfidence == NULL || nTrainFaces < MIN_N_FACE)
		return -1;
	*pConfidence = 0.0f;

	// Prepocessing
	IplImage* resiz = resizeImage(face, libFaceSize);
	IplImage* grey = convertImageToGreyscale(resiz);

	cvReleaseImage(&resiz);

	if (grey == NULL)
		return -1;

	IplImage* imageTmp = cvCreateImage(cvGetSize(grey), grey->depth, grey->nChannels);
	if (imageTmp == NULL)
	{
		cvReleaseImage(&grey);
		return -1;
	}
	else
		cvEqualizeHist(grey, imageTmp);
	cvReleaseImage(&grey);

	// Do face recognizition 
	CvMat* faceEigenMat = cvCreateMat(1, nEigenFaces, CV_32FC1);
	if (faceEigenMat != NULL)
	{
		float* faceEigenVector = faceEigenMat->data.fl;

		cvEigenDecomposite(imageTmp, nEigenFaces, eigenFaceArray, 0, 0, pAvgTrainImg, faceEigenVector);

		index = findNearest(faceEigenVector, pConfidence);

		cvReleaseMat(&faceEigenMat);
		faceEigenVector = NULL;
	}
	cvReleaseImage(&imageTmp);

	return index;
}

int facerec(const IplImage* face, const float threshold)
{
	float confidence;

	int index = facerec(face, &confidence);
	if (confidence < threshold)
		return -1;

	return index;
}