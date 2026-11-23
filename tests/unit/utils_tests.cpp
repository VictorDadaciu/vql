#include "suite_helpers.h"

class utils_tests : public vtl::suite
{
public:
    VTL_TESTS_BEGIN(utils_tests);
    VTL_TEST(alloc_aligned_bytes);
    VTL_TESTS_END();

    void alloc_aligned_bytes()
    {
        vql::aligned_bytes bytes = vql::allocate_aligned_bytes(64);
    }
};

VTL_REGISTER_SUITE(utils_tests);
