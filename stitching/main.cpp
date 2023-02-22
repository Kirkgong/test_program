#include "opencv2/core/core.hpp"
#include "highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/legacy/legacy.hpp"
 
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"
 
#include <iostream>
#include <fstream> 
#include <string>
#include <iomanip> 
using namespace cv;
using namespace std;
using namespace detail;
 
int main(int argc, char** argv)
{   
   vector<Mat> imgs;    //输入图像
   Mat img = imread("1.jpg");
   imgs.push_back(img);
   img = imread("2.jpg");
   imgs.push_back(img);
 
   Ptr<FeaturesFinder> finder;    //特征检测
   finder = new SurfFeaturesFinder();
   vector<ImageFeatures> features(2);
   (*finder)(imgs[0], features[0]);
   (*finder)(imgs[1], features[1]);
 
   vector<MatchesInfo> pairwise_matches;    //特征匹配
   BestOf2NearestMatcher matcher(false, 0.3f, 6, 6);
   matcher(features, pairwise_matches);
 
   HomographyBasedEstimator estimator;    //相机参数评估
   vector<CameraParams> cameras;
   estimator(features, pairwise_matches, cameras);
   for (size_t i = 0; i < cameras.size(); ++i)
   {
      Mat R;
      cameras[i].R.convertTo(R, CV_32F);
      cameras[i].R = R;
   }
 
   Ptr<detail::BundleAdjusterBase> adjuster;    //光束平差法，精确相机参数
   adjuster = new detail::BundleAdjusterReproj();
   adjuster->setConfThresh(1);
   (*adjuster)(features, pairwise_matches, cameras);
 
   vector<Mat> rmats;
   for (size_t i = 0; i < cameras.size(); ++i)
      rmats.push_back(cameras[i].R.clone());
   waveCorrect(rmats, WAVE_CORRECT_HORIZ);    //波形校正
   for (size_t i = 0; i < cameras.size(); ++i)
      cameras[i].R = rmats[i];
 
   //图像映射变换
   vector<Point> corners(2);
   vector<Mat> masks_warped(2);
   vector<Mat> images_warped(2);
   vector<Mat> masks(2);
   for (int i = 0; i < 2; ++i)
   {
      masks[i].create(imgs[i].size(), CV_8U);
      masks[i].setTo(Scalar::all(255));
   }
   Ptr<WarperCreator> warper_creator;
   warper_creator = new cv::PlaneWarper();    //平面投影
   Ptr<RotationWarper> warper = warper_creator->create(static_cast<float>(cameras[0].focal));
   for (int i = 0; i < 2; ++i)
   {
      Mat_<float> K;
      cameras[i].K().convertTo(K, CV_32F); 
      corners[i] = warper->warp(imgs[i], K, cameras[i].R, INTER_LINEAR, BORDER_REFLECT, images_warped[i]);
      warper->warp(masks[i], K, cameras[i].R, INTER_NEAREST, BORDER_CONSTANT, masks_warped[i]);
   }
 
   //创建曝光补偿器，应用增益补偿方法
   Ptr<ExposureCompensator> compensator = 
                     ExposureCompensator::createDefault(ExposureCompensator::GAIN);
   compensator->feed(corners, images_warped, masks_warped);    //得到曝光补偿器
   for(int i=0;i<2;++i)    //应用曝光补偿器，对图像进行曝光补偿
   {
      compensator->apply(i, corners[i], images_warped[i], masks_warped[i]);
   }
 
   imwrite("warped1.jpg", images_warped[0]);    //存储图像
   imwrite("warped2.jpg", images_warped[1]);
 
   return 0;
}