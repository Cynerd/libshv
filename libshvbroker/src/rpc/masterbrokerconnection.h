#pragma once

#include "commonrpcclienthandle.h"

#include <shv/iotqt/rpc/deviceconnection.h>

namespace shv {
namespace broker {
namespace rpc {

class MasterBrokerConnection : public shv::iotqt::rpc::DeviceConnection, public CommonRpcClientHandle
{
	Q_OBJECT

	using Super = shv::iotqt::rpc::DeviceConnection;
public:
	MasterBrokerConnection(QObject *parent = nullptr);

	int connectionId() const override;

	/// master broker connection cannot have userName
	/// because it is connected in oposite direction than client connections
	std::string loggedUserName() override {return std::string();}

	bool isConnectedAndLoggedIn() const override {return isSocketConnected() && !isLoginPhase();}
	bool isSlaveBrokerConnection() const override {return false;}
	bool isMasterBrokerConnection() const override {return true;}

	void sendRawData(const shv::chainpack::RpcValue::MetaData &meta_data, std::string &&data) override;
	void sendMessage(const shv::chainpack::RpcMessage &rpc_msg) override;

	Subscription createSubscription(const std::string &shv_path, const std::string &method) override;
	std::string toSubscribedPath(const Subscription &subs, const std::string &signal_path) const override;

	std::string masterExportedToLocalPath(const std::string &master_path) const;
	std::string localPathToMasterExported(const std::string &local_path) const;
	const std::string& exportedShvPath() const {return m_exportedShvPath;}

	void setOptions(const shv::chainpack::RpcValue &slave_broker_options);
	shv::chainpack::RpcValue options() {return m_options;}
protected:
	void onRpcDataReceived(shv::chainpack::Rpc::ProtocolType protocol_type, shv::chainpack::RpcValue::MetaData &&md, std::string &&msg_data) override;
protected:
	std::string m_exportedShvPath;
	shv::chainpack::RpcValue m_options;
};

}}}

