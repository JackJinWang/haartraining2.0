#include "haarTraining.h"
#include <vector>
#include <fstream>
#include"tinyxml2.h"
using namespace tinyxml2;
#include<string>
#include <ctype.h>
#include<time.h>
#include <math.h>
#include"myIntergal.h"
#include"delete.h"
#include"classifier.h"
using namespace std;
#define POS_FLAG 1
#define NEG_FLAG 2

/*
*
* define xml name
*/
#define WEAK "weak"
#define HAARFEATUR "haarfeature"
#define ERRORER "error"
#define LEFT "left"
#define RIGHT "right"
#define THRESHOLD "threshold"
#define POSISOVER 1
#define NEGISOVER 2
#define REPLACESUCCESSFUL 3
/*
* get sum image offsets for <rect> corner points
* step - row step (measured in image pixels!) of sum image
*/
#define CV_SUM_OFFSETS( p0, p1, p2, p3, rect, step )                      \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x + w, y) */                                                      \
    (p1) = (rect).x + (rect).width + (step) * (rect).y;                   \
    /* (x , y+h) */                                                      \
    (p2) = (rect).x + (step) * ((rect).y + (rect).height);                \
    /* (x + w, y + h) */                                                  \
    (p3) = (rect).x + (rect).width + (step) * ((rect).y + (rect).height);

/*
* get tilted image offsets for <rect> corner points
* step - row step (measured in image pixels!) of tilted image
*/
#define CV_TILTED_OFFSETS( p0, p1, p2, p3, rect, step )                   \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x - h, y + h) */                                                  \
    (p1) = (rect).x - (rect).height + (step) * ((rect).y + (rect).height);\
    /* (x + w, y + w) */                                                  \
    (p2) = (rect).x + (rect).width + (step) * ((rect).y + (rect).width);  \
    /* (x + w - h, y + w + h) */                                          \
    (p3) = (rect).x + (rect).width - (rect).height                        \
           + (step) * ((rect).y + (rect).width + (rect).height);

/*
* icvCreateIntHaarFeatures
*
* Create internal representation of haar features
*
* mode:
*  0 - BASIC = Viola
*  1 - CORE  = All upright
*  2 - ALL   = All features
*/
static
CvIntHaarFeatures* icvCreateIntHaarFeatures(MySize winsize,
	int mode,
	int symmetric)
{
	CvIntHaarFeatures* features = NULL;
	CvTHaarFeature haarFeature;

	//CvMemStorage* storage = NULL;
	//CvSeq* seq = NULL;
	//CvSeqWriter writer;

	vector<CvTHaarFeature> seq;

	int s0 = 36; /* minimum total area size of basic haar feature     */
	int s1 = 12; /* minimum total area size of tilted haar features 2 */
	int s2 = 18; /* minimum total area size of tilted haar features 3 */
	int s3 = 24; /* minimum total area size of tilted haar features 4 */

	int x = 0;
	int y = 0;
	int dx = 0;
	int dy = 0;

#if 0
	float factor = 1.0F;

	factor = ((float)winsize.width) * winsize.height / (24 * 24);

	s0 = (int)(s0 * factor);
	s1 = (int)(s1 * factor);
	s2 = (int)(s2 * factor);
	s3 = (int)(s3 * factor);
#else
	s0 = 1;
	s1 = 1;
	s2 = 1;
	s3 = 1;
#endif

	/* CV_VECTOR_CREATE( vec, CvIntHaarFeature, size, maxsize ) */
	//	storage = cvCreateMemStorage();
	//	cvStartWriteSeq(0, sizeof(CvSeq), sizeof(haarFeature), storage, &writer);

	for (x = 0; x < winsize.width; x++)
	{
		for (y = 0; y < winsize.height; y++)
		{
			for (dx = 1; dx <= winsize.width; dx++)
			{
				for (dy = 1; dy <= winsize.height; dy++)
				{
					// haar_x2 //右－左
					if ((x + dx * 2 <= winsize.width) && (y + dy <= winsize.height)) {
						if (dx * 2 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 2 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x2",
								x, y, dx * 2, dy, -1,
								x + dx, y, dx, dy, +2);
							/* CV_VECTOR_PUSH( vec, CvIntHaarFeature, haarFeature, size, maxsize, step ) */
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_y2 下减上
					if ((x + dx <= winsize.width) && (y + dy * 2 <= winsize.height)) {
						if (dx * 2 * dy < s0) continue;
						if (!symmetric || (x + x + dx <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_y2",
								x, y, dx, dy * 2, -1,
								x, y + dy, dx, dy, +2);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_x3  中间-两侧 横
					if ((x + dx * 3 <= winsize.width) && (y + dy <= winsize.height)) {
						if (dx * 3 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 3 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x3",
								x, y, dx * 3, dy, -1,
								x + dx, y, dx, dy, +3);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_y3 中间-两侧 竖
					if ((x + dx <= winsize.width) && (y + dy * 3 <= winsize.height)) {
						if (dx * 3 * dy < s0) continue;
						if (!symmetric || (x + x + dx <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_y3",
								x, y, dx, dy * 3, -1,
								x, y + dy, dx, dy, +3);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					if (mode != 0 /*BASIC*/) {
						// haar_x4
						if ((x + dx * 4 <= winsize.width) && (y + dy <= winsize.height)) {
							if (dx * 4 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 4 <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_x4",
									x, y, dx * 4, dy, -1,
									x + dx, y, dx * 2, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// haar_y4
						if ((x + dx <= winsize.width) && (y + dy * 4 <= winsize.height)) {
							if (dx * 4 * dy < s0) continue;
							if (!symmetric || (x + x + dx <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_y4",
									x, y, dx, dy * 4, -1,
									x, y + dy, dx, dy * 2, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}
					}

					// x2_y2
					if ((x + dx * 2 <= winsize.width) && (y + dy * 2 <= winsize.height)) {
						if (dx * 4 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 2 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x2_y2",
								x, y, dx * 2, dy * 2, -1,
								x, y, dx, dy, +2,
								x + dx, y + dy, dx, dy, +2);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					if (mode != 0 /*BASIC*/) {
						// point
						if ((x + dx * 3 <= winsize.width) && (y + dy * 3 <= winsize.height)) {
							if (dx * 9 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 3 <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_point",
									x, y, dx * 3, dy * 3, -1,
									x + dx, y + dy, dx, dy, +9);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}
					}

					if (mode == 2 /*ALL*/) {
						// tilted haar_x2                                      (x, y, w, h, b, weight)
						if ((x + 2 * dx <= winsize.width) && (y + 2 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 2 * dy < s1) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x2",
									x, y, dx * 2, dy, -1,
									x, y, dx, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y2                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 2 * dy <= winsize.height) && (x - 2 * dy >= 0)) {
							if (dx * 2 * dy < s1) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y2",
									x, y, dx, 2 * dy, -1,
									x, y, dx, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_x3                                   (x, y, w, h, b, weight)
						if ((x + 3 * dx <= winsize.width) && (y + 3 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 3 * dy < s2) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x3",
									x, y, dx * 3, dy, -1,
									x + dx, y + dx, dx, dy, +3);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y3                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 3 * dy <= winsize.height) && (x - 3 * dy >= 0)) {
							if (dx * 3 * dy < s2) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y3",
									x, y, dx, 3 * dy, -1,
									x - dy, y + dy, dx, dy, +3);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}


						// tilted haar_x4                                   (x, y, w, h, b, weight)
						if ((x + 4 * dx <= winsize.width) && (y + 4 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 4 * dy < s3) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x4",


									x, y, dx * 4, dy, -1,
									x + dx, y + dx, dx * 2, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y4                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 4 * dy <= winsize.height) && (x - 4 * dy >= 0)) {
							if (dx * 4 * dy < s3) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y4",
									x, y, dx, 4 * dy, -1,
									x - dy, y + dy, dx, 2 * dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}


						/*

						// tilted point
						if ( (x+dx*3 <= winsize.width - 1) && (y+dy*3 <= winsize.height - 1) && (x-3*dy>= 0)) {
						if (dx*9*dy < 36) continue;
						if (!symmetric || (x <= (winsize.width / 2) ))  {
						haarFeature = cvHaarFeature( "tilted_haar_point",
						x, y,    dx*3, dy*3, -1,
						x, y+dy, dx  , dy,   +9 );
						CV_WRITE_SEQ_ELEM( haarFeature, writer );
						}
						}
						*/
					}
				}
			}
		}
	}

	//seq = cvEndWriteSeq(&writer);
	features = (CvIntHaarFeatures*)malloc(sizeof(CvIntHaarFeatures) +
		(sizeof(CvTHaarFeature) + sizeof(CvFastHaarFeature)) * seq.size());
	features->feature = (CvTHaarFeature*)(features + 1);
	features->fastfeature = (CvFastHaarFeature*)(features->feature + seq.size());
	features->count = seq.size();
	features->winsize = winsize;
	//	cvCvtSeqToArray(seq, (CvArr*)features->feature);
	for (int i = 0;i < seq.size();i++)
	{
		features->feature[i] = seq[i];
	}
	//	cvReleaseMemStorage(&storage);

	icvConvertToFastHaarFeature(features->feature, features->fastfeature,
		features->count, (winsize.width + 1));

	return features;
}
/*
*加速特征计算
*/
void icvConvertToFastHaarFeature(CvTHaarFeature* haarFeature,
	CvFastHaarFeature* fastHaarFeature,
	int size, int step)
{
	int i = 0;
	int j = 0;

	for (i = 0; i < size; i++)
	{
		fastHaarFeature[i].tilted = haarFeature[i].tilted;
		if (!fastHaarFeature[i].tilted)
		{
			for (j = 0; j < CV_HAAR_FEATURE_MAX; j++)
			{
				fastHaarFeature[i].rect[j].weight = haarFeature[i].rect[j].weight;
				if (fastHaarFeature[i].rect[j].weight == 0.0F)
				{
					break;
				}
				CV_SUM_OFFSETS(fastHaarFeature[i].rect[j].p0,
					fastHaarFeature[i].rect[j].p1,
					fastHaarFeature[i].rect[j].p2,
					fastHaarFeature[i].rect[j].p3,
					haarFeature[i].rect[j].r, step)
			}

		}
		else
		{
			for (j = 0; j < CV_HAAR_FEATURE_MAX; j++)
			{
				fastHaarFeature[i].rect[j].weight = haarFeature[i].rect[j].weight;
				if (fastHaarFeature[i].rect[j].weight == 0.0F)
				{
					break;
				}
				CV_TILTED_OFFSETS(fastHaarFeature[i].rect[j].p0,
					fastHaarFeature[i].rect[j].p1,
					fastHaarFeature[i].rect[j].p2,
					fastHaarFeature[i].rect[j].p3,
					haarFeature[i].rect[j].r, step);
			}
		}
	}
}
/*
* icvCreateHaarTrainingData
*
* Create haar training data used in stage training
*/
static
CvHaarTrainigData* icvCreateHaarTrainingData(MySize winsize, int maxnumsamples)
{
	CvHaarTrainigData* data;
	data = NULL;
	uchar* ptr = NULL;
	size_t datasize = 0;

	datasize = sizeof(CvHaarTrainigData) +
		/* sum and tilted */
		(2 * (winsize.width + 1) * (winsize.height + 1) * sizeof(sum_type) +
			sizeof(float) +      /* normfactor */
			sizeof(float) +      /* cls */
			sizeof(float)        /* weight */
			) * maxnumsamples;
	data = (CvHaarTrainigData*)malloc(datasize);
	memset((void*)data, 0, datasize);

	data->maxnum = maxnumsamples;
	data->winsize = winsize;
	ptr = (uchar*)(data + 1);

	data->sum = myMat(maxnumsamples, (winsize.width + 1) * (winsize.height + 1), ONE_CHANNEL, INT_TYPE, (void*)ptr);
	ptr += sizeof(sum_type) * maxnumsamples * (winsize.width + 1) * (winsize.height + 1);

	data->tilted = myMat(maxnumsamples, (winsize.width + 1) * (winsize.height + 1), ONE_CHANNEL, INT_TYPE, (void*)ptr);
	ptr += sizeof(sum_type) * maxnumsamples * (winsize.width + 1) * (winsize.height + 1);

	data->normfactor = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);
	ptr += sizeof(float) * maxnumsamples;

	data->cls = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);
	ptr += sizeof(float) * maxnumsamples;

	data->weights = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);

	data->valcache = NULL;
	data->idxcache = NULL;

	return data;
}
typedef struct CvBackgroundData
{
	int    count;
	char** filename;
	int    last;
	int    round;
	MySize winsize;
} CvBackgroundData;
/*负图片*/
CvBackgroundData* cvbgdata = NULL;          //记住要释放
/*正图片*/
CvBackgroundData* cvposdata = NULL;      
/*样本获取计数*/
int trainingdata_number = 0;

static
CvBackgroundData* icvCreateBackgroundData(const char* filename,MySize winsize)
{
	CvBackgroundData* data = NULL;

	const char* dir = NULL;
	char full[PATH_MAX];
	char* imgfilename = NULL;
	size_t datasize = 0;
	int    count = 0;
	FILE*  input = NULL;
	char*  tmp = NULL;
	int    len = 0;

	assert(filename != NULL);

	dir = strrchr(filename, '\\');
	if (dir == NULL)
	{
		dir = strrchr(filename, '/');
	}
	if (dir == NULL)
	{
		imgfilename = &(full[0]);
	}
	else
	{
		strncpy(&(full[0]), filename, (dir - filename + 1));
		imgfilename = &(full[(dir - filename + 1)]);
	}

	input = fopen(filename, "r");
	if (input != NULL)
	{
		count = 0;
		datasize = 0;

		/* count */
		while (!feof(input))
		{
			*imgfilename = '\0';
			if (!fgets(imgfilename, PATH_MAX - (int)(imgfilename - full) - 1, input))
				break;
			len = (int)strlen(imgfilename);
			for (; len > 0 && isspace(imgfilename[len - 1]); len--)
				imgfilename[len - 1] = '\0';
			if (len > 0)
			{
				if ((*imgfilename) == '#') continue; /* comment */
				count++;
				datasize += sizeof(char) * (strlen(&(full[0])) + 1);
			}
		}
		if (count > 0)
		{
			//rewind( input );
			fseek(input, 0, SEEK_SET);
			datasize += sizeof(*data) + sizeof(char*) * count;
			data = (CvBackgroundData*)malloc(datasize);
			memset((void*)data, 0, datasize);
			data->count = count;
			data->filename = (char**)(data + 1);
			data->last = 0;
			data->round = 0;
			data->winsize = winsize;
			tmp = (char*)(data->filename + data->count);
			count = 0;
			while (!feof(input))
			{
				*imgfilename = '\0';
				if (!fgets(imgfilename, PATH_MAX - (int)(imgfilename - full) - 1, input))
					break;
 				len = (int)strlen(imgfilename);
				if (len > 0 && imgfilename[len - 1] == '\n')
					imgfilename[len - 1] = 0, len--;
				if (len > 0)
				{
					if ((*imgfilename) == '#') continue; /* comment */
					data->filename[count++] = tmp;
					strcpy(tmp, &(full[0]));
					tmp += strlen(&(full[0])) + 1;
				}
			}
		}
		fclose(input);
	}

	return data;
}
/*
* icvInitBackgroundReaders
*
* Initialize background reading process.
* <cvbgreader> and <cvbgdata> are initialized.
* Must be called before any usage of background
*
* filename - name of background description file
* winsize  - size of images will be obtained from background
*
* return 1 on success, 0 otherwise.
*/
static
int icvInitBackgroundReaders(const char* filename, MySize winsize)
{
	if (cvbgdata == NULL && filename != NULL)
	{
		cvbgdata = icvCreateBackgroundData(filename, winsize);
	}
	return (cvbgdata != NULL);
}
static
int icvInitPostiveReaders(const char* filename, MySize winsize)
{
	if (cvposdata == NULL && filename != NULL)
	{
		cvposdata = icvCreateBackgroundData(filename, winsize);
	}
	return (cvposdata != NULL);
}
/*
*替换图片，用于级联
*num_pos_all,用于判断指示当前图片从正样本取还是负样本
*/
static void replacePictrue(CvHaarTrainigData* training_data, int num_pos_all,int postion,int replace_postion, MySize mysize)
{
	MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //注意最后要释放	
	MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	if(postion >= num_pos_all)
		tempMat = transMat(tempMat, cvbgdata->filename[replace_postion]);
	else
		tempMat = transMat(tempMat, cvposdata->filename[replace_postion]);
	if (tempMat != nullptr)
	{
		//计算积分图
		//		myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
		int *address = training_data->sum.data.i;
		GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
		//GetGraySqImage(tempMat->data.ptr, tempSqsum->data.i, mysize.width, mysize.height, tempMat->step);
		//积分图复制到training_data中
		address = postion * (mysize.width + 1)*(mysize.height + 1) + address;
		memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));

	}
	releaseMyMat(tempMat);
	releaseMyMat(tempSum);

}
/*
*计算特征值
*/
float cvEvalFastHaarFeature2(const CvFastHaarFeature feature,
	const sum_type* sum, const sum_type* tilted)
{
	const sum_type* img = feature.tilted ? tilted : sum;
	float ret = feature.rect[0].weight*
		(img[feature.rect[0].p0] - img[feature.rect[0].p1] -
			img[feature.rect[0].p2] + img[feature.rect[0].p3]) +
		feature.rect[1].weight*
		(img[feature.rect[1].p0] - img[feature.rect[1].p1] -
			img[feature.rect[1].p2] + img[feature.rect[1].p3]);

	if (feature.rect[2].weight != 0.0f)
		ret += feature.rect[2].weight *
		(img[feature.rect[2].p0] - img[feature.rect[2].p1] -
			img[feature.rect[2].p2] + img[feature.rect[2].p3]);
	return ret;
}
/*
*对图片进行预测 级联检测
*/
static
int predictSignal(CvIntHaarFeatures* haarFeatures, MyMat* pic, MySize size, MyCascadeClassifier classifier)
{
	float scal = MAX(classifier.size.width / size.width * 1.0, classifier.size.height / size.height * 1.0, );
	float h_scale_rate = size.height * 1.0 / classifier.size.height;
	float w_scale_rate = size.width * 1.0 / classifier.size.width;
	int strong_number = classifier.StrongClassifier.size();

	for (int i = 0;i < strong_number;i++)
	{
		/*
		*计算加和 0.5 * (a1 + a2 + a3 +...)
		*/
		float val = 0.0f;
		float predictSum = 0.0;
		int length = classifier.StrongClassifier[i].classifier.size();
		vector<float> at;
		float sum = 0.0f;
		for (int j = 0;j < length;j++)
		{
			float error = classifier.StrongClassifier[i].classifier[j].error;
			float b = error / (1 - error);
			float temp = log(1 / b);
			at.push_back(temp);
			sum = sum + temp;
		}
		sum = sum * classifier.StrongClassifier[i].threshold;
		for (int j = 0;j < classifier.StrongClassifier[i].classifier.size();j++)
		{
			int featureNum = classifier.StrongClassifier[i].classifier[j].compidx;
			//cout << featureNum << " classifier.StrongClassifier[i].classifier[j].compidx" << endl;
			//重新构建haar特征
			CvTHaarFeature haarFeature;
			CvFastHaarFeature fastHaar;
			//特征拷贝 

			haarFeature.tilted = haarFeatures->feature[featureNum].tilted;
			haarFeature.rect[0].r.x = haarFeatures->feature[featureNum].rect[0].r.x * w_scale_rate;
			haarFeature.rect[0].r.y = haarFeatures->feature[featureNum].rect[0].r.y  * h_scale_rate;
			haarFeature.rect[0].r.width = haarFeatures->feature[featureNum].rect[0].r.width * w_scale_rate;
			haarFeature.rect[0].r.height = haarFeatures->feature[featureNum].rect[0].r.height * h_scale_rate;
			haarFeature.rect[0].weight = haarFeatures->feature[featureNum].rect[0].weight;

			haarFeature.rect[1].r.x = haarFeatures->feature[featureNum].rect[1].r.x * w_scale_rate;
			haarFeature.rect[1].r.y = haarFeatures->feature[featureNum].rect[1].r.y  * h_scale_rate;
			haarFeature.rect[1].r.width = haarFeatures->feature[featureNum].rect[1].r.width * w_scale_rate;
			haarFeature.rect[1].r.height = haarFeatures->feature[featureNum].rect[1].r.height * h_scale_rate;
			haarFeature.rect[1].weight = haarFeatures->feature[featureNum].rect[1].weight;

			haarFeature.rect[2].r.x = haarFeatures->feature[featureNum].rect[2].r.x * w_scale_rate;
			haarFeature.rect[2].r.y = haarFeatures->feature[featureNum].rect[2].r.y  * h_scale_rate;
			haarFeature.rect[2].r.width = haarFeatures->feature[featureNum].rect[2].r.width * w_scale_rate;
			haarFeature.rect[2].r.height = haarFeatures->feature[featureNum].rect[2].r.height * h_scale_rate;
			haarFeature.rect[2].weight = haarFeatures->feature[featureNum].rect[2].weight;

			//		cout << haarFeature.rect[0].r.x << ","<<haarFeatures->feature[featureNum].rect[0].r.x << endl;
			icvConvertToFastHaarFeature(&haarFeature, &fastHaar, 1, size.width + 1);
			//cout << fastHaar.rect[0].p0<<","<<haarFeatures->fastfeature[featureNum].rect[0].p0;

			val = cvEvalFastHaarFeature2(fastHaar, pic->data.i, pic->data.i);
			if ((classifier.StrongClassifier[i].classifier[j].left == 1) && (val < classifier.StrongClassifier[i].classifier[j].threshold))
			{
				predictSum = predictSum + at[j];
			}
			else if ((classifier.StrongClassifier[i].classifier[j].right == 1) && (val > classifier.StrongClassifier[i].classifier[j].threshold))
			{
				predictSum = predictSum + at[j];
			}
		}
		if (predictSum >= sum)
		{
			at.clear();
			continue;
		}
		else
		{
			return 0;
		}


	}
	return 1;

}
/*
*加强替换替换图片，用于级联
*num_pos_all,用于判断指示当前图片从正样本取还是负样本
*/
static int replacePictrueStrengthen(CvIntHaarFeatures* haar_features,MyCascadeClassifier classifier,CvHaarTrainigData* training_data, int num_pos_all, int postion, int &replace_postion, MySize mysize)
{
	int labels; //标识当前样本类型
	int *address;
	MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //注意最后要释放	
	MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	if (postion >= num_pos_all)
	{
		tempMat = transMat(tempMat, cvbgdata->filename[replace_postion]);
		labels = 0;  //非人脸
	}		
	else
	{
		tempMat = transMat(tempMat, cvposdata->filename[replace_postion]);
		labels = 1;
	}
    //第一幅直接计算
	if (tempMat != nullptr)
	{
		//计算积分图
		//		myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
		address = training_data->sum.data.i;
		GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
	}
	int result = 1; //需要循环
	//正样本预测正确直接替换 fn
	if ((predictSignal(haar_features, tempSum, classifier.size, classifier) == labels) && (labels == 1))
	{
		address = training_data->sum.data.i;
		address = postion * (mysize.width + 1)*(mysize.height + 1) + address;
		memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
		result = 0;  //终止循环
	}
	//负样本错误检测 fp
	else if ((predictSignal(haar_features, tempSum, classifier.size, classifier) == 1) && (labels == 0))
	{
		int *address = training_data->sum.data.i;
		address = postion * (mysize.width + 1)*(mysize.height + 1) + address;
		memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
		result = 0;
		//cout<<"nn"<<replace_postion << endl;
	}
    while(result)
	{
		replace_postion++;
		//cout << "替换"<<replace_postion << endl;
		if (replace_postion == 70972)
		{
			int mm;
			mm = 1;
		}
		//判断样本是否还存在
		if((postion > num_pos_all)&&(replace_postion>=cvbgdata->count))
		{
			return NEGISOVER;
		}else if ((postion <= num_pos_all) && (replace_postion>=cvposdata->count))
			return POSISOVER;
		if (postion >= num_pos_all)
		{
			tempMat = transMat(tempMat, cvbgdata->filename[replace_postion]);
			labels = 0;  //非人脸
		}
		else
		{
			tempMat = transMat(tempMat, cvposdata->filename[replace_postion]);
			labels = 1;
		}
		if (tempMat != nullptr)
		{
			//计算积分图
			//		myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
			address = training_data->sum.data.i;
			GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
		}
		if ((predictSignal(haar_features, tempSum, classifier.size, classifier) == labels) && (labels == 1))
		{
			address = training_data->sum.data.i;
			address = postion * (mysize.width + 1)*(mysize.height + 1) + address;
			memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
			result = 0;  //终止循环
		}
		//负样本错误检测 fp
		else if ((predictSignal(haar_features, tempSum, classifier.size, classifier) == 1) && (labels == 0))
		{
		//	cout << "nn" << replace_postion << endl;
			int *address = training_data->sum.data.i;
			address = postion * (mysize.width + 1)*(mysize.height + 1) + address;
			memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
			result = 0;
		}
	}

	
	releaseMyMat(tempMat);
	releaseMyMat(tempSum);
	return REPLACESUCCESSFUL;

}

static void getPicture(CvHaarTrainingData* training_data,int *number_all,int number,int flag,MySize mysize)
{
	switch (flag)
	{
	case NEG_FLAG:
	{
		MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //注意最后要释放	
		MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	//  MyMat *tempTitle = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	 //   MyMat *tempSqsum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, DOUBLE_TYPE);//注意最后要释放
		for (int i = 0;i < number;i++)
		{

			int temp = number_all[i];
			tempMat = transMat(tempMat, cvbgdata->filename[temp]);
			if (tempMat != nullptr)
			{
				//计算积分图
		//		myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
				int *address = training_data->sum.data.i;
				GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
				//GetGraySqImage(tempMat->data.ptr, tempSqsum->data.i, mysize.width, mysize.height, tempMat->step);
				//积分图复制到training_data中
				address = trainingdata_number * (mysize.width + 1)*(mysize.height + 1) + address;
				memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
				training_data->cls.data.fl[trainingdata_number] = 0.0;
				trainingdata_number++;
			}

		}
		releaseMyMat(tempMat);
		releaseMyMat(tempSum);
		//	releaseMyMat(tempTitle);
		//releaseMyMat(tempSqsum);
		break;
	}
	case POS_FLAG:
	{
		MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //注意最后要释放	
		MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	//	MyMat *tempTitle = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	//	MyMat *tempSqsum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, DOUBLE_TYPE);//注意最后要释放
		for (int i = 0;i < number;i++)
		{
			
			int temp = number_all[i];
			tempMat = transMat(tempMat, cvposdata->filename[temp]);
			if (tempMat != nullptr)
			{
				//计算积分图
				//myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
				int *address = training_data->sum.data.i;
				GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
				//积分图复制到training_data中
				address = trainingdata_number * (mysize.width + 1)*(mysize.height + 1) + address;
				memcpy(address, tempSum->data.i,sizeof(int)*(mysize.width+1)*(mysize.height +1));
				training_data->cls.data.fl[trainingdata_number] = 1.0;
				trainingdata_number++;
			}

		}
		releaseMyMat(tempMat);
		releaseMyMat(tempSum);
	//	releaseMyMat(tempTitle);
	//	releaseMyMat(tempSqsum);
		break;
	}
	default:
		break;
	}
}
/*
*计算特征值
*/
float cvEvalFastHaarFeature(const CvFastHaarFeature* feature,
	const sum_type* sum, const sum_type* tilted)
{
	const sum_type* img = feature->tilted ? tilted : sum;
	float ret = feature->rect[0].weight*
		(img[feature->rect[0].p0] - img[feature->rect[0].p1] -
			img[feature->rect[0].p2] + img[feature->rect[0].p3]) +
		feature->rect[1].weight*
		(img[feature->rect[1].p0] - img[feature->rect[1].p1] -
			img[feature->rect[1].p2] + img[feature->rect[1].p3]);

	if (feature->rect[2].weight != 0.0f)
		ret += feature->rect[2].weight *
		(img[feature->rect[2].p0] - img[feature->rect[2].p1] -
			img[feature->rect[2].p2] + img[feature->rect[2].p3]);
	return ret;
}


/*
*获得特征值，并保存处理
*numprecalculated 内存限制 后续用
*fileOrMem 特征值存放到文件还是内存 0 内存，1文件
*/
static
void icvPrecalculate(int stage,int num_samples,CvHaarTrainingData* data, CvIntHaarFeatures* haarFeatures,
	int numprecalculated,int fileOrMem,const char* filedirname)
{
	switch (fileOrMem)
	{
	//存入内存计算
	case SAVE_FEATURE_FILE:
	{
		//计算特征值
		char fileName[100];
		char idxfileName[100];
		int val = 0;
		//ofstream *file;
		//file = new ofstream[haarFeatures->count];
		ofstream file;
		ofstream idxfile;
		sprintf(fileName, "%s//feature%d.dat", filedirname, stage);
		sprintf(idxfileName, "%s//idx%d.dat", filedirname, stage);
		file.open(fileName, ios::out|ios::app|ios::binary);
		idxfile.open(idxfileName, ios::out | ios::app | ios::binary);
		int *valarry = new int[num_samples];
		int *idxarry = new int[num_samples];
		for (int i = 0; i < haarFeatures->count; i++)
		{
			for (int j = 0; j < num_samples; j++)
			{
				valarry[j] = cvEvalFastHaarFeature(haarFeatures->fastfeature+i,data->sum.data.i + j * data->sum.width, data->sum.data.i);
				idxarry[j] = j;
			}
			quickSort(valarry,idxarry,0,num_samples - 1);
			for (int j = 0;j < num_samples;j++)
			{
				file.write((char*)&valarry[j], sizeof(int));
				idxfile.write((char*)&idxarry[j], sizeof(int));
			}
		}
		file.close();
		idxfile.close();
		break;
	}
	//存入内存计算
	case SAVE_FEATURE_MEM:
	{
		data->valcache = createMyMat(haarFeatures->count, num_samples, ONE_CHANNEL, INT_TYPE);//注意最后要释放;
		data->idxcache = createMyMat(haarFeatures->count, num_samples, ONE_CHANNEL, INT_TYPE);//注意最后要释放;
		if ((nullptr == data->valcache)||(nullptr == data->idxcache))
		{
			printf("the size of memory is not enough!\n");
			return;
		}

		int val = 0;
		int *valarry = new int[num_samples];
		int *idxarry = new int[num_samples];
		for (int i = 0; i < haarFeatures->count; i++)
		{
			for (int j = 0; j < num_samples; j++)
			{
				data->valcache->data.i[i * data->valcache->width + j] = cvEvalFastHaarFeature(haarFeatures->fastfeature + i, data->sum.data.i + j * data->sum.width, data->sum.data.i);
				data->idxcache->data.i[i * data->valcache->width + j] = j;
			}
			quickSort(data->valcache->data.i, data->idxcache->data.i, 0, num_samples - 1);
		}
		break;
	}
	default:
		break;
	}
}
/*
*更新权重
*/
static void icvSetWeightsAndClasses(CvHaarTrainingData* training_data,
	int num1, float weight1, float cls1,
	int num2, float weight2, float cls2)
{
	int j;

	assert(num1 + num2 <= training_data->maxnum);

	for (j = 0; j < num1; j++)
	{
		training_data->weights.data.fl[j] = weight1;
		training_data->cls.data.fl[j] = cls1;
	}
	for (j = num1; j < num1 + num2; j++)
	{
		training_data->weights.data.fl[j] = weight2;
		training_data->cls.data.fl[j] = cls2;
	}
}
static 
void saveXML(CvIntHaarFeatures* haarFeatures,int stage,vector<MyStumpClassifier> strongClassifier,const char* dirname,float stage_thresold)
{
	XMLDocument doc;
	MyStumpClassifier weakClassifier;
	char rootName[100];
	sprintf(rootName,"stage_%d",stage);
	// 创建根元素<China>  
	XMLElement* root = doc.NewElement(rootName);
	doc.InsertEndChild(root);
	root->SetAttribute("stage_thresold", stage_thresold);
	char rectStr[200];
	for (int i = 0;i < strongClassifier.size();i++)
	{
		weakClassifier = strongClassifier[i];

		XMLElement* sonElement = doc.NewElement("weak");
		sonElement->SetAttribute("id", i);
		root->InsertEndChild(sonElement);
		XMLElement* sunElement1 = doc.NewElement("haarfeature");
		/*
		//haar特征标号
		XMLElement* haarNumber = doc.NewElement("number");
		haarNumber->SetText(weakClassifier.compidx);
		sunElement1->InsertEndChild(haarNumber);
		//haar特征描述
		XMLElement* haarDesc = doc.NewElement("desc");
		haarDesc->SetText(haarFeatures->feature[weakClassifier.compidx].desc);
		sunElement1->InsertEndChild(haarDesc);
		XMLElement* haarRect = doc.NewElement("rect");
		
		for (int k = 0;k < CV_HAAR_FEATURE_MAX;k++)
		{
			stringstream ss;
			ss << k;// int->string
			string str = ss.str();
			if (!ss.good())
			{
				printf("矩形id转换错误\n");
			}
			XMLElement* rect = doc.NewElement(str.c_str());
			sprintf(rectStr, "%d %d %d %d %f", haarFeatures->fastfeature[weakClassifier.compidx].rect[k].p0, haarFeatures->fastfeature[weakClassifier.compidx].rect[k].p1,
					haarFeatures->fastfeature[weakClassifier.compidx].rect[k].p2, haarFeatures->fastfeature[weakClassifier.compidx].rect[k].p3,
					haarFeatures->fastfeature[weakClassifier.compidx].rect[k].weight);
			rect->SetText(rectStr);
			haarRect->InsertEndChild(rect);

		}
		
		sunElement1->InsertEndChild(haarRect);
		XMLElement* haarTitle = doc.NewElement("titled");
		haarTitle->SetText(haarFeatures->feature[weakClassifier.compidx].tilted);
		sunElement1->InsertEndChild(haarTitle);
		*/
		sunElement1->SetText(weakClassifier.compidx);
		sonElement->InsertEndChild(sunElement1);
		
		XMLElement* sunElement2 = doc.NewElement("error");
		sunElement2->SetText(weakClassifier.error);
		sonElement->InsertEndChild(sunElement2);
		XMLElement* sunElement3 = doc.NewElement("left");
		sunElement3->SetText(weakClassifier.left);
		sonElement->InsertEndChild(sunElement3);

		XMLElement* sunElement4 = doc.NewElement("right");
		sunElement4->SetText(weakClassifier.right);
		sonElement->InsertEndChild(sunElement4);
		XMLElement* sunElement5 = doc.NewElement("threshold");
		sunElement5->SetText(weakClassifier.threshold);
		sonElement->InsertEndChild(sunElement5);


	}
	char docName[100];
	sprintf(docName, "%s//change_stage%d.xml", dirname, stage);
	doc.SaveFile(docName);
	
	
}
/*
*求解弱分类器累加和
*/
static
float* predict_sum(float* preResultSum, int picturePosNum, CvIntHaarFeatures* haarFeatures, CvHaarTrainigData* haarTrainingData, vector<MyStumpClassifier> strongClassifier)
{
	/*
	*计算加和 0.5 * (a1 + a2 + a3 +...)
	*/
	int length = strongClassifier.size();
	float *at = new float[length];
	float sum = 0.0f;
	for (int i = 0;i < length;i++)
	{
		float error = strongClassifier[i].error;
		float b = error / (1 - error);
		at[i] = log(1 / b);
		sum = sum + at[i];
	}
	/*
	*对图片进行预测分类
	*/
	//sum = sum * 0.5;
	for (int i = 0;i < picturePosNum;i++)
	{
		float val = 0.0f;
		preResultSum[i] = 0.0;
		for (int j = 0;j < length;j++)
		{
			int featureNum = strongClassifier[j].compidx;
			val = cvEvalFastHaarFeature(haarFeatures->fastfeature + featureNum, haarTrainingData->sum.data.i + i * haarTrainingData->sum.width, haarTrainingData->sum.data.i);
			//	if ((strongClassifier[j].left == 1)&&(val < strongClassifier[j].threshold) && (haarTrainingData->cls.data.fl[i] == strongClassifier[j].left))
			if ((strongClassifier[j].left == 1) && (val < strongClassifier[j].threshold))
			{
				preResultSum[i]  += at[j];
			}
			//else if ((strongClassifier[j].right == 1) && (val > strongClassifier[j].threshold) && (haarTrainingData->cls.data.fl[i] == strongClassifier[j].right))
			else if ((strongClassifier[j].right == 1) && (val > strongClassifier[j].threshold))
			{
				preResultSum[i] += at[j];
			}
		}
		preResultSum[i] = preResultSum[i] / sum;

	}
	delete[]at;
	return preResultSum;
}
/*
*对图片进行预测
*/
static
int* predict(int* preResult, float thresold,int pictureNum, CvIntHaarFeatures* haarFeatures, CvHaarTrainigData* haarTrainingData, vector<MyStumpClassifier> strongClassifier)
{
	/*
	*计算加和 0.5 * (a1 + a2 + a3 +...)
	*/
	int length = strongClassifier.size();
	float *at = new float[length];
	float sum = 0.0f;
	for (int i = 0;i < length;i++)
	{
		float error = strongClassifier[i].error;
		float b = error / (1 - error);
		at[i] = log(1 / b);
		sum = sum + at[i];
	}
	/*
	*对图片进行预测分类
	*/
	sum = sum * thresold;
	for (int i = 0;i < pictureNum;i++)
	{
		float val = 0.0f;
		float predictSum = 0.0;
		for (int j = 0;j < length;j++)
		{
			int featureNum = strongClassifier[j].compidx;
			val = cvEvalFastHaarFeature(haarFeatures->fastfeature + featureNum, haarTrainingData->sum.data.i + i * haarTrainingData->sum.width, haarTrainingData->sum.data.i);
			//	if ((strongClassifier[j].left == 1)&&(val < strongClassifier[j].threshold) && (haarTrainingData->cls.data.fl[i] == strongClassifier[j].left))
			if ((strongClassifier[j].left == 1) && (val < strongClassifier[j].threshold))
			{
				predictSum = predictSum + at[j];
				/*
				if (strongClassifier[j].left == 1)
				cout << "i:" << i << ",j:" << j << ",face" << endl;
				else
				cout << "i:" << i << ",j:" << j << ",non-face" << endl;
				*/

			}
			//else if ((strongClassifier[j].right == 1) && (val > strongClassifier[j].threshold) && (haarTrainingData->cls.data.fl[i] == strongClassifier[j].right))
			else if ((strongClassifier[j].right == 1) && (val > strongClassifier[j].threshold))
			{
				predictSum = predictSum + at[j];
				/*
				if (strongClassifier[j].left == 1)
				cout << "i:" << i << ",j:" << j << ",face" << endl;
				else
				cout << "i:" << i << ",j:" << j << ",non-face" << endl;
				*/
			}
		}
		if (predictSum >= sum)
		{
			preResult[i] = 1;
			//	cout << "最终face" << endl;
		}
		else
		{
			preResult[i] = 0;
			//	cout << "最终non-face" << endl;
		}
	}
	delete[]at;
	return preResult;
}
/*
*加强替换替换图片，用于级联
*/
static int replaceNegPictrueStrengthen(CvIntHaarFeatures* haar_features, MyCascadeClassifier classifier, CvHaarTrainigData* training_data, int postion, int &replace_postion, MySize mysize)
{
	int *address = nullptr;
	float normfactor = 0.0;
	MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //注意最后要释放	
	MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	while (true)
	{
		//判断样本是否还存在
		if (replace_postion >= cvbgdata->count)
		{
			replace_postion = 0;
			//return NEGISOVER;
		}
		tempMat = transMat(tempMat, cvbgdata->filename[replace_postion]);
		if (tempMat != nullptr)
		{
			//计算积分图
			//		myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
			address = training_data->sum.data.i;
			GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
		}
		else
		{
			printf("%s is not a picture!\n", cvposdata->filename[replace_postion]);
			replace_postion++;
			continue;
		}
		//负样本错误检测 fp
		if (predictSignal(haar_features, tempSum, classifier.size, classifier) == 1)
		{


			int *address = training_data->sum.data.i;
			address = postion * (mysize.width + 1)*(mysize.height + 1) + address;
			memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
			break;
		}
		replace_postion++;
	}


	releaseMyMat(tempMat);
	releaseMyMat(tempSum);
	return REPLACESUCCESSFUL;

}
/*
*加强替换替换图片，用于级联
*/

static int replacePosPictrueStrengthen(CvIntHaarFeatures* haar_features, MyCascadeClassifier classifier, CvHaarTrainigData* training_data, int postion, int &replace_postion, MySize mysize)
{
	int *address = nullptr;
	MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //注意最后要释放	
	MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	while (true)
	{

		//判断样本是否还存在
		if (replace_postion >= cvposdata->count)
		{
			replace_postion = 0;
		}
		tempMat = transMat(tempMat, cvposdata->filename[replace_postion]);
		if (tempMat != nullptr)
		{
			//计算积分图
			address = training_data->sum.data.i;
			GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
		}
		else
		{
			printf("%s is not a picture!\n", cvposdata->filename[replace_postion]);
			replace_postion++;
			continue;
		}
		//正样检测正确 tp
		if (predictSignal(haar_features, tempSum, classifier.size, classifier) == 1)
		{
			int *address = training_data->sum.data.i;
			address = postion * (mysize.width + 1)*(mysize.height + 1) + address;
			memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
			break;
		}
		replace_postion++;
	}
	releaseMyMat(tempMat);
	releaseMyMat(tempSum);
	return REPLACESUCCESSFUL;

}
static
/*
*读入XML
*/
MyCARTClassifier readXML(const char* xmlPath, MyCARTClassifier &strongClassifier)
{

	XMLDocument doc;
	/*读文件*/
	if (doc.LoadFile(xmlPath))
	{
		doc.PrintError();
		exit(1);
	}
	// 根元素  
	XMLElement* scene = doc.RootElement();

	float th = atof(scene->Attribute("stage_thresold"));
	strongClassifier.threshold = th;
	// 遍历<surface>元素  
	XMLElement* surface = scene->FirstChildElement("weak");
	while (surface)
	{
		MyStumpClassifier tempWeak;
		// 遍历属性列表  
		const XMLAttribute* surfaceAttr = surface->FirstAttribute();
		while (surfaceAttr)
		{
			//	cout << surfaceAttr->Name() << ":" << surfaceAttr->Value() << "  ";
			surfaceAttr = surfaceAttr->Next();
		}

		// 遍历子元素  
		XMLElement* surfaceChild = surface->FirstChildElement();
		while (surfaceChild)
		{
			//	cout << surfaceChild->Name() << " = " << surfaceChild->GetText() << endl;
			if (strcmp(surfaceChild->Name(), HAARFEATUR) == 0)
			{
				tempWeak.compidx = atoi(surfaceChild->GetText());
			}
			else if (strcmp(surfaceChild->Name(), ERRORER) == 0)
			{
				tempWeak.error = atof(surfaceChild->GetText());
			}
			else if (strcmp(surfaceChild->Name(), LEFT) == 0)
			{
				tempWeak.left = atof(surfaceChild->GetText());
			}
			else if (strcmp(surfaceChild->Name(), RIGHT) == 0)
			{
				tempWeak.right = atof(surfaceChild->GetText());
			}
			else if (strcmp(surfaceChild->Name(), THRESHOLD) == 0)
			{
				tempWeak.threshold = atof(surfaceChild->GetText());
			}
			surfaceChild = surfaceChild->NextSiblingElement();
		}
		strongClassifier.classifier.push_back(tempWeak);
		surface = surface->NextSiblingElement("weak");
	}
	return strongClassifier;


}

static
void icvBoost(int maxweaksplits, int stage_all, CvIntHaarFeatures* haarFeatures, CvHaarTrainigData* haarTrainingData,
	const char* featdirname, const char* resultname, int num_pos, int num_neg, int numsplits, int equalweights,
	const char* dirname, float minhitrate, float maxfalsealarms, MySize mysize, int numprecalculated)
{
	float posweight, negweight;
	int feature_size = haarFeatures->count;
	char fileName[100];
	char idxName[100];
	MyCascadeClassifier cascadeClassifier;  //级联分类器
	ifstream istream;
	ifstream idxstream;
	string str;
	float threshold;//阈值
	int sampleNumber = num_pos + num_neg;
	MyStumpClassifier tempWeakClassifier;//弱分类器计算
	vector<MyStumpClassifier> strongClassifier;//强分类器  
	MyStumpClassifier currentWeakClassifier; //当前阶弱分类器
	int *vector_feat = new int[sampleNumber];              //特征向量，记住要释放
	int *idx = new int[sampleNumber];						//特征索引号	
	float hitRate_real = 0;
	float maxFalse_real = 1;
	int pos_next = num_pos;
	int neg_next = num_neg;   //用于收集样本的指针
	double start, end, one, two, three, four, five, six;
	//级联相关参数
	float current_hitrate = 1.0; //当前命中率
	float current_falsealarms = 1.0;//当前虚警率
	vector<float> stagethresold;//阶段分类器阈值
	int stage = 0;  //当前stage数
	int featurenumber = 0;//当前特征数
	float *eval = new float[num_pos]; //特征累积置信值
	int *predit_result = new int[sampleNumber];
	float thresold = 0.5f;
	cascadeClassifier.size.width = mysize.width;
	cascadeClassifier.size.height = mysize.height;
	printf("文件计算\n");
	while (stage < stage_all)
	{

		featurenumber = 0;
		current_falsealarms = 1.0f;
		printf("******%d执行**********\n", stage);
		stagethresold.clear();
		hitRate_real = 0;
		maxFalse_real = 0;
		strongClassifier.clear();
		//收集样本
		if (stage != 0)
		{
			//收集tp，fp样本,也就是替换tn，fn样本
			printf("样本替换\n");
			pos_next = 0;
			neg_next = 0;
			for (int sss = 0;sss < sampleNumber;sss++)
			{
				if (haarTrainingData->cls.data.fl[sss] == 1.0)  //fn
				{
					if (pos_next >= cvposdata->count)
					{
						pos_next = 0;
						printf("正样本已经用完，开始下一轮\n");
					}

					int res = replacePosPictrueStrengthen(haarFeatures, cascadeClassifier, haarTrainingData, sss, pos_next, mysize);
					pos_next++;
				}
				else if (haarTrainingData->cls.data.fl[sss] == 0.0)
				{

					if (neg_next >= cvbgdata->count)
					{
						neg_next = 0;
						printf("负样本已经用完,下一轮开始\n");
					}
					int res = replaceNegPictrueStrengthen(haarFeatures, cascadeClassifier, haarTrainingData, sss, neg_next, mysize);
					neg_next++;
				}

			}
			printf("正样本消耗:%d,负样本消耗:%d\n", pos_next, neg_next);
			//计算当前样本特征值		
			start = clock();
			icvPrecalculate(stage, num_pos + num_neg, haarTrainingData, haarFeatures, numprecalculated, SAVE_FEATURE_FILE, featdirname);
			end = clock();
			printf("特征计算，耗时%f\n", (end - start) / CLOCKS_PER_SEC * 1000);
		}
		//以下用adaboost训练featurenumber个弱分类器
		while (current_falsealarms > maxfalsealarms && (!maxweaksplits || (featurenumber < maxweaksplits)))
		{

			start = clock();
			printf("%d开始\n", featurenumber);
			//文件打开
			sprintf(fileName, "%s//feature%d.dat", featdirname, stage);
			istream.open(fileName, ios::in | ios::binary);
			if (!istream)
			{
				printf("%s打开错误\n", fileName);
				return;
			}
			sprintf(idxName, "%s//idx%d.dat", featdirname, stage);
			idxstream.open(idxName, ios::in | ios::binary);
			if (!idxstream)
			{
				printf("%s打开错误\n", idxName);
				return;
			}
			//更新权重
			if (featurenumber == 0)
			{
				posweight = (equalweights) ? 1.0F / (num_pos + num_neg) : (0.5F / num_pos);
				negweight = (equalweights) ? 1.0F / (num_pos + num_neg) : (0.5F / num_neg);
				icvSetWeightsAndClasses(haarTrainingData,
					num_pos, posweight, 1.0F, num_neg, negweight, 0.0F);
			}
			else
			{
				MyStumpClassifier lastWeakClassifier;
				lastWeakClassifier = strongClassifier[strongClassifier.size() - 1];//取出上一轮弱分类器
				float bt;
				bt = currentWeakClassifier.error / (1.0f - currentWeakClassifier.error);
				float sum = 0.0f;
				for (int ll = 0;ll < sampleNumber;ll++)
				{
					//寻找错误样本
					float val = cvEvalFastHaarFeature(haarFeatures->fastfeature + lastWeakClassifier.compidx, haarTrainingData->sum.data.i + ll * haarTrainingData->sum.width, haarTrainingData->sum.data.i); //计算特征值
					float object = haarTrainingData->cls.data.fl[ll];  //取出当前样本标签
					bool result = false;
					if (val < lastWeakClassifier.threshold)
					{
						if (object == lastWeakClassifier.left)
						{
							result = true;
						}
					}
					else
					{
						if (object == lastWeakClassifier.right)
						{
							result = true;
						}
					}
					if (result)
						haarTrainingData->weights.data.fl[ll] = haarTrainingData->weights.data.fl[ll] * pow(bt, 1);
					else
						haarTrainingData->weights.data.fl[ll] = haarTrainingData->weights.data.fl[ll] * pow(bt, 0);
					sum += haarTrainingData->weights.data.fl[ll];
				}
				//归一化
				for (int ll = 0;ll < sampleNumber;ll++)
				{
					haarTrainingData->weights.data.fl[ll] /= sum;
				}
			}
			//开始计算弱分类器
			int i = 0;//弱分类器循环计数
			int nRead;
			int nNum, nidx;
			int vector_count = 0;
			int end_num = sampleNumber - 1;
			int vector_count_above = 10 * sampleNumber;
			int nIdxRead;
			while (nRead = istream.read((char*)&nNum, sizeof(int)).gcount())
			{
				nIdxRead = idxstream.read((char*)&nidx, sizeof(int)).gcount();  //读入标号
				if (sizeof(int) != nRead)
					break;
				else if (sizeof(int) != nIdxRead)
				{
					break;
				}
				else
				{
					vector_feat[vector_count % sampleNumber] = nNum;
					idx[vector_count % sampleNumber] = nidx;
					//读入一组特征完毕
					//开始训练弱分类器
					if (vector_count % sampleNumber == end_num)
					{
						//	quickSort(vector_feat, idx, 0, sampleNumber - 1);
						//寻找阈值范围
						vector <float> threArray;
						threArray.push_back(vector_feat[0]);
						for (int ii = 0;ii < sampleNumber - 1;ii++)
						{
							if (vector_feat[ii + 1] == vector_feat[ii])
							{
								continue;
							}
							else
							{
								threArray.push_back(vector_feat[ii + 1]);
							}
						}
						//弱分类器训练
						for (int k = 0;k < threArray.size() - 1;k++)
						{
							//循环默认阈值左侧为1，右侧为0 即 左侧人脸
							threshold = 0.5F * (threArray[k] + threArray[k + 1]);    //求取阈值
							float error = 0.0F; //错误率
							float left = 1;  //左侧标签
							float right = 0;  //右侧标签
							float tn = 0.0; //T- 所有负样本权重
							float tp = 0.0; //T+ 所有正样本权重
							float sn = 0.0; //S- 低于阈值负样本权重
							float sp = 0.0; //S+ 低于阈值正样本权重
							for (int n = 0;n < sampleNumber;n++)
							{
								float object = haarTrainingData->cls.data.fl[idx[n]];  //取出当前样本标签

								if (object == 1)
								{
									tp = tp + haarTrainingData->weights.data.fl[idx[n]];   //计算T+
									if (vector_feat[n] < threshold)
										sp += haarTrainingData->weights.data.fl[idx[n]];   //计算S+
								}
								else
								{
									tn += haarTrainingData->weights.data.fl[idx[n]];    //计算T-
									if (vector_feat[n] < threshold)
										sn += haarTrainingData->weights.data.fl[idx[n]];  //计算S-
								}

							}
							//error = (min(s+)+(t-)-(s-),(s-)+(t+)-(s+));，并计算极性
							float errorl = sp + (tn - sn);
							float errorr = sn + (tp - sp);
							if (errorl > errorr)
							{
								error = errorr;
								left = 1;
								right = 0;
							}
							else
							{
								error = errorl;
								left = 0;
								right = 1;
							}
							//寻找最小阈值
							if (k == 0)
							{
								tempWeakClassifier.compidx = i;
								tempWeakClassifier.left = left;
								tempWeakClassifier.right = right;
								tempWeakClassifier.threshold = threshold;
								tempWeakClassifier.error = error;
							}
							else if (error < tempWeakClassifier.error)
							{
								tempWeakClassifier.compidx = i;
								tempWeakClassifier.left = left;
								tempWeakClassifier.right = right;
								tempWeakClassifier.threshold = threshold;
								tempWeakClassifier.error = error;
							}
						}
						if (i == 0)
						{
							currentWeakClassifier.compidx = tempWeakClassifier.compidx;
							currentWeakClassifier.left = tempWeakClassifier.left;
							currentWeakClassifier.right = tempWeakClassifier.right;
							currentWeakClassifier.threshold = tempWeakClassifier.threshold;
							currentWeakClassifier.error = tempWeakClassifier.error;
						}
						else if (tempWeakClassifier.error < currentWeakClassifier.error)
						{
							currentWeakClassifier.compidx = tempWeakClassifier.compidx;
							currentWeakClassifier.left = tempWeakClassifier.left;
							currentWeakClassifier.right = tempWeakClassifier.right;
							currentWeakClassifier.threshold = tempWeakClassifier.threshold;
							currentWeakClassifier.error = tempWeakClassifier.error;
						}
						i++;
					}
					vector_count++;
					if (vector_count == vector_count_above)
					{
						vector_count = 0;
					}
				}
			}

			strongClassifier.push_back(currentWeakClassifier);
			//预测
			eval = predict_sum(eval, num_pos, haarFeatures, haarTrainingData, strongClassifier);
			//eval 存放总置信度的倍数
			quickSort(eval, 0, num_pos - 1);
			thresold = eval[(int)((1.0F - minhitrate) * num_pos)];

			predit_result = predict(predit_result, thresold, sampleNumber, haarFeatures, haarTrainingData, strongClassifier);
			hitRate_real = 0;
			maxFalse_real = 0;
			for (int iii = 0;iii < sampleNumber;iii++)
			{
				if ((predit_result[iii] == 1) && (haarTrainingData->cls.data.fl[iii] == 1.0))
				{
					hitRate_real++;
				}
				else if ((predit_result[iii] == 1) && (haarTrainingData->cls.data.fl[iii] == 0.0))
					maxFalse_real++;
			}
			current_falsealarms = maxFalse_real / num_neg;

			end = clock();
			printf("feature_thresold:%f,falsealarms:%f,hitrate:%f,featurenumber:%d,time:%f\n", thresold,
				current_falsealarms, hitRate_real / num_pos, featurenumber, (end - start) / CLOCKS_PER_SEC * 1000);
			featurenumber++;
			idxstream.close();
			istream.close();
		}              //训练场
		stagethresold.push_back(thresold);
		MyCARTClassifier tempStrongClassifier;
		tempStrongClassifier.classifier = strongClassifier;
		tempStrongClassifier.threshold = thresold;
		cascadeClassifier.StrongClassifier.push_back(tempStrongClassifier);
		printf("stage_thresold:%f,falsealarms:%f,feature_num:%d\n", thresold, current_falsealarms, featurenumber);
		//保存成xml文件
		saveXML(haarFeatures, stage, strongClassifier, dirname, thresold);
		stage++;
	}

	delete[]vector_feat;
	delete[]eval;
	delete[]predit_result;
	delete[]idx;
}
static
void icvBoostInMem(int maxweaksplits, int stage_all, CvIntHaarFeatures* haarFeatures, CvHaarTrainigData* haarTrainingData,
    const char* featdirname, const char* resultname, int num_pos, int num_neg, int numsplits,
	int equalweights, const char* dirname, float minhitrate, float maxfalsealarms, MySize mysize, int numprecalculated)
{
	float posweight, negweight;
	int feature_size = haarFeatures->count;
	MyCascadeClassifier cascadeClassifier;  //级联分类器

	string str;
	float threshold;//阈值
	int sampleNumber = num_pos + num_neg;
	MyStumpClassifier tempWeakClassifier;//弱分类器计算
	vector<MyStumpClassifier> strongClassifier;//强分类器  
	MyStumpClassifier currentWeakClassifier; //当前阶弱分类器
	int *vector_feat;              //特征向量，记住要释放
	int *idx ;						//特征索引号	
	float hitRate_real = 0;
	float maxFalse_real = 1;
	int pos_next = num_pos;
	int neg_next = num_neg;   //用于收集样本的指针
	double start, end, one, two, three, four, five, six;
	//级联相关参数
	float current_hitrate = 1.0; //当前命中率
	float current_falsealarms = 1.0;//当前虚警率
	int stage = 0;  //当前stage数
	int featurenumber = 0;//当前特征数
	float *eval = new float[num_pos]; //特征累积置信值
	int *predit_result = new int[sampleNumber];
	float thresold = 0.5f;
	cascadeClassifier.size.width = mysize.width;
	cascadeClassifier.size.height = mysize.height;
	printf("内存计算\n");
	while (stage < stage_all)
	{

		featurenumber = 0;
		current_falsealarms = 1.0f;
		printf("******%d执行**********\n", stage);

		hitRate_real = 0;
		maxFalse_real = 0;
		strongClassifier.clear();
		//收集样本
		if (stage != 0)
		{
			//收集tp，fp样本,也就是替换tn，fn样本
			printf("样本替换\n");
			pos_next = 0;
			neg_next = 0;
			for (int sss = 0;sss < sampleNumber;sss++)
			{
				if (haarTrainingData->cls.data.fl[sss] == 1.0)  //fn
				{
					if (pos_next >= cvposdata->count)
					{
						pos_next = 0;
						printf("正样本已经用完，开始下一轮\n");
					}

					int res = replacePosPictrueStrengthen(haarFeatures, cascadeClassifier, haarTrainingData, sss, pos_next, mysize);
					pos_next++;
				}
				else if (haarTrainingData->cls.data.fl[sss] == 0.0)
				{

					if (neg_next >= cvbgdata->count)
					{
						neg_next = 0;
						printf("负样本已经用完,下一轮开始\n");
					}
					int res = replaceNegPictrueStrengthen(haarFeatures, cascadeClassifier, haarTrainingData, sss, neg_next, mysize);
					neg_next++;
				}

			}
			printf("正样本消耗:%d,负样本消耗:%d\n", pos_next, neg_next);
			//计算当前样本特征值		
			start = clock();
			icvPrecalculate(stage, num_pos + num_neg, haarTrainingData, haarFeatures, numprecalculated, SAVE_FEATURE_MEM, featdirname);
			end = clock();
			printf("特征计算，耗时%f\n", (end - start) / CLOCKS_PER_SEC * 1000);
		}
		//以下用adaboost训练featurenumber个弱分类器
		while (current_falsealarms > maxfalsealarms && (!maxweaksplits || (featurenumber < maxweaksplits)))
		{

			start = clock();
			printf("%d开始\n", featurenumber);			
			//更新权重
			if (featurenumber == 0)
			{
				posweight = (equalweights) ? 1.0F / (num_pos + num_neg) : (0.5F / num_pos);
				negweight = (equalweights) ? 1.0F / (num_pos + num_neg) : (0.5F / num_neg);
				icvSetWeightsAndClasses(haarTrainingData,
					num_pos, posweight, 1.0F, num_neg, negweight, 0.0F);
			}
			else
			{
				MyStumpClassifier lastWeakClassifier;
				lastWeakClassifier = strongClassifier[strongClassifier.size() - 1];//取出上一轮弱分类器
				float bt;
				bt = currentWeakClassifier.error / (1.0f - currentWeakClassifier.error);
				float sum = 0.0f;
				for (int ll = 0;ll < sampleNumber;ll++)
				{
					//寻找错误样本
					float val = cvEvalFastHaarFeature(haarFeatures->fastfeature + lastWeakClassifier.compidx,
						haarTrainingData->sum.data.i + ll * haarTrainingData->sum.width, haarTrainingData->sum.data.i); //计算特征值
					float object = haarTrainingData->cls.data.fl[ll];  //取出当前样本标签
					bool result = false;
					if (val < lastWeakClassifier.threshold)
					{
						if (object == lastWeakClassifier.left)
						{
							result = true;
						}
					}
					else
					{
						if (object == lastWeakClassifier.right)
						{
							result = true;
						}
					}
					if (result)
						haarTrainingData->weights.data.fl[ll] = haarTrainingData->weights.data.fl[ll] * pow(bt, 1);
					else
						haarTrainingData->weights.data.fl[ll] = haarTrainingData->weights.data.fl[ll] * pow(bt, 0);
					sum += haarTrainingData->weights.data.fl[ll];
				}
				//归一化
				for (int ll = 0;ll < sampleNumber;ll++)
				{
					haarTrainingData->weights.data.fl[ll] /= sum;
				}
			}
			//开始计算弱分类器
			int nRead;
			int nNum, nidx;
			int vector_count = 0;
			int end_num = sampleNumber - 1;
			int vector_count_above = 10 * sampleNumber;
			int nIdxRead;
			for(int weakLine = 0;weakLine < haarFeatures->count;weakLine++)
			{
				   //读入一组特征
					vector_feat = &(haarTrainingData->valcache->data.i[weakLine * haarTrainingData->valcache->width]);
					idx = &(haarTrainingData->idxcache->data.i[weakLine * haarTrainingData->idxcache->width]);					
					//开始训练弱分类器
					//寻找阈值范围
					vector <float> threArray;
					threArray.push_back(vector_feat[0]);
					for (int ii = 0;ii < sampleNumber - 1;ii++)
					{
						if (vector_feat[ii + 1] == vector_feat[ii])
						{
							continue;
						}
						else
						{
							threArray.push_back(vector_feat[ii + 1]);
						}
					}
					//弱分类器训练
					for (int k = 0;k < threArray.size() - 1;k++)
					{
						//循环默认阈值左侧为1，右侧为0 即 左侧人脸
						threshold = 0.5F * (threArray[k] + threArray[k + 1]);    //求取阈值
						float error = 0.0F; //错误率
						float left = 1;  //左侧标签
						float right = 0;  //右侧标签
						float tn = 0.0; //T- 所有负样本权重
						float tp = 0.0; //T+ 所有正样本权重
						float sn = 0.0; //S- 低于阈值负样本权重
						float sp = 0.0; //S+ 低于阈值正样本权重
						for (int n = 0;n < sampleNumber;n++)
						{
							float object = haarTrainingData->cls.data.fl[idx[n]];  //取出当前样本标签

							if (object == 1)
							{
								tp = tp + haarTrainingData->weights.data.fl[idx[n]];   //计算T+
								if (vector_feat[n] < threshold)
									sp += haarTrainingData->weights.data.fl[idx[n]];   //计算S+
							}
							else
							{
								tn += haarTrainingData->weights.data.fl[idx[n]];    //计算T-
								if (vector_feat[n] < threshold)
									sn += haarTrainingData->weights.data.fl[idx[n]];  //计算S-
							}

						}
							//error = (min(s+)+(t-)-(s-),(s-)+(t+)-(s+));，并计算极性
						float errorl = sp + (tn - sn);
						float errorr = sn + (tp - sp);
						if (errorl > errorr)
						{
							error = errorr;
							left = 1;
							right = 0;
						}
						else
						{
							error = errorl;
							left = 0;
							right = 1;
						}
						//寻找最小阈值
						if (k == 0)
						{
							tempWeakClassifier.compidx = weakLine;
							tempWeakClassifier.left = left;
							tempWeakClassifier.right = right;
							tempWeakClassifier.threshold = threshold;
							tempWeakClassifier.error = error;
						}
						else if (error < tempWeakClassifier.error)
						{
							tempWeakClassifier.compidx = weakLine;
							tempWeakClassifier.left = left;
							tempWeakClassifier.right = right;
							tempWeakClassifier.threshold = threshold;
							tempWeakClassifier.error = error;
						}
					}
					if (weakLine == 0)
					{
						currentWeakClassifier.compidx = tempWeakClassifier.compidx;
						currentWeakClassifier.left = tempWeakClassifier.left;
						currentWeakClassifier.right = tempWeakClassifier.right;
						currentWeakClassifier.threshold = tempWeakClassifier.threshold;
						currentWeakClassifier.error = tempWeakClassifier.error;
					}
					else if (tempWeakClassifier.error < currentWeakClassifier.error)
					{
						currentWeakClassifier.compidx = tempWeakClassifier.compidx;
						currentWeakClassifier.left = tempWeakClassifier.left;
						currentWeakClassifier.right = tempWeakClassifier.right;
						currentWeakClassifier.threshold = tempWeakClassifier.threshold;
						currentWeakClassifier.error = tempWeakClassifier.error;
					}
			
				
			}

			strongClassifier.push_back(currentWeakClassifier);
			//预测
			eval = predict_sum(eval, num_pos, haarFeatures, haarTrainingData, strongClassifier);
			//eval 存放总置信度的倍数
			quickSort(eval, 0, num_pos - 1);
			thresold = eval[(int)((1.0F - minhitrate) * num_pos)];

			predit_result = predict(predit_result, thresold, sampleNumber, haarFeatures, haarTrainingData, strongClassifier);
			hitRate_real = 0;
			maxFalse_real = 0;
			for (int iii = 0;iii < sampleNumber;iii++)
			{
				if ((predit_result[iii] == 1) && (haarTrainingData->cls.data.fl[iii] == 1.0))
				{
					hitRate_real++;
				}
				else if ((predit_result[iii] == 1) && (haarTrainingData->cls.data.fl[iii] == 0.0))
					maxFalse_real++;
			}
			current_falsealarms = maxFalse_real / num_neg;

			end = clock();
			printf("feature_thresold:%f,falsealarms:%f,hitrate:%f,featurenumber:%d,time:%f\n", thresold,
				current_falsealarms, hitRate_real/num_pos, featurenumber, (end - start) / CLOCKS_PER_SEC * 1000);
			featurenumber++;

		}             

		MyCARTClassifier tempStrongClassifier;
		tempStrongClassifier.classifier = strongClassifier;
		tempStrongClassifier.threshold = thresold;
		cascadeClassifier.StrongClassifier.push_back(tempStrongClassifier);
		printf("stage_thresold:%f,falsealarms:%f,feature_num:%d\n", thresold, current_falsealarms, featurenumber);
		//保存成xml文件
		saveXML(haarFeatures, stage, strongClassifier, dirname, thresold);
		stage++;
	}
	delete[]eval;
	delete[]predit_result;
}
/*
* 释放空间
*/
static
void icvReleaseHaarTrainingDataCache(CvHaarTrainigData** haarTrainingData)
{
	if (haarTrainingData != NULL && (*haarTrainingData) != NULL)
	{
		if ((*haarTrainingData)->valcache != NULL)
		{
			releaseMyMat((*haarTrainingData)->valcache);
			(*haarTrainingData)->valcache = NULL;
		}
		if ((*haarTrainingData)->idxcache != NULL)
		{
			releaseMyMat((*haarTrainingData)->idxcache);
			(*haarTrainingData)->idxcache = NULL;
		}
	}
}
static
void icvReleaseIntHaarFeatures(CvIntHaarFeatures** intHaarFeatures)
{
	if (intHaarFeatures != NULL && (*intHaarFeatures) != NULL)
	{
		free((*intHaarFeatures));
		(*intHaarFeatures) = NULL;
	}
}

static
void icvReleaseHaarTrainingData(CvHaarTrainigData** haarTrainingData)
{
	if (haarTrainingData != NULL && (*haarTrainingData) != NULL)
	{
		
		icvReleaseHaarTrainingDataCache(haarTrainingData);
		free((*haarTrainingData));
	}
}
static
void icvReleaseBackgroundData(CvBackgroundData** data)
{
	assert(data != NULL && (*data) != NULL);

	free((*data));
}
/*
*合并xml
*/
static 
void combineXml(const char* dirname,MySize winsize)
{
	XMLDocument doc2;
	MyCARTClassifier *strongClassifier = NULL;//强分类器
	char cascadeName[100];
	vector<string> files; //读取xml文件
	//获取文件
	getFiles(dirname, files);
	strongClassifier = new MyCARTClassifier[files.size()];
	XMLElement* root2 = doc2.NewElement("root");
	XMLElement* size = doc2.NewElement("size");
	XMLElement* stageNumber = doc2.NewElement("stage_number");
	char strSzie[20];
	char strStageNumber[20];
	sprintf(strSzie, "%d %d", winsize.width, winsize.height);
	sprintf(strStageNumber, "%d",files.size());
	size->SetText(strSzie);
	stageNumber->SetText(strStageNumber);
	root2->InsertEndChild(size);
	root2->InsertEndChild(stageNumber);
	for (int i = 0;i < files.size();i++)
	{
		strongClassifier[i] = readXML(files[i].c_str(), strongClassifier[i]);
	}
	for (int j = 0;j < files.size();j++)
	{
		char rootName[100];
		sprintf(rootName, "stage_%d", j);
		MyStumpClassifier weakClassifier;
		// 创建根元素<China>  
		XMLElement* root = doc2.NewElement(rootName);
		doc2.InsertEndChild(root);
		root->SetAttribute("stage_thresold", strongClassifier[j].threshold);
		char rectStr[200];
		for (int i = 0;i < strongClassifier[j].classifier.size();i++)
		{
			weakClassifier = strongClassifier[j].classifier[i];

			XMLElement* sonElement = doc2.NewElement("weak");
			sonElement->SetAttribute("id", i);
			root->InsertEndChild(sonElement);
			XMLElement* sunElement1 = doc2.NewElement("haarfeature");

			sunElement1->SetText(weakClassifier.compidx);
			sonElement->InsertEndChild(sunElement1);

			XMLElement* sunElement2 = doc2.NewElement("error");
			sunElement2->SetText(weakClassifier.error);
			sonElement->InsertEndChild(sunElement2);
			XMLElement* sunElement3 = doc2.NewElement("left");
			sunElement3->SetText(weakClassifier.left);
			sonElement->InsertEndChild(sunElement3);

			XMLElement* sunElement4 = doc2.NewElement("right");
			sunElement4->SetText(weakClassifier.right);
			sonElement->InsertEndChild(sunElement4);
			XMLElement* sunElement5 = doc2.NewElement("threshold");
			sunElement5->SetText(weakClassifier.threshold);
			sonElement->InsertEndChild(sunElement5);


		}
		root2->InsertEndChild(root);
	}

	sprintf(cascadeName, "%s\\cascade.xml", dirname);
	doc2.InsertEndChild(root2);
	doc2.SaveFile(cascadeName);
	delete[]strongClassifier;
}
void myHaarTraining(const char* dirname,
	const char* posfilename,
	const char* bgfilename,
	const char* featuredir,
	int npos, int nneg, int nstages,
	int numprecalculated,
	int numsplits,int memcalcu, 
	float minhitrate, float maxfalsealarm,
	float weightfraction,
	int mode, int symmetric,
	int equalweights,
	int winwidth, int winheight,
	int boosttype, int stumperror,
	int maxtreesplits, int minpos, bool bg_vecfile,bool pos_vecfile)
{

	
	

	CvIntHaarFeatures* haar_features = NULL;
	CvHaarTrainingData* training_data = NULL;           //记住要释放空间(已经释放)
	MySize winsize;
	int *number_pos = new int[npos];  //正样本序号集合
	int *number_neg = new int[nneg];  //负样本序号集合         已经释放空间
	winsize = mySize(winwidth, winheight);
	haar_features = icvCreateIntHaarFeatures(winsize, mode, symmetric); // 计算haar特征个数
	printf("Number of features used : %d\n", haar_features->count);
	training_data = icvCreateHaarTrainingData(winsize, npos + nneg); //获取haar特征
	if (!bg_vecfile)
		if (!icvInitBackgroundReaders(bgfilename, winsize) && nstages > 0)
		{
			printf("Unable to read negative images");
			__MY_EXIT__
		}
	if (!pos_vecfile)
		if (!icvInitPostiveReaders(posfilename, winsize) && nstages > 0)
		{
			printf("Unable to read postive images");
			__MY_EXIT__
		}
	double start, read_image_time, calc_feature_time, end;
	std::cout << "程序开始" << endl;
	start = clock();
	//读入图像
	number_pos = getRand(number_pos,0, cvposdata->count - 1,npos);
	number_neg = getRand(number_neg, 0, cvbgdata->count - 1, nneg);
	
	for (int i = 0;i < npos;i++)
	{
		number_pos[i] = i;
	}
	for (int i = 0;i < nneg;i++)
	{
		number_neg[i] = i;
	}
	
	//收集样本 级联要替换样本
	trainingdata_number = 0;
	getPicture(training_data, number_pos,npos,POS_FLAG,winsize);
	getPicture(training_data, number_neg, nneg, NEG_FLAG, winsize);
	read_image_time = clock();
	std::cout << "样本收集耗时：" << (read_image_time - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;

	//boost过程
	//计算特征
    
	if(memcalcu)
		icvPrecalculate(0, npos + nneg, training_data, haar_features, numprecalculated, SAVE_FEATURE_MEM, featuredir);
	else
		icvPrecalculate(0,npos+nneg,training_data, haar_features,numprecalculated, SAVE_FEATURE_FILE, featuredir);
	calc_feature_time = clock();
	std::cout << "特征计算（排序）耗时：" << (calc_feature_time - read_image_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
	
	//开始级联计算
	if(memcalcu)
		icvBoostInMem(maxtreesplits, nstages, haar_features, training_data,
			featuredir, dirname, npos, nneg, numsplits, equalweights, dirname, minhitrate, maxfalsealarm, winsize, numprecalculated);
	else
		icvBoost(maxtreesplits, nstages, haar_features, training_data,
			featuredir, dirname, npos, nneg, numsplits, equalweights, dirname,minhitrate,maxfalsealarm,winsize, numprecalculated);
	
	combineXml(dirname, winsize);

	_MY_END_
	if (cvbgdata != NULL)
	{
		icvReleaseBackgroundData(&cvbgdata);
		cvbgdata = NULL;
	}
	if (cvposdata != NULL)
	{
		icvReleaseBackgroundData(&cvposdata);
		cvposdata = NULL;
	}
	free(number_pos);
	free(number_neg);
	icvReleaseIntHaarFeatures(&haar_features);
	icvReleaseHaarTrainingData(&training_data);
}