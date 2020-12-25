#ifndef bux_ImplLR1_H_
#define bux_ImplLR1_H_

/*! \file
    This header is constantly included by ParserGen-generated *.cpp files.
*/

#include "ParserBase.h" // bux::FC_GetRelLexT<>, LexBase.h
#include "XAutoPtr.h"   // bux::C_AutoNode<>

namespace bux {
namespace LR1 {

//
//      Types
//
typedef FC_GetRelLexT<I_LexAttr,C_AutoNode> FC_GetRelLex;
typedef C_RetLvalT<I_LexAttr,C_AutoNode> C_RetLval;

//
//      Function Templates
//
using bux::index2value;

} // namespace LR1
} //namespace bux

#endif // bux_ImplLR1_H_
