//
// Created by isaac on 10/13/17.
//

#ifndef ROUTES_POD_H
#define ROUTES_POD_H

#include <boost/compute.hpp>
#include <math.h>

class pod {
    public:
        static double g()
        {return 9.81;}

        static double maxSlopeGrade()
        {return 0.06;}

        static double maxSpeed()
        {return 339.0;}

        static double minCurveRadius()
        {return pow (maxSpeed(), 2) / 2*g(); }

        static double excavationDepth()
        {return 10.0;}

};


#endif //ROUTES_POD_H
