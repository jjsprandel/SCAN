#include <stdio.h>
#include "unity.h"

void app_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_assign);
    RUN_TEST(test_assign_2);
    RUN_TEST(test_assign_3);
    RUN_TEST(test_messageDelete);
    UNITY_END();
}