#pragma once

#include "pch.h"
#include "Constants.hpp"
#include <nlohmann/json.hpp>

class ControllerCommandReq
{
public:
	ControllerCommandReq(CommandType commandType, std::string szTargetAgent, std::string szGroupName, std::string szParameters);
	ControllerCommandReq();
	CommandType GetCommandType() const;
	std::string GetTargetAgent() const;
	std::string GetGroupName() const;
	std::string GetParameters() const;
    VOID SetCommandType(CommandType type);
    VOID SetTargetAgent(const std::string& agent);
    VOID SetGroupName(const std::string& group);
    VOID SetParameters(const std::string& params);

private:
	CommandType commandType;
	std::string szTargetAgent;
	std::string szGroupName;
	std::string szParameters;
};

inline VOID to_json(nlohmann::json& j, const ControllerCommandReq& req) {
	j["commandType"] = static_cast<int>(req.GetCommandType());
	j["targetAgent"] = req.GetTargetAgent();
	j["groupName"] = req.GetGroupName();
	j["parameters"] = req.GetParameters();
}
inline VOID from_json(const nlohmann::json& j, ControllerCommandReq& req) {
	req.SetCommandType(static_cast<CommandType>(j.value("commandType", CommandType::Unknown)));
	req.SetTargetAgent(j.value("targetAgent", ""));
	req.SetGroupName(j.value("groupName", ""));
	req.SetParameters(j.value("parameters", ""));
}