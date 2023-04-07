#include <shv/broker/ldap/ldap.h>

#include <QScopeGuard>
#include <QString>
#include <QStringLiteral>

namespace shv::ldap {
std::vector<std::string> getGroupsForUser(const std::unique_ptr<shv::ldap::Ldap>& my_ldap, const std::string_view& base_dn, const std::string_view& field_name, const std::string_view& user_name) {
	std::vector<std::string> res;
	auto filter = QStringLiteral(R"(%1=%2)").arg(field_name.data()).arg(user_name.data());
	auto entries = my_ldap->search(base_dn.data(), qPrintable(filter), {"memberOf"});
	for (const auto& entry : entries) {
		for (const auto& [key, values]: entry.keysAndValues) {
			for (const auto& v : values) {
				res.emplace_back(v);
				auto derived_groups = getGroupsForUser(my_ldap, v, "objectClass", "*");
				std::copy(derived_groups.begin(), derived_groups.end(), std::back_inserter(res));
			}
		}
	}

	return res;
}

namespace {
template <typename Type, typename DeleteFunc>
auto wrap(Type* ptr, DeleteFunc delete_func)
{
	return std::unique_ptr<Type, decltype(delete_func)>(ptr, delete_func);
}

void throwIfError(int code, const std::string& err_msg)
{
	if (code) {
		throw LdapError(err_msg + ": " + ldap_err2string(code) + " (" + std::to_string(code) + ")", code);
	}
}
}

LdapError::LdapError(const std::string& err_string, int code)
	: std::runtime_error(err_string)
	, m_code(code)
{
}

int LdapError::code() const
{
	return m_code;
}

Ldap::Ldap(LDAP* conn)
	: m_conn(conn, ldap_destroy)
{
}

std::unique_ptr<Ldap> Ldap::create(const std::string_view& hostname)
{
	LDAP* conn;
	auto ret = ldap_initialize(&conn, hostname.data());
	throwIfError(ret, "Couldn't initialize LDAP context");
	return std::unique_ptr<Ldap>(new Ldap(conn));
}

void Ldap::setVersion(Version version)
{
	int ver_int = static_cast<int>(version);
	auto ret = ldap_set_option(m_conn.get(), LDAP_OPT_PROTOCOL_VERSION, &ver_int);
	throwIfError(ret, "Couldn't set LDAP version");
}

void Ldap::connect()
{
	auto ret = ldap_connect(m_conn.get());
	throwIfError(ret, "Couldn't connect to the LDAP server");
}

void Ldap::bindSasl(const std::string_view& bind_dn, const std::string_view& bind_pw)
{
	auto pw_copy = wrap(strdup(bind_pw.data()), std::free); // Yes, you have to make a copy.
	berval cred {
		.bv_len = bind_pw.size(),
		.bv_val = pw_copy.get(),
	};
	auto ret = ldap_sasl_bind_s(m_conn.get(), bind_dn.data(), LDAP_SASL_SIMPLE, &cred, nullptr, nullptr, nullptr);
	throwIfError(ret, "Couldn't authenticate to the LDAP server");
}

std::vector<Entry> Ldap::search(const std::string_view& base_dn, const std::string_view& filter, const std::vector<std::string_view> requested_attr)
{
	// I can't think of a better way of doing this while keeping the input arguments the same.
	auto attr_array = std::make_unique<char*[]>(requested_attr.size() + 1);
	std::transform(requested_attr.begin(), requested_attr.end(), attr_array.get(), [] (const std::string_view& str) {
		return strdup(str.data());
	});
	auto attr_array_deleter = qScopeGuard([&] {
		std::for_each(attr_array.get(), attr_array.get() + requested_attr.size() + 1, std::free);
	});

	LDAPMessage* msg;
	auto ret = ldap_search_ext_s(m_conn.get(), base_dn.data(), LDAP_SCOPE_SUBTREE, filter.data(), attr_array.get(), 0, nullptr, nullptr, nullptr, 0, &msg);
	throwIfError(ret, "Couldn't complete search");
	auto msg_deleter = wrap(msg, ldap_msgfree);

	std::vector<Entry> res;
	for (auto entry = ldap_first_entry(m_conn.get(), msg); entry != nullptr; entry = ldap_next_entry(m_conn.get(), entry)) {
		auto& res_entry = res.emplace_back();
		BerElement* ber;
		for (auto attr = wrap(ldap_first_attribute(m_conn.get(), entry, &ber), std::free); attr != nullptr; attr = wrap(ldap_next_attribute(m_conn.get(), entry, ber), std::free)) {
			auto& res_attr = res_entry.keysAndValues.try_emplace(attr.get(), std::vector<std::string>{}).first->second;
			auto values = wrap(ldap_get_values_len(m_conn.get(), entry, attr.get()), ldap_value_free_len);
			for (auto value = values.get(); *value; value++) {
				auto value_deref = *value;
				res_attr.emplace_back(value_deref->bv_val, value_deref->bv_len);
			}
		}
		ber_free(ber, 0);
	}
	return res;
}
}
