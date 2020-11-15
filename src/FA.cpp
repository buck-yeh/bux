#include "FA.h"

namespace bux {

unsigned C_NfaState::nextId =0;

C_NfaState::C_NfaState(): id(nextId++)
{
}

} //namespace bux

