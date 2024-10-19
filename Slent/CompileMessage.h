#pragma once

#include "Slent.h"


#define ENGLISH 0
#define KOREAN 1

#define DEFAULT ENGLISH


#define COMPILE_MESSAGE_LANG DEFAULT


#if COMPILE_MESSAGE_LANG == DEFAULT

#define SL0000(token) MessageType::ERROR, string("Unrecognized token \'").append(##token##).append("\'.")
#define SL0001 MessageType::ERROR, "Unexpected macro declear."
#define SL0002 MessageType::ERROR, "Macro name expected."
#define SL0003 MessageType::ERROR, "Missing macro parameters."
#define SL0004 MessageType::ERROR, "Missing macro body."
#define SL0005 MessageType::ERROR, "Unexpected macro parameter syntax."
#define SL0006 MessageType::ERROR, "Macro module name expected."
#define SL0007 MessageType::ERROR, "Macro not found."
#define SL0008 MessageType::ERROR, "Class name missing."
#define SL0009 MessageType::ERROR, "Class naming rule not followed."
#define SL0010 MessageType::ERROR, "Missing class body"
#define SL0011 MessageType::ERROR, "Missing function parameter."
#define SL0012 MessageType::ERROR, "Type expected."
#define SL0013 MessageType::ERROR, "Variable naming rule not followed."
#define SL0014 MessageType::ERROR, "Parameter should only have its type and name."
#define SL0015 MessageType::ERROR, "Access modifier cannot be used inside function."
#define SL0016 MessageType::ERROR, "Variable name expected."
#define SL0017 MessageType::ERROR, "Expression expected."
#define SL0018 MessageType::ERROR, "Missing \';\'."
#define SL0019 MessageType::ERROR, "Function name expected."
#define SL0020 MessageType::ERROR, "Function naming rule not followed."
#define SL0021 MessageType::ERROR, "Return type expected."
#define SL0022 MessageType::ERROR, "Missing function body."
#define SL0023 MessageType::ERROR, "Missing \',\'."
#define SL0024 MessageType::ERROR, "Unexpected operator use."
#define SL0025 MessageType::ERROR, "Constant variable should be initialized when declearing."
#define SL0026 MessageType::ERROR, "Missing \'(\'."
#define SL0027 MessageType::ERROR, "Missing \')\'."
#define SL0028 MessageType::ERROR, "Export target expected."
#define SL0029 MessageType::ERROR, "Module name expected."
#define SL0030 MessageType::ERROR, "Module naming rule not followed."
#define SL0031 MessageType::ERROR, "Missing module body."
#define SL0032 MessageType::ERROR, "Missing \'}\'."
#define SL0033 MessageType::ERROR, "Module not found."
#define SL0034 MessageType::ERROR, "Implicitly typed variable must be initialized."
#define SL0035 MessageType::ERROR, "Unexpected global keyword."
#define SL0036 MessageType::ERROR, "Access modifier missing."



#elif COMPILE_MESSAGE_LANG == KOREAN



#endif