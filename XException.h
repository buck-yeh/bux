#ifndef XExceptionH
#define XExceptionH

#include "XPlatform.h"  // CUR_FUNC_
#include <stdexcept>    // std::runtime_error, std::logic_error
#include <sstream>      // std::ostringstream

namespace bux {

//
//      Externals
//
/*! \brief
    Causing std::exception to be thrown upon SEH ( Structured Exception Handling ).
    Must be called upon entry of each thread.
*/
void catchSE(bool useOldHookFirst =true);

} // namespace bux

#define THROW_AS(exp_class, msg) {\
    std::ostringstream out_;\
    out_ <<__FILE__ "(" __DATE__ ")#" <<__LINE__ <<' ' <<CUR_FUNC_ <<": " <<msg;\
    throw exp_class(out_.str());\
}
#define THROW_CLASS(exp_class) {\
    std::ostringstream out_;\
    out_ <<__FILE__ "(" __DATE__ ")#" <<__LINE__ <<' ' <<CUR_FUNC_;\
    throw exp_class(out_.str());\
}

#define LOGIC_ERROR(msg) THROW_AS(std::logic_error, msg)
    ///< \brief Wrap __FILE__(__DATE__)\#__LINE__ __FUNCTION__: msg into std::logic_error
#define RUNTIME_ERROR(msg) THROW_AS(std::runtime_error, msg)
    ///< \brief Wrap __FILE__(__DATE__)\#__LINE__ __FUNCTION__: msg into std::runtime_error

#endif // XExceptionH
