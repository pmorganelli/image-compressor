/*
 *      componentVideo.h
 *      by Peter Morganelli and Shepard Rodgers, 10/17/24
 *      arith assignment
 *
 *      This file contains the componentVideo struct, which represents a pixel
 *      in the component video format. A component video pixel has a luminance
 *      value Y, and the two color-difference signals Pb and Pr.
 *  
 */

#ifndef COMPONENT_VIDEO
#define COMPONENT_VIDEO

struct componentVideo {
     float Y, Pb, Pr;
};

#undef COMPONENT_VIDEO
#endif