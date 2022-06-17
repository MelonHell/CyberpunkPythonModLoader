#include "PyGameObject.h"

#include <utility>

PyGameObject::PyGameObject(RED4ext::CBaseRTTIType *type, RED4ext::ScriptInstance value) {
	this->type = type;
	this->value = value;
}

pybind11::object PyGameObject::get(const std::string& propName) {
	return ToPython(GetPropValue(*this, propName));
}

pybind11::object PyGameObject::exec(const std::string& funcName, std::vector<pybind11::object> pyArgs) {
	return ToPython(ExecuteFunction(*this, funcName, std::move(pyArgs)));
}

std::string PyGameObject::getTypeName() const {
	return this->type->GetName().ToString();
}

int PyGameObject::getTypeType() const {
	return static_cast<int>(this->type->GetType());
}

void PyGameObject::set(const std::string& propName, const pybind11::object& propValue) {
	SetPropValue(*this, propName, propValue);
}

std::vector<std::string> PyGameObject::getFuncs() {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto cls = rtti->GetClass(type->GetName());
	RED4ext::DynArray<RED4ext::CBaseFunction*> funcs;
	rtti->GetClassFunctions(funcs);
	std::vector<std::string> result;
	for (const auto &item : funcs) {
		if (item->GetParent() == cls) {
			result.emplace_back(item->fullName.ToString());
		}
	}
	return result;
}

PyGameObject ExecuteFunction(RED4ext::ScriptInstance aInstance, RED4ext::CBaseFunction *aFunc, std::vector<pybind11::object> pyArgs) {

	RED4ext::StackArgs_t aArgs;

	int j = 0;
	for (int i = 0; i < aFunc->params.size; ++i) {
		if (aFunc->params[i]->type->GetName() == "ScriptGameInstance") {
			RED4ext::ScriptGameInstance scriptGameInstance;
			aArgs.push_back({nullptr, &scriptGameInstance});
		} else {
			aArgs.push_back(FromPython(pyArgs[j], aFunc->params[i]->type));
			j++;
		}
	}

	RED4ext::CStackType result;
	if (aFunc->returnType) {
		result.type = aFunc->returnType->type;
		auto allocator = aFunc->returnType->type->GetAllocator();
		auto allocResult = allocator->Alloc(aFunc->returnType->type->GetSize());
		aFunc->returnType->type->Construct(allocResult.memory);
		result.value = allocResult.memory;
	}
	for (size_t i = 0; i < aArgs.size() && i < aFunc->params.size; i++) {
		auto &arg = aArgs[i];
		if (!arg.type) arg.type = aFunc->params[static_cast<uint32_t>(i)]->type;
	}
	RED4ext::CStack stack(aInstance, aArgs.data(), static_cast<uint32_t>(aArgs.size()), aFunc->returnType ? &result : nullptr);
	aFunc->Execute(&stack);
	return UnHandle(result);
}

PyGameObject ExecuteFunction(PyGameObject object, const std::string &funcName, std::vector<pybind11::object> pyArgs) {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto cls = rtti->GetClass(object.type->GetName());
	auto aFunc = cls->GetFunction(funcName.c_str());
	return ExecuteFunction(object.value, aFunc, std::move(pyArgs));
}

PyGameObject GetInstance(const std::string &className) {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto type = rtti->GetClass(className.c_str());
	auto engine = RED4ext::CGameEngine::Get();
	auto game = engine->framework->gameInstance;
	RED4ext::Handle<RED4ext::IScriptable> instance(game->GetInstance(type));
	return UnHandle({instance->GetType(), instance.GetPtr()});
}

PyGameObject ExecuteGlobalFunction(const std::string &funcName, std::vector<pybind11::object> pyArgs) {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto func = rtti->GetFunction(funcName.c_str());
	return ExecuteFunction(GetInstance("cpPlayerSystem").value, func, std::move(pyArgs));
}

PyGameObject UnHandle(RED4ext::CStackType object) {
	if (object.type && object.value) {
		if (object.type->GetType() == RED4ext::ERTTIType::Handle) {
			RED4ext::Handle<RED4ext::IScriptable> handle = *static_cast<RED4ext::Handle<RED4ext::IScriptable> *>(object.value);
			return {handle->GetType(), handle.GetPtr()};
		}
	}
	return {object.type, object.value};
}

PyGameObject GetValuePtr(RED4ext::CProperty *prop, RED4ext::ScriptInstance aInstance) {
	void *holder = aInstance;
	if (prop->flags.b21) {
		using func_t = void *(*)(RED4ext::ScriptInstance);
		RED4ext::RelocFunc<func_t> func(RED4ext::Addresses::IScriptable_GetValueHolder);
		holder = func(aInstance);
	}
	void *value = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(holder) + prop->valueOffset);
	return {prop->type, value};
}

PyGameObject GetPropValue(PyGameObject object, const std::string &propName) {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto cls = rtti->GetClass(object.type->GetName());
	auto prop = cls->GetProperty(propName.c_str());
	return GetValuePtr(prop, object.value);
}

void SetPropValue(PyGameObject object, const std::string &propName, const pybind11::object &value) {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto cls = rtti->GetClass(object.type->GetName());
	auto prop = cls->GetProperty(propName.c_str());
	auto prevValue = GetValuePtr(prop, object.value);
	auto newValue = FromPython(value, prop->type);
	prop->type->Assign(prevValue.value, newValue.value);
}

pybind11::object ToPython(PyGameObject object) {
	if (object.type == nullptr || object.value == nullptr) return pybind11::none();
	if (object.type->GetType() == RED4ext::ERTTIType::Class) return pybind11::cast(object);
	if (object.type->GetType() == RED4ext::ERTTIType::Enum) return pybind11::cast((int32_t *) object.value);
	if (object.type->GetType() == RED4ext::ERTTIType::Fundamental) {
		if (object.type->GetName() == "Bool") return pybind11::cast((bool *) object.value);
		if (object.type->GetName() == "Int8") return pybind11::cast((int8_t *) object.value);
		if (object.type->GetName() == "Uint8") return pybind11::cast((uint8_t *) object.value);
		if (object.type->GetName() == "Int16") return pybind11::cast((int16_t *) object.value);
		if (object.type->GetName() == "Uint16") return pybind11::cast((uint16_t *) object.value);
		if (object.type->GetName() == "Int32") return pybind11::cast((int32_t *) object.value);
		if (object.type->GetName() == "Uint32") return pybind11::cast((uint32_t *) object.value);
		if (object.type->GetName() == "Int64") return pybind11::cast((int64_t *) object.value);
		if (object.type->GetName() == "Uint64") return pybind11::cast((uint64_t *) object.value);
		if (object.type->GetName() == "Float") return pybind11::cast((float *) object.value);
		if (object.type->GetName() == "Double") return pybind11::cast((double *) object.value);
	}
	MessageBoxA(nullptr, (std::string(object.type->GetName().ToString()) + " " + std::to_string((int) object.type->GetType())).c_str(), "Unknown Type", 0);
	return pybind11::none();
}

RED4ext::CStackType FromPython(const pybind11::object &object, RED4ext::CBaseRTTIType *type) {
	if (type->GetType() == RED4ext::ERTTIType::Class) {
		auto pgo = object.cast<PyGameObject>();
		return {pgo.type, pgo.value};
	}
	if (type->GetType() == RED4ext::ERTTIType::Enum) {
		auto value = object.cast<int32_t>();
		return {type, &value};
	}
	if (type->GetType() == RED4ext::ERTTIType::Fundamental) {
		if (type->GetName() == "Bool") {
			auto value = object.cast<bool>();
			return {type, &value};
		}
		if (type->GetName() == "Int8") {
			auto value = object.cast<int8_t>();
			return {type, &value};
		}
		if (type->GetName() == "Bool") {
			auto value = object.cast<bool>();
			return {type, &value};
		}
		if (type->GetName() == "Int8") {
			auto value = object.cast<int8_t>();
			return {type, &value};
		}
		if (type->GetName() == "Uint8") {
			auto value = object.cast<uint8_t>();
			return {type, &value};
		}
		if (type->GetName() == "Int16") {
			auto value = object.cast<int16_t>();
			return {type, &value};
		}
		if (type->GetName() == "Uint16") {
			auto value = object.cast<uint16_t>();
			return {type, &value};
		}
		if (type->GetName() == "Int32") {
			auto value = object.cast<int32_t>();
			return {type, &value};
		}
		if (type->GetName() == "Uint32") {
			auto value = object.cast<uint32_t>();
			return {type, &value};
		}
		if (type->GetName() == "Int64") {
			auto value = object.cast<int64_t>();
			return {type, &value};
		}
		if (type->GetName() == "Uint64") {
			auto value = object.cast<uint64_t>();
			return {type, &value};
		}
		if (type->GetName() == "Float") {
			auto value = object.cast<float>();
			return {type, &value};
		}
		if (type->GetName() == "Double") {
			auto value = object.cast<double>();
			return {type, &value};
		}
	}
	if (type->GetType() == RED4ext::ERTTIType::Simple) {
		if (type->GetName() == "String") {
			auto value = RED4ext::CString(object.cast<std::string>().c_str());
			return {type, &value};
		}
	}
	return {};
}