#include "mockserialport.h"
#include "mockrpcconnection.h"

#include <shv/iotqt/rpc/serialportsocket.h>
#include <shv/iotqt/rpc/socket.h>

#include <shv/chainpack/rpcmessage.h>

#include <necrolog.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace shv::iotqt::rpc;
using namespace shv::chainpack;
using namespace std;

DOCTEST_TEST_CASE("Send")
{
	MockRpcConnection conn;
	auto *serial = new MockSerialPort("TestSend", &conn);
	//serial->open(QIODevice::ReadWrite);
	//nInfo() << "serial is open:" << serial->isOpen();
	//serial->write("abc", 2);
	auto *socket = new SerialPortSocket(serial);
	conn.setSocket(socket);
	conn.connectToHost(QUrl());
	RpcRequest rq;
	rq.setRequestId(1);
	rq.setShvPath("foo/bar");
	rq.setMethod("baz");
	rq.setParams(RpcValue::List{1, 2,3});
	conn.sendMessage(rq);
	nInfo() << "data writen:" << serial->writeData().toHex().toStdString();
}