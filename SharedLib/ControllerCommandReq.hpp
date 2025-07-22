#pragma once

#include "pch.h"
#include "Constants.hpp"
#include <nlohmann/json.hpp>
#include "StringUtils.hpp"

class ControllerCommandReq
{
public:
	ControllerCommandReq(CommandType commandType, std::wstring szTargetAgent, std::wstring szGroupName, std::wstring szParameters);
	ControllerCommandReq();

    VOID SetCommandType(CommandType type);
    VOID SetTargetAgent(const std::wstring& agent);
    VOID SetGroupName(const std::wstring& group);
    VOID SetParameters(const std::wstring& params);
	CommandType GetCommandType() const;
	std::wstring GetTargetAgent() const;
	std::wstring GetGroupName() const;
	std::wstring GetParameters() const;

private:
	CommandType commandType;
	std::wstring wszTargetAgent;
	std::wstring wszGroupName;
	std::wstring wszParameters;
};

inline VOID to_json(nlohmann::json& j, const ControllerCommandReq& req) {
	j["commandType"] = static_cast<int>(req.GetCommandType());
	j["targetAgent"] = WstringToString(req.GetTargetAgent());
	j["groupName"] = WstringToString(req.GetGroupName());
	j["parameters"] = WstringToString(req.GetParameters());
}
inline VOID from_json(const nlohmann::json& j, ControllerCommandReq& req) {
	req.SetCommandType(static_cast<CommandType>(j.value("commandType", CommandType::Unknown)));
	req.SetTargetAgent(StringToWstring(j.value("targetAgent", "")));
	req.SetGroupName(StringToWstring(j.value("groupName", "")));
	req.SetParameters(StringToWstring(j.value("parameters", "")));
}