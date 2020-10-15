#pragma once

#include "commonrpcclienthandle.h"

#include <shv/iotqt/rpc/serverconnection.h>

class QTimer;

namespace shv { namespace core { class StringView; }}
namespace shv { namespace core { namespace utils { class ServiceProviderPath; }}}
namespace shv { namespace iotqt { namespace rpc { class Socket; }}}
namespace shv { namespace iotqt { namespace node { class ShvNode; }}}

namespace shv {
namespace broker {
namespace rpc {

class ClientConnectionOnBroker : public shv::iotqt::rpc::ServerConnection, public CommonRpcClientHandle
{
	Q_OBJECT

	using Super = shv::iotqt::rpc::ServerConnection;
public:
	ClientConnectionOnBroker(shv::iotqt::rpc::Socket* socket, QObject *parent = nullptr);
	~ClientConnectionOnBroker() override;

	int connectionId() const override {return Super::connectionId();}
	bool isConnectedAndLoggedIn() const override {return Super::isConnectedAndLoggedIn();}
	bool isSlaveBrokerConnection() const override {return Super::isSlaveBrokerConnection();}
	bool isMasterBrokerConnection() const override {return false;}

	std::string loggedUserName() override {return Super::userName();}

	shv::chainpack::RpcValue tunnelOptions() const;
	shv::chainpack::RpcValue deviceOptions() const;
	shv::chainpack::RpcValue deviceId() const;

	void setMountPoint(const std::string &mp);
	const std::string& mountPoint() const {return m_mountPoint;}

	int idleTime() const;
	int idleTimeMax() const;

	std::string resolveLocalPath(const shv::core::utils::ServiceProviderPath &spp, shv::iotqt::node::ShvNode **pnd = nullptr);

	void setIdleWatchDogTimeOut(int sec);

	void sendMessage(const shv::chainpack::RpcMessage &rpc_msg) override;
	void sendRawData(const shv::chainpack::RpcValue::MetaData &meta_data, std::string &&data) override;

	Subscription createSubscription(const std::string &shv_path, const std::string &method) override;
	std::string toSubscribedPath(const Subscription &subs, const std::string &signal_path) const override;
	void propagateSubscriptionToSlaveBroker(const Subscription &subs);

	void setLoginResult(const chainpack::UserLoginResult &result) override;
private:
	void onSocketConnectedChanged(bool is_connected);
	void onRpcDataReceived(shv::chainpack::Rpc::ProtocolType protocol_type, shv::chainpack::RpcValue::MetaData &&md, const std::string &data, size_t start_pos, size_t data_len) override;
	bool checkTunnelSecret(const std::string &s);

	void processLoginPhase() override;
private:
	QTimer *m_idleWatchDogTimer = nullptr;
	std::string m_mountPoint;
};

}}}
