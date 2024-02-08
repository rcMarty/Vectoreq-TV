#define DOCTEST_CONFIG_IMPLEMENT // REQUIRED: Enable custom main()
#include <doctest.h>

// write test suite here
TEST_CASE("Vectoring class")
{
    // write test case here
    CHECK(1 == 1);
}

int main(int argc, char **argv)
{
    doctest::Context context;

    // BEGIN:: PLATFORMIO REQUIRED OPTIONS
    context.setOption("success", true);     // Report successful tests
    context.setOption("no-exitcode", true); // Do not return non-zero code on failed test case
    context.setOption("no-run", false);     // Run the test cases
    // END:: PLATFORMIO REQUIRED OPTIONS

    // YOUR CUSTOM DOCTEST OPTIONS
    context.setOption("no-breaks", true); // Skip debugger breaks

    context.applyCommandLine(argc, argv);
    return context.run();
}