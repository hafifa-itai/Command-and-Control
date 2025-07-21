#include "pch.h"
#include "ControllerCommandReq.hpp"

ControllerCommandReq::ControllerCommandReq(CommandType commandType, std::wstring szTargetAgent, std::wstring szGroupName, std::wstring szParameters)
{
	this->commandType = commandType;
	this->wszTargetAgent = szTargetAgent;
	this->wszGroupName = szGroupName;
	this->wszParameters = szParameters;
}

ControllerCommandReq::ControllerCommandReq()
{
	this->commandType = CommandType::Unknown;
	this->wszTargetAgent = L"";
	this->wszGroupName = L"";
	this->wszParameters = L"";
}

CommandType ControllerCommandReq::GetCommandType() const
{
	return commandType;
}

std::wstring ControllerCommandReq::GetTargetAgent() const
{
	return wszTargetAgent;
}

std::wstring ControllerCommandReq::GetGroupName() const
{
	return wszGroupName;
}

std::wstring ControllerCommandReq::GetParameters() const
{
	return wszParameters;
}

VOID ControllerCommandReq::SetCommandType(CommandType type) { 
	commandType = type; 
}
VOID ControllerCommandReq::SetTargetAgent(const std::wstring& agent) { 
	wszTargetAgent = agent; 
}

VOID ControllerCommandReq::SetGroupName(const std::wstring& group) {
	wszGroupName = group;
}

VOID ControllerCommandReq::SetParameters(const std::wstring& params) {
	wszParameters = params;
}
