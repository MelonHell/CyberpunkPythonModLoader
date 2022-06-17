#pragma once

#include "stdafx.h"

//struct PyGameObject : RED4ext::CStackType {
//	PyGameObject(RED4ext::CBaseRTTIType *type, RED4ext::ScriptInstance value);
//	RED4ext::CBaseRTTIType *type;
//	RED4ext::ScriptInstance value;
//
//	pybind11::object exec(const std::string& funcName, std::vector<pybind11::object> pyArgs);
//	pybind11::object get(const std::string& propName);
//	void set(const std::string& propName, const pybind11::object& value);
//	[[nodiscard]] std::string getTypeName() const;
//	int getTypeType() const;
//	std::vector<std::string> getFuncs();
//};

RED4ext::CStackType ExecuteFunction(RED4ext::ScriptInstance aInstance, RED4ext::CBaseFunction *aFunc, std::vector<pybind11::handle> pyArgs);
RED4ext::CStackType ExecuteFunction(RED4ext::CStackType object, const std::string& funcName, std::vector<pybind11::handle> pyArgs);
RED4ext::CStackType ExecuteGlobalFunction(const std::string& funcName, std::vector<pybind11::handle> pyArgs);
RED4ext::CStackType GetInstance(const std::string& className);
RED4ext::CStackType UnHandle(RED4ext::CStackType object);
RED4ext::CStackType GetValuePtr(RED4ext::CProperty *prop, RED4ext::ScriptInstance aInstance);
RED4ext::CStackType GetPropValue(RED4ext::CStackType object, const std::string& propName);
void SetPropValue(RED4ext::CStackType object, const std::string& propName, const pybind11::handle& value);
pybind11::handle ToPython(RED4ext::CStackType object);
RED4ext::CStackType FromPython(const pybind11::handle& object, RED4ext::CBaseRTTIType *type);