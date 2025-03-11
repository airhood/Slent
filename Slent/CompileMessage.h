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
#define SL0000(token) MessageType::ERROR, string("�νĵ��� ���� ��ū \'").append(##token##).append("\'.")
#define SL0001 MessageType::ERROR, "�ùٸ��� ���� ��ũ�� ����."
#define SL0002 MessageType::ERROR, "��ũ���� �̸��� �ʿ��մϴ�.."
#define SL0003 MessageType::ERROR, "��ũ�� �Ű������� �����ϴ�."
#define SL0004 MessageType::ERROR, "��ũ�� ������ �����ϴ�."
#define SL0005 MessageType::ERROR, "�ùٸ��� ���� ��ũ�� �Ű����� ����."
#define SL0006 MessageType::ERROR, "��ũ�� ��� �̸��� �ʿ��մϴ�."
#define SL0007 MessageType::ERROR, "�νĵ��� ���� ��ũ��."
#define SL0008 MessageType::ERROR, "Ŭ���� �̸��� �ʿ��մϴ�."
#define SL0009 MessageType::ERROR, "Ŭ���� ��� ��Ģ�� �ؼ��ؾ� �մϴ�."
#define SL0010 MessageType::ERROR, "Ŭ���� ������ �����ϴ�."
#define SL0011 MessageType::ERROR, "�Լ� �Ű������� �����ϴ�."
#define SL0012 MessageType::ERROR, "Ÿ���� �ʿ��մϴ�."
#define SL0013 MessageType::ERROR, "���� ��� ��Ģ�� �ؼ��ؾ� �մϴ�."
#define SL0014 MessageType::ERROR, "�Ű������� Ÿ�԰� �̸��� �־�� �մϴ�."
#define SL0015 MessageType::ERROR, "���� �����ڴ� �Լ� ������ ���� �� �����ϴ�."
#define SL0016 MessageType::ERROR, "���� �̸��� �ʿ��մϴ�."
#define SL0017 MessageType::ERROR, "ǥ������ �ʿ��մϴ�."
#define SL0018 MessageType::ERROR, "\';\'�� �ʿ��մϴ�."
#define SL0019 MessageType::ERROR, "�Լ� �̸��� �ʿ��մϴ�."
#define SL0020 MessageType::ERROR, "�Լ� ��� ��Ģ�� �ؼ��ؾ� �մϴ�."
#define SL0021 MessageType::ERROR, "��ȯ���� �ʿ��մϴ�."
#define SL0022 MessageType::ERROR, "�Լ� ������ �����ϴ�."
#define SL0023 MessageType::ERROR, "\',\'�� �ʿ��մϴ�."
#define SL0024 MessageType::ERROR, "�ùٸ��� ���� ������ ���."
#define SL0025 MessageType::ERROR, "����� ����� ���ÿ� �ʱ�ȭ�Ǿ�� �մϴ�."
#define SL0026 MessageType::ERROR, "\'(\'�� �ʿ��մϴ�."
#define SL0027 MessageType::ERROR, "\')\'�� �ʿ��մϴ�."
#define SL0028 MessageType::ERROR, "������ ����� �ʿ��մϴ�."
#define SL0029 MessageType::ERROR, "��� �̸��� �ʿ��մϴ�."
#define SL0030 MessageType::ERROR, "��� ��� ��Ģ�� �ؼ��ؾ� �մϴ�."
#define SL0031 MessageType::ERROR, "��� ������ �����ϴ�."
#define SL0032 MessageType::ERROR, "\'}\'�� �ʿ��մϴ�."
#define SL0033 MessageType::ERROR, "�νĵ��� ���� ���."
#define SL0034 MessageType::ERROR, "Ÿ�� �߷��� ������ ����� ���ÿ� �ʱ�ȭ�Ǿ�� �մϴ�."
#define SL0035 MessageType::ERROR, "�ùٸ��� ���� �۷ι� Ű����."
#define SL0036 MessageType::ERROR, "������ �����ڰ� �����ϴ�."
#define SL0037 MessageType::ERROR, "dynamic Ű����� extern Ű����� �Բ� ����ؾ� �մϴ�."
#define SL0038 MessageType::ERROR, "�ܺ� �ڵ� �ε� ����� �ʿ��մϴ�."
#define SL0039 MessageType::ERROR, "�ܺ� �ڵ� ��ũ ����� �ʿ��մϴ�."
#define SL0040 MessageType::ERROR, "Ŭ���� �ܺ��� �Լ��� ������ �����ڸ� ����� �� �����ϴ�."


#endif