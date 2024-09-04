#pragma once

#include "Slent.h"


#define ENGLISH 0
#define KOREAN 1

#define DEFAULT ENGLISH


#define COMPILE_MESSAGE_LANG DEFAULT


#if COMPILE_MESSAGE_LANG == DEFAULT

#define SL0001 MessageType::ERROR, "Unexpected preprocessor parameter."
#define SL0002 MessageType::ERROR, "Preprocessor literal bracket should be closed."
#define SL0003 MessageType::ERROR, "Missing preprocessor parameter."
#define SL0004 MessageType::ERROR, "#elif can be used after #if."
#define SL0005 MessageType::ERROR, "Unexpected preprocessor parameter. Literal required."
#define SL0006(command_name) MessageType::ERROR, string("Unsupported preprocessor command.").append(" \'").append(##command_name##).append("\' command doesn't exist")
#define SL0007(token) MessageType::ERROR, string("Unrecognized token \'").append(##token##).append("\'.")
#define SL0008 MessageType::ERROR, "Class naming rule not followed."
#define SL0009 MessageType::ERROR, "Missing class body"
#define SL0010 MessageType::ERROR, "Missing function parameter."
#define SL0011 MessageType::ERROR, "Type expected"
#define SL0012 MessageType::ERROR, "Variable naming rule not followed."
#define SL0013 MessageType::ERROR, "Parameter should only have its type and name."
#define SL0014 MessageType::ERROR, "Access modifier cannot be used inside function."
#define SL0015 MessageType::ERROR, "Variable name expected."
#define SL0016 MessageType::ERROR, "Expression expected."
#define SL0017 MessageType::ERROR, "Missing semicolon."
#define SL0018 MessageType::ERROR, "Function name missing."
#define SL0019 MessageType::ERROR, "Function naming rule not followed."
#define SL0020 MessageType::ERROR, "Return type expected"
#define SL0021 MessageType::ERROR, "Missing function body"
#define SL0022 MessageType::ERROR, "Missing bracket."
#define SL0023 MessageType::ERROR, "Unexpected operator use."
#define SL0024 MessageType::ERROR, "Constant variable should be initialized when declearing."



#elif COMPILE_MESSAGE_LANG == KOREAN



#endif