#pragma once

#include "stdafx.h"
#include <any>

namespace RedPython {

	RED4ext::CStackType ExecuteFunction(RED4ext::ScriptInstance aInstance, RED4ext::CBaseFunction *aFunc, const pybind11::tuple &pyArgs);

	RED4ext::CStackType ExecuteFunction(RED4ext::CStackType object, const std::string &funcName, const pybind11::tuple &pyArgs);

	RED4ext::CStackType ExecuteGlobalFunction(const std::string &funcName, const pybind11::tuple &pyArgs);

	RED4ext::CStackType GetInstance(const std::string &className);

	RED4ext::CStackType UnHandle(RED4ext::CStackType object);

	RED4ext::CStackType GetValuePtr(RED4ext::CProperty *prop, RED4ext::ScriptInstance aInstance);

	RED4ext::CStackType GetPropValue(RED4ext::CStackType object, const std::string &propName);

	void SetPropValue(RED4ext::CStackType object, const std::string &propName, const pybind11::handle &value);

	pybind11::object ToPython(RED4ext::CStackType object);

	RED4ext::CStackType FromPython(const pybind11::handle &object, RED4ext::CBaseRTTIType *type);

}