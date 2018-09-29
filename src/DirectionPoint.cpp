#include "../include/Piet.h"

using Piet::DirectionPoint;

DirectionPoint Piet::incrementDirectionPointer(DirectionPoint direction) {
    switch (direction) {
        case TopLeft:
            return RightTop;
        case TopRight:
            return RightBottom;
        case RightTop:
            return BottomRight;
        case RightBottom:
            return BottomLeft;
        case BottomLeft:
            return LeftTop;
        case BottomRight:
            return LeftBottom;
        case LeftBottom:
            return TopLeft;
        case LeftTop:
            return TopRight;
        default:
            return TopRight;
    }
};