/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef __OPENCV_OBJDETECT_ERFILTER_HPP__
#define __OPENCV_OBJDETECT_ERFILTER_HPP__

#include "er.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> //TODO: remove
#include <vector>
#include <deque>
#include <string>

namespace er
{

/*!
    Base class for 1st and 2nd stages of Neumann and Matas scene text detection algorithms
    Neumann L., Matas J.: Real-Time Scene Text Localization and Recognition, CVPR 2012

    Extracts the component tree (if needed) and filter the extremal regions (ER's) by using a given classifier.
*/
//class ERFilter : public Algorithm
class ERFilter 
{
public:

    //! callback with the classifier is made a class. By doing it we hide SVM, Boost etc.
    class Callback
    {
    public:
        virtual ~Callback() { }
        //! The classifier must return probability measure for the region.
        virtual double eval(const ERStat& stat) = 0; //const = 0; //TODO why cannot use const = 0 here?
    };

    /*!
        the key method. Takes image on input and returns the selected regions in a vector of ERStat
        only distinctive ERs which correspond to characters are selected by a sequential classifier
        \param image   is the input image
        \param regions is output for the first stage, input/output for the second one.
    */
    virtual void run( cv::InputArray image, std::vector<ERStat>& regions ) = 0;


    //! set/get methods to set the algorithm properties,
    virtual void setCallback(const cv::Ptr<ERFilter::Callback>& cb) = 0;
    virtual void setThresholdDelta(int thresholdDelta) = 0;
    virtual void setMinArea(float minArea) = 0;
    virtual void setMaxArea(float maxArea) = 0;
    virtual void setMinProbability(float minProbability) = 0;
    virtual void setMinProbabilityDiff(float minProbabilityDiff) = 0;
    virtual void setNonMaxSuppression(bool nonMaxSuppression) = 0;
    virtual int  getNumRejected() = 0;
};


/*!
    Create an Extremal Region Filter for the 1st stage classifier of N&M algorithm
    Neumann L., Matas J.: Real-Time Scene Text Localization and Recognition, CVPR 2012

    The component tree of the image is extracted by a threshold increased step by step
    from 0 to 255, incrementally computable descriptors (aspect_ratio, compactness,
    number of holes, and number of horizontal crossings) are computed for each ER
    and used as features for a classifier which estimates the class-conditional
    probability P(er|character). The value of P(er|character) is tracked using the inclusion
    relation of ER across all thresholds and only the ERs which correspond to local maximum
    of the probability P(er|character) are selected (if the local maximum of the
    probability is above a global limit pmin and the difference between local maximum and
    local minimum is greater than minProbabilityDiff).

    \param  cb                Callback with the classifier.
                              default classifier can be implicitly load with function loadClassifierNM1()
                              from file in samples/cpp/trained_classifierNM1.xml
    \param  thresholdDelta    Threshold step in subsequent thresholds when extracting the component tree
    \param  minArea           The minimum area (% of image size) allowed for retreived ER's
    \param  minArea           The maximum area (% of image size) allowed for retreived ER's
    \param  minProbability    The minimum probability P(er|character) allowed for retreived ER's
    \param  nonMaxSuppression Whenever non-maximum suppression is done over the branch probabilities
    \param  minProbability    The minimum probability difference between local maxima and local minima ERs
*/
cv::Ptr<ERFilter> createERFilterNM1(const cv::Ptr<ERFilter::Callback>& cb,
                                                  int thresholdDelta = 1, float minArea = 0.00025,
                                                  float maxArea = 0.13, float minProbability = 0.4,
                                                  bool nonMaxSuppression = true,
                                                  float minProbabilityDiff = 0.1);

/*!
    Create an Extremal Region Filter for the 2nd stage classifier of N&M algorithm
    Neumann L., Matas J.: Real-Time Scene Text Localization and Recognition, CVPR 2012

    In the second stage, the ERs that passed the first stage are classified into character
    and non-character classes using more informative but also more computationally expensive
    features. The classifier uses all the features calculated in the first stage and the following
    additional features: hole area ratio, convex hull ratio, and number of outer inflexion points.

    \param  cb             Callback with the classifier
                           default classifier can be implicitly load with function loadClassifierNM2()
                           from file in samples/cpp/trained_classifierNM2.xml
    \param  minProbability The minimum probability P(er|character) allowed for retreived ER's
*/
cv::Ptr<ERFilter> createERFilterNM2(const cv::Ptr<ERFilter::Callback>& cb,
                                                  float minProbability = 0.3);


/*!
    Allow to implicitly load the default classifier when creating an ERFilter object.
    The function takes as parameter the XML or YAML file with the classifier model
    (e.g. trained_classifierNM1.xml) returns a pointer to ERFilter::Callback.
*/
cv::Ptr<ERFilter::Callback> loadClassifierNM1(const std::string& filename);

/*!
    Allow to implicitly load the default classifier when creating an ERFilter object.
    The function takes as parameter the XML or YAML file with the classifier model
    (e.g. trained_classifierNM1.xml) returns a pointer to ERFilter::Callback.
*/
cv::Ptr<ERFilter::Callback> loadClassifierNM2(const std::string& filename);

}
#endif // _OPENCV_ERFILTER_HPP_

