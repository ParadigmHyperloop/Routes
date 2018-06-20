//
// Created by isaac on 10/13/17.
//


#include "pod.h"

Pod::Pod(float max_speed) {

   // Save the max speed of the pod
   _max_speed = max_speed;

}

float Pod::minCurveRadius() const { return pow (_max_speed, 2.0) / g; }

float Pod::calcCentripetalAccel(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) const {

   float accel = 0.0;

   accel = pow(_max_speed, 2) / Bezier::radiusOfCurvature(p0,p1,p2);

   return accel;
}

float Pod::timeForCurve(const std::vector<glm::vec3>& points) const {

   // The length of the track. This is calculated by adding the straight line distance between each point.
   float length = 0.0;

   /*
    * This will also initialize the accelerations along the track. At the beginning and end of the track, the acceleration is
    * determined by the accel needed to get the pod up to speed. Between these points, the acceleration is the
    * centripetal acceleration at the point. In the future, centripetal accel should be accounted for in the beginning
    * and end of the route.
    */

   std::vector<float> accels;

   float vfSquared = std::pow(DEFAULT_POD_MAX_SPEED, 2);
   float v0Squared = 0;

   float posAccel = (vfSquared - v0Squared) / (2 * ACCEL_DISTANCE);
   float negAccel = -1 * posAccel;

   int firstIndex;
   int secondIndex;

   int toggle = 0;

   for (int i = 0; i < points.size() - 1; i++) {

       if (length < ACCEL_DISTANCE) {
           accels.push_back(posAccel);
       } else if (length > ACCEL_DISTANCE) {
           // We don't deal with the end of the track yet because the full length of the track isnt yet known,
           // this will be corrected after this loop.
           accels.push_back(calcCentripetalAccel(points[i - 1], points[i], points[i + 1]));
       }

       if (length > ACCEL_DISTANCE && toggle == 0) {
           firstIndex = i;
           toggle = 1;
       }

       length = Bezier::bezierLength(points);
   }

   float tempLength = 0.0;

   // Correct accels at end of route
   for (int i = 0; i < points.size(); i++) {

       while (tempLength > length - ACCEL_DISTANCE) {
           accels[i] = negAccel;
       }

       if (tempLength > length - ACCEL_DISTANCE && toggle == 1) {
           secondIndex = i;
           toggle = 0;
       }

       tempLength = Bezier::bezierLength(points);
   }

   // Now we need to find all the points where the acceleration is larger than "g". For now we will ignore the
   // beginning and the end of the route.

   // this is a vector of the indices in the points and accels array where the acceleration is larger than g.
   std::vector<int> accel_too_high;

   // ignore first and last points
   for (int i = firstIndex + 1; i < secondIndex - 1; i++) {
       if (calcCentripetalAccel(points[i - 1], points[i], points[i + 1]) > g) {
           accel_too_high.push_back(i);
       }
   }

   // Now we must go to all these points, using the velocity, and bringing it down to the velocity that it must be
   // in order to make the centripetal acceleration at a good level.

   for (int i = 0; i < accel_too_high.size(); i++) {
       float radiusAtPoint = Bezier::radiusOfCurvature(points[accel_too_high[i] - 1],
                                                       points[accel_too_high[i]],
                                                       points[accel_too_high[i] + 1]);

       float targetVel = glm::sqrt(g * radiusAtPoint);

       float distanceToAccel = (targetVel - _max_speed) / (2 * DE_ACCEL);

       std::vector<glm::vec3> tempPoints;

       for (int i = 0; i < accel_too_high[i]; i++) {
           tempPoints[i] = points[i];
       }

       float distanceFromStart = Bezier::bezierLength(tempPoints);

       float lengthSoFar = 0.0;

       // Correct the Accelerations. After this the accelerations are done.
       for (int i = 0; i < tempPoints.size(); i++) {

           if (lengthSoFar > (distanceFromStart - distanceToAccel)) {
               accels[i] = DE_ACCEL * -1;
           }

           lengthSoFar += glm::length(points[i + 1] - points[i]);
       }


   }

   // We now calculate the time for the pod to traverse the route

   float time = 0.0;

   for (int i = 0; i < accels.size(); i++) {
       time += glm::sqrt((2 * length) / accels[i]);
   }

   return time;

}