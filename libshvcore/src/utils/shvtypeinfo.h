#ifndef SHV_CORE_UTILS_SHVLOGTYPEDESCRIPTION_H
#define SHV_CORE_UTILS_SHVLOGTYPEDESCRIPTION_H

#include "../shvcoreglobal.h"

#include <shv/chainpack/rpcvalue.h>

#include <string>
#include <variant>
#include <functional>

namespace shv::chainpack { class MetaMethod; }

namespace shv {
namespace core {
namespace utils {

class SHVCORE_DECL_EXPORT ShvDescriptionBase
{
public:
	ShvDescriptionBase() = default;
	ShvDescriptionBase(const chainpack::RpcValue &v) : m_data(v) {}

	bool isValid() const { return !m_data.asMap().empty(); }
	chainpack::RpcValue dataValue(const std::string &key, const chainpack::RpcValue &default_val = {}) const;

	bool operator==(const ShvDescriptionBase &o) const { return m_data == o.m_data; }
protected:
	void setDataValue(const std::string &key, const chainpack::RpcValue &val);

	void setData(const chainpack::RpcValue &data);
	static void mergeTags(shv::chainpack::RpcValue::Map &map);
protected:
	chainpack::RpcValue m_data;
};

class SHVCORE_DECL_EXPORT ShvFieldDescr : public ShvDescriptionBase
{
	using Super = ShvDescriptionBase;
public:
	ShvFieldDescr() = default;
	ShvFieldDescr(const std::string &n,
				  const std::string &type_name = {},
				  const chainpack::RpcValue &value = {},
				  chainpack::RpcValue::Map &&tags = {});

	std::string name() const;
	std::string typeName() const;
	std::string label() const;
	std::string description() const;
	chainpack::RpcValue value() const;
	std::string visualStyleName() const;
	std::string alarm() const;
	int alarmLevel() const;
	std::string alarmDescription() const { return description(); }

	chainpack::RpcValue toRpcValue() const;
	static ShvFieldDescr fromRpcValue(const chainpack::RpcValue &v);

	shv::chainpack::RpcValue bitfieldValue(uint64_t val) const;
	uint64_t setBitfieldValue(uint64_t bitfield, uint64_t uval) const;
private:
	std::pair<unsigned, unsigned> bitRange() const;
};

// backward compatibility
using ShvLogTypeDescrField = ShvFieldDescr;

class SHVCORE_DECL_EXPORT ShvTypeDescr : public ShvDescriptionBase
{
	using Super = ShvDescriptionBase;
public:
	enum class Type {
		Invalid,
		BitField,
		Enum,
		Bool,
		UInt,
		Int,
		Decimal,
		Double,
		String,
		DateTime,
		List,
		Map,
		IMap,
	};
	enum class SampleType {Invalid = 0, Continuous , Discrete};

	ShvTypeDescr() = default;
	ShvTypeDescr(const std::string &type_name) : ShvTypeDescr(typeFromString(type_name)) {}
	ShvTypeDescr(Type t, SampleType st = SampleType::Continuous, chainpack::RpcValue::Map &&tags = {})
		: ShvTypeDescr(t, std::vector<ShvFieldDescr>(), st, std::move(tags)) {}
	ShvTypeDescr(Type t, std::vector<ShvFieldDescr> &&flds, SampleType st = SampleType::Continuous, chainpack::RpcValue::Map &&tags = {});

	Type type() const;
	ShvTypeDescr& setType(Type t);
	std::vector<ShvFieldDescr> fields() const;
	ShvTypeDescr &setFields(const std::vector<ShvFieldDescr> &fields);
	SampleType sampleType() const; // = SampleType::Continuous;
	ShvTypeDescr &setSampleType(SampleType st);

	bool isValid() const { return type() != Type::Invalid; }

	ShvFieldDescr field(const std::string &field_name) const;
	shv::chainpack::RpcValue fieldValue(const shv::chainpack::RpcValue &val, const std::string &field_name) const;

	static std::string typeToString(Type t);
	static Type typeFromString(const std::string &s);

	static std::string sampleTypeToString(SampleType t);
	static SampleType sampleTypeFromString(const std::string &s);

	int decimalPlaces() const;
	ShvTypeDescr& setDecimalPlaces(int n);

	chainpack::RpcValue toRpcValue() const;
	static ShvTypeDescr fromRpcValue(const chainpack::RpcValue &v);

	chainpack::RpcValue defaultRpcValue() const;
};

using ShvMethodDescr = shv::chainpack::MetaMethod;

class SHVCORE_DECL_EXPORT ShvPropertyDescr : public ShvDescriptionBase
{
	using Super = ShvDescriptionBase;
public:
	ShvPropertyDescr() = default;
	ShvPropertyDescr(const std::string &type_name) { setTypeName(type_name); }

	std::string typeName() const;
	ShvPropertyDescr &setTypeName(const std::string &type_name);
	std::string label() const;
	ShvPropertyDescr &setLabel(const std::string &label);
	std::string description() const;
	ShvPropertyDescr &setDescription(const std::string &description);
	std::string unit() const;
	ShvPropertyDescr &setUnit(const std::string &unit);

	std::string visualStyleName() const;
	ShvPropertyDescr &setVisualStyleName(const std::string &visual_style_name);

	//chainpack::RpcValue blacklist() const;
	//ShvNodeDescr &setBlacklist(chainpack::RpcValue::Map &&black_list);

	std::string alarm() const;
	ShvPropertyDescr &setAlarm(const std::string &alarm);
	int alarmLevel() const;
	std::string alarmDescription() const;

	std::vector<ShvMethodDescr> methods() const;
	ShvMethodDescr method(const std::string &name) const;

	chainpack::RpcValue toRpcValue() const;
	static ShvPropertyDescr fromRpcValue(const chainpack::RpcValue &v, chainpack::RpcValue::Map *extra_tags = nullptr);
};

class SHVCORE_DECL_EXPORT ShvTypeInfo
{
public:
	ShvTypeInfo() = default;

	static ShvTypeInfo fromVersion2(std::map<std::string, ShvTypeDescr> &&types, std::map<std::string, ShvPropertyDescr> &&node_descriptions);

	bool isValid() const { return !(m_types.empty() && m_devicePaths.empty() && m_deviceProperties.empty()); }

	using DeviceProperties = std::map<std::string, ShvPropertyDescr>;
	using DevicePropertiesMap = std::map<std::string, DeviceProperties>;

	const std::map<std::string, std::string>& devicePaths() const { return m_devicePaths; }
	const std::map<std::string, ShvTypeDescr>& types() const { return m_types; }
	const DevicePropertiesMap& deviceProperties() const { return m_deviceProperties; }
	const DeviceProperties& devicePropertyDescriptions(const std::string &device_type) const;
	const std::map<std::string, std::string>& systemPathsRoots() const { return m_systemPathsRoots; }
	const std::map<std::string, shv::chainpack::RpcValue>& extraTags() const { return m_extraTags; }

	ShvTypeInfo& setDevicePath(const std::string &device_path, const std::string &device_type);
	ShvTypeInfo& setDevicePropertyDescription(const std::string &device_path, const std::string &device_type, const std::string &property_path, const ShvPropertyDescr &property_descr);
	ShvTypeInfo& setDevicePropertyDescription(const std::string &device_type, const std::string &property_path, const ShvPropertyDescr &property_descr);
	ShvTypeInfo& setExtraTags(const std::string &node_path, const shv::chainpack::RpcValue &tags);
	shv::chainpack::RpcValue extraTags(const std::string &node_path) const;

	// obsolete
	ShvTypeInfo& setTypeDescription(const ShvTypeDescr &type_descr, const std::string &type_name)
	{
		setTypeDescription(type_name, type_descr);
		return *this;
	}
	ShvTypeInfo& setTypeDescription(const std::string &type_name, const ShvTypeDescr &type_descr);

	ShvPropertyDescr propertyDescriptionForPath(const std::string &shv_path, std::string *p_field_name = nullptr) const;

	std::tuple<std::string, std::string, std::string> findDeviceType(const std::string &shv_path) const;
	std::tuple<std::string, std::string, ShvPropertyDescr> findPropertyDescription(const std::string &device_type, const std::string &property_path) const;
	ShvTypeDescr findTypeDescription(const std::string &type_name) const;

	ShvTypeDescr typeDescriptionForPath(const std::string &shv_path) const;
	shv::chainpack::RpcValue extraTagsForPath(const std::string &shv_path) const;
	std::string findSystemPath(const std::string &shv_path) const;
	bool isPathBlacklisted(const std::string &shv_path) const;
	void setBlacklist(const std::string &shv_path, const chainpack::RpcValue &blacklist);

	struct SHVCORE_DECL_EXPORT PathInfo
	{
		std::string devicePath;
		std::string propertyPath;
		std::string fieldPath;
		std::string deviceType;
		ShvPropertyDescr propertyDescription;
	};
	PathInfo pathInfo(const std::string &shv_path) const;

	chainpack::RpcValue typesAsRpcValue() const;
	chainpack::RpcValue toRpcValue() const;
	static ShvTypeInfo fromRpcValue(const chainpack::RpcValue &v);

	shv::chainpack::RpcValue applyTypeDescription(const shv::chainpack::RpcValue &val, const std::string &type_name, bool translate_enums = true) const;
	shv::chainpack::RpcValue applyTypeDescription(const shv::chainpack::RpcValue &val, const ShvTypeDescr &type_descr, bool translate_enums = true) const;

	void forEachDeviceProperty(const std::string &device_type, std::function<void (const std::string &property_path, const ShvPropertyDescr &node_descr)> fn) const;
	void forEachProperty(std::function<void (const std::string &shv_path, const ShvPropertyDescr &node_descr)> fn) const;
private:
	static ShvTypeInfo fromNodesTree(const chainpack::RpcValue &v);
	void fromNodesTree_helper(const shv::chainpack::RpcValue &node, const std::string &shv_path, const std::string &recent_device_type, const std::string &recent_device_path, const shv::chainpack::RpcValue::Map &node_types);
private:
	std::map<std::string, ShvTypeDescr> m_types; // type-name -> type-description
	std::map<std::string, std::string> m_devicePaths; // path -> device-type
	DevicePropertiesMap m_deviceProperties; // device-property -> node_descr
	std::map<std::string, shv::chainpack::RpcValue> m_extraTags; // shv-path -> tags
	std::map<std::string, std::string> m_systemPathsRoots; // shv-path-root -> system-path
	std::map<std::string, chainpack::RpcValue> m_blacklistedPaths; // shv-path -> blacklist
};

// backward compatibility
using ShvLogMethodDescr = ShvMethodDescr;
using ShvLogFieldDescr = ShvFieldDescr;
using ShvLogTypeDescr = ShvTypeDescr;
using ShvLogNodeDescr = ShvPropertyDescr;
using ShvNodeDescr = ShvPropertyDescr;
using ShvLogTypeInfo = ShvTypeInfo;
using ShvLogPathDescr = ShvPropertyDescr;

} // namespace utils
} // namespace core
} // namespace shv

#endif // SHV_CORE_UTILS_SHVLOGTYPEDESCRIPTION_H
