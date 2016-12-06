#pragma once

#define IMAGE_WIDTH 50   // This is taken from NUbots/module/input/LinuxCamera/data/config/LinuxCamera.yaml  (rUNSWift is using 1280!!!!!!!!!)
#define IMAGE_HEIGHT 50
#define SURF_SUBSAMPLE 1  // Reduced from 8, since we are using 1/4 of the resolution that rUNSWift is using
#define INIT_SAMPLE 1
#define SURF_HORIZON_WIDTH 1

/**
 * The size of the SURF feature point descriptor in floats, must be even number
 **/
#define SURF_DESCRIPTOR_LENGTH 6

/**
 * The number of scale space octaves used when searching for features
 **/
#define OCTAVES 2

/**
 * The threshold value for interest points
 **/
#define THRESH 325.125f

/**
 * The number of samples taken from within each descriptor window.
 **/
#define SURF_DESCRIPTOR_SAMPLES 5

/**
 * The possible states of the robot
 **/
#define STATE_INITIAL 0
#define STATE_READY   1
#define STATE_PLAYING 2
