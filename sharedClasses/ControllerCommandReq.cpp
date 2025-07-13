#include "ControllerCommandReq.hpp"

ControllerCommandReq::ControllerCommandReq(CommandType commandType, std::string szTargetAgent, std::string szGroupName, std::string szParameters)
{
	this->commandType = commandType;
	this->szTargetAgent = szTargetAgent;
	this->szGroupName = szGroupName;
	this->szParameters = szParameters;
}

ControllerCommandReq::ControllerCommandReq()
{
	this->commandType = CommandType::Unknown;
	this->szTargetAgent = "";
	this->szGroupName = "";
	this->szParameters = "";
}

CommandType ControllerCommandReq::GetCommandType() const
{
	return commandType;
}

std::string ControllerCommandReq::GetTargetAgent() const
{
	return szTargetAgent;
}

std::string ControllerCommandReq::GetGroupName() const
{
	return szGroupName;
}

std::string ControllerCommandReq::GetParameters() const
{
	return szParameters;
}

VOID ControllerCommandReq::SetCommandType(CommandType type) { 
	commandType = type; 
}
VOID ControllerCommandReq::SetTargetAgent(const std::string& agent) { 
	szTargetAgent = agent; 
}

VOID ControllerCommandReq::SetGroupName(const std::string& group) {
	szGroupName = group;
}

VOID ControllerCommandReq::SetParameters(const std::string& params) {
	szParameters = params;
}

//VOID to_json(nlohmann::json& j, const ControllerCommandReq& req) {
//	// You can serialize the enum as its string representation for readability
//	j["commandType"] = static_cast<int>(req.GetCommandType());
//	// Or, if you prefer the integer value:
//	// j["commandType"] = static_cast<int>(req.GetCommandType());
//
//	j["targetAgent"] = req.GetTargetAgent();
//	j["groupName"] = req.GetGroupName();
//	j["parameters"] = req.GetParameters();
//}


//VOID from_json(const nlohmann::json& j, ControllerCommandReq& req) {
//	req.SetCommandType(static_cast<CommandType>(j.value("commandType", CommandType::Unknown)));
//	req.SetTargetAgent(j.value("targetAgent", ""));
//	req.SetGroupName(j.value("groupName", ""));
//	req.SetParameters(j.value("parameters", ""));
//}
