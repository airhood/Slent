#pragma once

#include "Slent.h"


#define ENGLISH 0
#define KOREAN 1

#define DEFAULT ENGLISH


#include "SlentSetting.h"


#if COMPILER_MESSAGE_LANG == ENGLISH

#define SL0000E(token) MessageType::ERROR, string("Unrecognized token \'").append(##token##).append("\'.")
#define SL0001E MessageType::ERROR, "Unexpected macro declear."
#define SL0002E MessageType::ERROR, "Macro name expected."
#define SL0003E MessageType::ERROR, "Missing macro parameters."
#define SL0004E MessageType::ERROR, "Missing macro body."
#define SL0005E MessageType::ERROR, "Unexpected macro parameter syntax."
#define SL0006E MessageType::ERROR, "Macro module name expected."
#define SL0007E MessageType::ERROR, "Macro not found."
#define SL0008E MessageType::ERROR, "Class name missing."
#define SL0009E MessageType::ERROR, "Class naming rule not followed."
#define SL0010E MessageType::ERROR, "Missing class body"
#define SL0011E MessageType::ERROR, "Missing function parameter."
#define SL0012E MessageType::ERROR, "Type expected."
#define SL0013E MessageType::ERROR, "Variable naming rule not followed."
#define SL0014E MessageType::ERROR, "Parameter should only have its type and name."
#define SL0015E MessageType::ERROR, "Access modifier cannot be used inside function."
#define SL0016E MessageType::ERROR, "Variable name expected."
#define SL0017E MessageType::ERROR, "Expression expected."
#define SL0018E MessageType::ERROR, "Missing \';\'."
#define SL0019E MessageType::ERROR, "Function name expected."
#define SL0020E MessageType::ERROR, "Function naming rule not followed."
#define SL0021E MessageType::ERROR, "Return type expected."
#define SL0022E MessageType::ERROR, "Missing function body."
#define SL0023E MessageType::ERROR, "Missing \',\'."
#define SL0024E MessageType::ERROR, "Unexpected operator use."
#define SL0025E MessageType::ERROR, "Constant variable should be initialized when declearing."
#define SL0026E MessageType::ERROR, "Missing \'(\'."
#define SL0027E MessageType::ERROR, "Missing \')\'."
#define SL0028E MessageType::ERROR, "Export target expected."
#define SL0029E MessageType::ERROR, "Module name expected."
#define SL0030E MessageType::ERROR, "Module naming rule not followed."
#define SL0031E MessageType::ERROR, "Missing module body."
#define SL0032E MessageType::ERROR, "Missing \'}\'."
#define SL0033E MessageType::ERROR, "Module not found."
#define SL0034E MessageType::ERROR, "Implicitly typed variable must be initialized."
#define SL0035E MessageType::ERROR, "Unexpected global keyword."
#define SL0036E MessageType::ERROR, "Access modifier missing."
#define SL0037E MessageType::ERROR, "dynamic must be used with extern function."
#define SL0038E MessageType::ERROR, "extern load target missing."
#define SL0039E MessageType::ERROR, "extern link target missing."
#define SL0040E MessageType::ERROR, "Functions outside class cannot have access modifier."
#define SL0041E MessageType::ERROR, "'import' keyword can only be used at root scope."
#define SL0042E MessageType::ERROR, "Import module name expected."


#elif COMPILER_MESSAGE_LANG == KOREAN
#define SL0000(token) MessageType::ERROR, string("인식되지 않은 토큰 \'").append(##token##).append("\'.")
#define SL0001 MessageType::ERROR, "올바르지 않은 매크로 선언."
#define SL0002 MessageType::ERROR, "매크로의 이름이 필요합니다.."
#define SL0003 MessageType::ERROR, "매크로 매개변수가 없습니다."
#define SL0004 MessageType::ERROR, "매크로 본문이 없습니다."
#define SL0005 MessageType::ERROR, "올바르지 않은 매크로 매개변수 선언."
#define SL0006 MessageType::ERROR, "매크로 모듈 이름이 필요합니다."
#define SL0007 MessageType::ERROR, "인식되지 않은 매크로."
#define SL0008 MessageType::ERROR, "클래스 이름이 필요합니다."
#define SL0009 MessageType::ERROR, "클래스 명명 규칙을 준수해야 합니다."
#define SL0010 MessageType::ERROR, "클래스 본문이 없습니다."
#define SL0011 MessageType::ERROR, "함수 매개변수가 없습니다."
#define SL0012 MessageType::ERROR, "타입이 필요합니다."
#define SL0013 MessageType::ERROR, "변수 명명 규칙을 준수해야 합니다."
#define SL0014 MessageType::ERROR, "매개변수는 타입과 이름이 있어야 합니다."
#define SL0015 MessageType::ERROR, "접근 제어자는 함수 내에서 사용될 수 없습니다."
#define SL0016 MessageType::ERROR, "변수 이름이 필요합니다."
#define SL0017 MessageType::ERROR, "표현식이 필요합니다."
#define SL0018 MessageType::ERROR, "\';\'이 필요합니다."
#define SL0019 MessageType::ERROR, "함수 이름이 필요합니다."
#define SL0020 MessageType::ERROR, "함수 명명 규칙을 준수해야 합니다."
#define SL0021 MessageType::ERROR, "반환형이 필요합니다."
#define SL0022 MessageType::ERROR, "함수 본문이 없습니다."
#define SL0023 MessageType::ERROR, "\',\'이 필요합니다."
#define SL0024 MessageType::ERROR, "올바르지 않은 연산자 사용."
#define SL0025 MessageType::ERROR, "상수는 선언과 동시에 초기화되어야 합니다."
#define SL0026 MessageType::ERROR, "\'(\'가 필요합니다."
#define SL0027 MessageType::ERROR, "\')\'가 필요합니다."
#define SL0028 MessageType::ERROR, "내보낼 대상이 필요합니다."
#define SL0029 MessageType::ERROR, "모듈 이름이 필요합니다."
#define SL0030 MessageType::ERROR, "모듈 명명 규칙을 준수해야 합니다."
#define SL0031 MessageType::ERROR, "모듈 본문이 없습니다."
#define SL0032 MessageType::ERROR, "\'}\'가 필요합니다."
#define SL0033 MessageType::ERROR, "인식되지 않은 모듈."
#define SL0034 MessageType::ERROR, "타입 추론형 변수는 선언과 동시에 초기화되어야 합니다."
#define SL0035 MessageType::ERROR, "올바르지 않은 글로벌 키워드."
#define SL0036 MessageType::ERROR, "엑세스 한정자가 없습니다."
#define SL0037 MessageType::ERROR, "dynamic 키워드는 extern 키워드와 함께 사용해야 합니다."
#define SL0038 MessageType::ERROR, "외부 코드 로드 대상이 필요합니다."
#define SL0039 MessageType::ERROR, "외부 코드 링크 대상이 필요합니다."
#define SL0040 MessageType::ERROR, "클래스 외부의 함수는 엑세스 한정자를 사용할 수 없습니다."


#endif