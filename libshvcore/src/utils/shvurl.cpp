#include "shvurl.h"
#include "shvpath.h"
#include "../utils.h"

using namespace std;

namespace shv::core::utils {

ShvUrl::ShvUrl(const std::string &shv_path)
	: m_shvPath(shv_path)
{
	auto ix = serviceProviderMarkIndex(shv_path);
	if(ix > 0) {
		auto type_mark = shv_path[ix];
		if(type_mark == ABSOLUTE_MARK)
			m_type = Type::AbsoluteService;
		else if(type_mark == RELATIVE_MARK)
			m_type = Type::MountPointRelativeService;
		else if(type_mark == DOWNTREE_MARK)
			m_type = Type::DownTreeService;
		m_service = StringView(shv_path).substr(0, ix);
		auto bid_ix = m_service.rfind('@');
		if(bid_ix != std::string_view::npos) {
			m_fullBrokerId = m_service.substr(bid_ix);
			m_service = m_service.substr(0, bid_ix);
		}
		m_pathPart = StringView(shv_path).substr(ix + typeMark(m_type).size() + 1);
	}
	else {
		m_type = Type::Plain;
		m_pathPart = StringView(shv_path);
	}
}

const char *ShvUrl::typeString() const
{
	switch (type()) {
	case Type::Plain: return "Plain";
	case Type::MountPointRelativeService: return "Relative";
	case Type::AbsoluteService: return "Absolute";
	case Type::DownTreeService: return "DownTree";
	}
	return "???";
}

std::string ShvUrl::typeMark(Type t)
{
	switch (t) {
	case Type::Plain: return std::string();
	case Type::MountPointRelativeService: return std::string(1, RELATIVE_MARK) + END_MARK;
	case Type::AbsoluteService: return std::string(1, ABSOLUTE_MARK) + END_MARK;
	case Type::DownTreeService: return std::string(1, DOWNTREE_MARK) + END_MARK;
	}
	return std::string();
}

std::string ShvUrl::toPlainPath(const StringView &path_part_prefix) const
{
	std::string ret = std::string{service()};
	if(!path_part_prefix.empty()) {
		ret += ShvPath::SHV_PATH_DELIM;
		ret += path_part_prefix;
	}
	if(!pathPart().empty()) {
		ret += ShvPath::SHV_PATH_DELIM;
		ret	+= pathPart();
	}
	return ret;
}

std::string ShvUrl::toString(const StringView &path_part_prefix) const
{
	string rest = shv::core::utils::joinPath(path_part_prefix, pathPart());
	return makeShvUrlString(type(), service(), fullBrokerId(), rest);
}

std::string ShvUrl::makeShvUrlString(ShvUrl::Type type, const StringView &service, const StringView &full_broker_id, const StringView &path_rest)
{
	if(type == Type::Plain) {
		if(service.empty())
			return std::string{path_rest};

		return StringViewList{service, path_rest}.join(ShvPath::SHV_PATH_DELIM);
	}

	string srv = std::string{service};
	if(!full_broker_id.empty())
		srv += std::string{full_broker_id};
	srv += typeMark(type);
	return StringViewList{srv, path_rest}.join(ShvPath::SHV_PATH_DELIM);
}

size_t ShvUrl::serviceProviderMarkIndex(const std::string &path)
{
	for (size_t ix = 1; ix + 1 < path.size(); ++ix) {
		if(path[ix + 1] == END_MARK && (path.size() == ix + 2 || path[ix + 2] == ShvPath::SHV_PATH_DELIM)) {
			if(path[ix] == RELATIVE_MARK || path[ix] == ABSOLUTE_MARK || path[ix] == DOWNTREE_MARK)
				return ix;
		}
	}
	return 0;
}

} // namespace shv
