#ifndef bux_XException_H_
#define bux_XException_H_

#include "XPlatform.h"  // CUR_FUNC_
#include <stdexcept>    // std::runtime_error, std::logic_error
#include <fmt/format.h> // fmt::format(), FMT_STRING()

namespace bux {

//
//      Externals
//
#ifdef _WIN32
/*! \brief
    Causing std::exception to be thrown upon SEH ( Structured Exception Handling ).
    Must be called upon entry of each thread.
*/
void catchSE(bool useOldHookFirst =true);
#endif

} // namespace bux

#define THROW_AS(exp_class,fmtStr,...) \
    throw exp_class(fmt::format(std::string{__FILE__ "(" __DATE__ ")#{} {}: "}.append(fmtStr), __LINE__, CUR_FUNC_, ##__VA_ARGS__))
#define THROW_CLASS(exp_class) \
    throw exp_class(fmt::format(FMT_STRING(__FILE__ "(" __DATE__ ")#{} {}: "), __LINE__, CUR_FUNC_))

#define LOGIC_ERROR(fmtStr,...) THROW_AS(std::logic_error, fmtStr, ##__VA_ARGS__)
    ///< \brief Wrap __FILE__(__DATE__)\#__LINE__ __FUNCTION__: msg into std::logic_error
#define RUNTIME_ERROR(fmtStr,...) THROW_AS(std::runtime_error, fmtStr, ##__VA_ARGS__)
    ///< \brief Wrap __FILE__(__DATE__)\#__LINE__ __FUNCTION__: msg into std::runtime_error

#endif // bux_XException_H_
