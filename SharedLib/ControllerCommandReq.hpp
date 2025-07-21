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
	j["targetAgent"] = wstring_to_utf8(req.GetTargetAgent());
	j["groupName"] = wstring_to_utf8(req.GetGroupName());
	j["parameters"] = wstring_to_utf8(req.GetParameters());
}
inline VOID from_json(const nlohmann::json& j, ControllerCommandReq& req) {
	req.SetCommandType(static_cast<CommandType>(j.value("commandType", CommandType::Unknown)));
	req.SetTargetAgent(utf8_to_wstring(j.value("targetAgent", "")));
	req.SetGroupName(utf8_to_wstring(j.value("groupName", "")));
	req.SetParameters(utf8_to_wstring(j.value("parameters", "")));
}