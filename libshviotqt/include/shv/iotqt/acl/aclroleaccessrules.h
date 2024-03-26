#pragma once

#include "../shviotqtglobal.h"

#include <shv/chainpack/accessgrant.h>
#include <shv/core/utils/shvpath.h>

namespace shv {
namespace core { namespace utils { class ShvUrl; } }
namespace iotqt {
namespace acl {

struct SHVIOTQT_DECL_EXPORT AclAccessRule
{
public:
	std::string path;
	std::string method;
	std::string access;

	AclAccessRule();
	AclAccessRule(const std::string &path_, const std::string &signal_, const std::string &access_);

	//void setAccess(const std::string &access_, std::optional<int> access_level);
	//static std::pair<std::string, std::optional<chainpack::MetaMethod::AccessLevel>> parseAccess(const std::string &access_, std::optional<int> access_level);

	chainpack::RpcValue toRpcValue() const;
	static AclAccessRule fromRpcValue(const chainpack::RpcValue &rpcval);

	bool isValid() const;
	//bool isMoreSpecificThan(const AclAccessRule &other) const;
	bool isPathMethodMatch(const shv::core::utils::ShvUrl &shv_url, const std::string &signal_) const;
};

class SHVIOTQT_DECL_EXPORT AclRoleAccessRules : public std::vector<AclAccessRule>
{
public:
	shv::chainpack::RpcValue toRpcValue() const;
	shv::chainpack::RpcValue toRpcValue_legacy() const;

	static AclRoleAccessRules fromRpcValue(const shv::chainpack::RpcValue &v);
};

} // namespace acl
} // namespace iotqt
} // namespace shv

