//
// Created by isaac on 6/21/18.
//

#include <boost/test/unit_test.hpp>
#include <pod/pod.h>

BOOST_AUTO_TEST_CASE(test_min_curve) {

    // make a new pod
    Pod pod1 = Pod(20);
    Pod pod2 = Pod(0);

    BOOST_CHECK_CLOSE(pod1.minCurveRadius(), 40.7747f, 0.01);
    BOOST_CHECK_CLOSE(pod2.minCurveRadius(), 0.0f, 0.01);

}

BOOST_AUTO_TEST_CASE(test_centrip_accel) {

    Pod pod1 = Pod(10);

    BOOST_CHECK_CLOSE(pod1.calcCentripetalAccel({0,0,0}, {1,1,0}, {2,2,0}), 0.0f  , 0.001f);
    BOOST_CHECK_CLOSE(pod1.calcCentripetalAccel({0,0,0}, {1,1,0}, {0,0,0}), 0.0f  , 0.001f);
    BOOST_CHECK_CLOSE(pod1.calcCentripetalAccel({-3,2,0}, {1,-1,0}, {-1,4,0}), 11.2f, 0.1f);

}

BOOST_AUTO_TEST_CASE(test_time_for_curve) {

    float epsilon = 0.001;

    Pod pod1 = Pod(10);

    std::vector<glm::vec3> points1 = {{0, 0, 0},
                                      {1, 1, 0},
                                      {2, 2, 0}};

    std::vector<glm::vec3> points2 = {{0, 0, 0},
                                      {1, 1, 0},
                                      {2, 2, 0},
                                      {3, 3, 0},
                                      {4, 4, 0},
                                      {5, 5, 0},
                                      {6, 6, 0},
                                      {7, 7, 0},
                                      {8, 8, 0},
                                      {9, 9, 0},
                                      {10, 10, 0},
                                      {11, 11, 0},
                                      {12, 12, 0}};

    std::vector<glm::vec3> points3 = {{0, 3, 0},
                                      {1, 1, 0},
                                      {1, 2, 0},
                                      {3, 3, 0},
                                      {4, 5, 0},
                                      {5, 5, 0},
                                      {6, 7, 0},
                                      {8, 10, 0}};

    BOOST_CHECK_CLOSE(pod1.timeForCurve(points1), 0.0, epsilon);
    BOOST_CHECK_CLOSE(pod1.timeForCurve(points2), 1.34803f, epsilon);
    BOOST_CHECK_CLOSE(pod1.timeForCurve(points3), 0.899789f, epsilon);

}