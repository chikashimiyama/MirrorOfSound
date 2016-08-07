
#pragma once
#include "ofMain.h"

const std::string patchname = "audio.pd";
const int kTargetFPS = 30;
const int kMaxTouch = 8;
const int kWidth = 1980;
const int kHeight = 1024;
const float kCameraSpeed = 0.0008;
const int kKinectHeight = 480;
const int kHalfKinectHeight = kKinectHeight /2;
const float kKinectHeightFloat = static_cast<float>(kKinectHeight);
const float kHalfKinectHeightFloat = static_cast<float>(kHalfKinectHeight);

const int kKinectWidth = 640;
const int kHalfKinectWidth = kKinectWidth / 2;
const float kKinectWidthFloat = static_cast<float>(kKinectWidth);
const float kHalfKinectWidthFloat = static_cast<float>(kHalfKinectWidth);

const int kNumKinectPixels = kKinectHeight * kKinectWidth;

const int kNumTimeSlices = 128;
const float kRNumTimeSlices = 1.0 / static_cast<float>(kNumTimeSlices);
const int kNumBins = 1024;
const int kHalfNumBins = kNumBins / 2;
const int kNumVertices = kNumBins * kNumTimeSlices;
const int kHalfNumVertices = kNumVertices /2;
const int kSampleRate = 44100;
const int kNumInput = 0;
const int kNumOutput = 2;

const float kWidthToBinRatio = static_cast<float>(kKinectWidth) / static_cast<float>(kNumBins);
const float binToWidhRatio = static_cast<float>(kNumBins) / static_cast<float>(kKinectWidth);

const float kEnterThreshold = 25;
const float kExitThreshold = 100;
const float kDistanceBetweenLines = 0.7;
const float kLineSpread = 0.15;
const float kMaxDistance = kDistanceBetweenLines * (kNumTimeSlices-1);
const float kMaxSpread = kLineSpread* (kNumTimeSlices-1);
