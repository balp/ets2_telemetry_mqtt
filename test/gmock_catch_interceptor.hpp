#ifndef BALP_GMOCK_CATCH_INTERCEPTOR_HPP_
#define BALP_GMOCK_CATCH_INTERCEPTOR_HPP_

#include "gmock/gmock.h"

namespace balp
{
class gmock_catch_interceptor : public ::testing::EmptyTestEventListener
{
  public:
    gmock_catch_interceptor() {}
    virtual ~gmock_catch_interceptor() {}
    // Called after a failed assertion or a SUCCEED() invocation.
    virtual void OnTestPartResult(::testing::TestPartResult const &test_part_result)
    {
        printf("%s in %s:%d\n%s\n",
               test_part_result.failed() ? "*** Failure" : "Success",
               test_part_result.file_name(),
               test_part_result.line_number(),
               test_part_result.summary());
        REQUIRE_FALSE(test_part_result.failed());
    }

    virtual void OnTestProgramStart(const ::testing::UnitTest & /*unit_test*/) {}
    virtual void OnTestIterationStart(const ::testing::UnitTest & /*unit_test*/, int /*iteration*/) {}
    virtual void OnEnvironmentsSetUpStart(const ::testing::UnitTest & /*unit_test*/) {}
    virtual void OnEnvironmentsSetUpEnd(const ::testing::UnitTest & /*unit_test*/) {}
    virtual void OnTestCaseStart(const ::testing::TestCase & /*test_case*/) {}
    virtual void OnTestStart(const ::testing::TestInfo & /*test_info*/) {}
    virtual void OnTestEnd(const ::testing::TestInfo & /*test_info*/) {}
    virtual void OnTestCaseEnd(const ::testing::TestCase & /*test_case*/) {}
    virtual void OnEnvironmentsTearDownStart(const ::testing::UnitTest & /*unit_test*/) {}
    virtual void OnEnvironmentsTearDownEnd(const ::testing::UnitTest & /*unit_test*/) {}
    virtual void OnTestIterationEnd(const ::testing::UnitTest & /*unit_test*/, int /*iteration*/) {}
    virtual void OnTestProgramEnd(const ::testing::UnitTest & /*unit_test*/) {}
};
} // namespace balp

#endif /* BALP_GMOCK_CATCH_INTERCEPTOR_HPP_ */