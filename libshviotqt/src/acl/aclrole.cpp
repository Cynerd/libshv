#include "aclrole.h"

#include <shv/chainpack/rpcvalue.h>

namespace shv::iotqt::acl {

shv::chainpack::RpcValue AclRole::toRpcValue() const
{
	if(isValid()) {
		shv::chainpack::RpcValue::Map m { {"weight", weight}, };
		if(!roles.empty())
			m["roles"] = shv::chainpack::RpcValue::List::fromStringList(roles);
		if(profile.isMap())
			m["profile"] = profile;
		return m;
	}
	return shv::chainpack::RpcValue();
}

AclRole AclRole::fromRpcValue(const shv::chainpack::RpcValue &v)
{
	AclRole ret;
	if(v.isMap()) {
		const auto &m = v.asMap();
		//ret.name = m.value("name").toString();
		ret.weight = m.value("weight").toInt();
		std::vector<std::string> roles;
		for(const auto &lst : m.value("roles").asList())
			roles.push_back(lst.toString());
		// legacy key for 'roles' was 'grants'
		for(const auto &lst : m.value("grants").asList())
			roles.push_back(lst.toString());
		ret.roles = roles;
		ret.profile = m.value("profile");
		if(!ret.profile.isMap())
			ret.profile = shv::chainpack::RpcValue();
	}
	return ret;
}

} // namespace shv
