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

    Pod pod1 = Pod(10);

    std::vector<glm::vec3> points = {{0, 0, 0},
                                     {1, 1, 0},
                                     {2, 2, 0}};

    std::cout << "1" << std::endl;

    // This takes forever, possibly need to offload to the GPU, or perhaps the algorithm should just be changed
    //BOOST_CHECK_CLOSE(pod1.timeForCurve(points), 0.5f, 0.001);

}