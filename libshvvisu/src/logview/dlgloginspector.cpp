#include "dlgloginspector.h"
#include "ui_dlgloginspector.h"

#include "logmodel.h"

#include "../timeline/graphmodel.h"
#include "../timeline/graphwidget.h"
#include "../timeline/graph.h"

#include <shv/chainpack/rpcvalue.h>
#include <shv/core/exception.h>
#include <shv/core/string.h>
#include <shv/coreqt/log.h>
#include <shv/core/utils/shvjournalgetlogparams.h>
#include <shv/iotqt/rpc/clientconnection.h>
#include <shv/iotqt/rpc/rpcresponsecallback.h>
#include <shv/iotqt/utils.h>

#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

namespace cp = shv::chainpack;
namespace tl = shv::visu::timeline;

namespace shv {
namespace visu {
namespace logview {

DlgLogInspector::DlgLogInspector(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DlgLogInspector)
{
	ui->setupUi(this);
	ui->lblInfo->hide();
	ui->btMoreOptions->setChecked(false);
	QDateTime dt2 = QDateTime::currentDateTime();
	QDateTime dt1 = dt2.addDays(-1);
	dt2 = dt2.addSecs(60 * 60);
	ui->edSince->setDateTime(dt1);
	ui->edUntil->setDateTime(dt2);

	m_logModel = new LogModel(this);
	m_sortFilterProxy = new QSortFilterProxyModel(this);
	m_sortFilterProxy->setFilterKeyColumn(-1);
	m_sortFilterProxy->setSourceModel(m_logModel);
	ui->tblData->setModel(m_sortFilterProxy);
	ui->tblData->setSortingEnabled(true);
	connect(ui->edFilter, &QLineEdit::textChanged, [this]() {
		QString str = ui->edFilter->text().trimmed();
		m_sortFilterProxy->setFilterFixedString(str);
		//if(str.isEmpty()) {
		//}
		//m_sortFilterProxy->setFilterWildcard("*" + str + "*");
	});

	m_graphModel = new tl::GraphModel(this);
	//connect(m_dataModel, &tl::GraphModel::xRangeChanged, this, &MainWindow::onGraphXRangeChanged);
	//ui->graphView->viewport()->show();
	m_graphWidget = new tl::GraphWidget();

	ui->graphView->setBackgroundRole(QPalette::Dark);
	ui->graphView->setWidget(m_graphWidget);
	ui->graphView->widget()->setBackgroundRole(QPalette::ToolTipBase);
	ui->graphView->widget()->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	//shvInfo() << qobject_cast<QWidget*>(ui->graphView->widget());
	m_graph = new tl::Graph(this);
	m_graph->setModel(m_graphModel);
	m_graphWidget->setGraph(m_graph);

	connect(ui->btLoad, &QPushButton::clicked, this, &DlgLogInspector::downloadLog);

	loadSettings();
}

DlgLogInspector::~DlgLogInspector()
{
	saveSettings();
	delete ui;
}

void DlgLogInspector::loadSettings()
{
	QSettings settings;
	QByteArray ba = settings.value("ui/DlgLogView/geometry").toByteArray();
	restoreGeometry(ba);
}

void DlgLogInspector::saveSettings()
{
	QSettings settings;
	QByteArray ba = saveGeometry();
	settings.setValue("ui/DlgLogView/geometry", ba);
}

shv::iotqt::rpc::ClientConnection *DlgLogInspector::rpcConnection()
{
	if(!m_rpcConnection)
		SHV_EXCEPTION("RPC connection is NULL");
	return m_rpcConnection;
}

void DlgLogInspector::setRpcConnection(iotqt::rpc::ClientConnection *c)
{
	m_rpcConnection = c;
}

QString DlgLogInspector::shvPath() const
{
	return ui->edShvPath->text();
}

void DlgLogInspector::setShvPath(const QString &s)
{
	ui->edShvPath->setText(s);
}

shv::chainpack::RpcValue DlgLogInspector::getLogParams()
{
	shv::core::utils::ShvJournalGetLogParams params;
	auto get_dt = [](QDateTimeEdit *ed) {
		QDateTime dt = ed->dateTime();
		return dt == ed->minimumDateTime()?  cp::RpcValue():  cp::RpcValue::DateTime::fromMSecsSinceEpoch(ed->dateTime().toMSecsSinceEpoch());
	};
	params.since = get_dt(ui->edSince);
	params.until = get_dt(ui->edUntil);
	if(ui->edMaxRecordCount->value() > ui->edMaxRecordCount->minimum())
		params.maxRecordCount = ui->edMaxRecordCount->value();
	params.withUptime = ui->chkWithUptime->isChecked();
	params.withSnapshot = ui->chkWithSnapshot->isChecked();
	unsigned header_opts = 0;
	if(ui->chkBasicInfo->isChecked())
		header_opts |= static_cast<unsigned>(shv::core::utils::ShvJournalGetLogParams::HeaderOptions::BasicInfo);
	if(ui->chkFieldInfo->isChecked())
		header_opts |= static_cast<unsigned>(shv::core::utils::ShvJournalGetLogParams::HeaderOptions::FieldInfo);
	if(ui->chkTypeInfo->isChecked())
		header_opts |= static_cast<unsigned>(shv::core::utils::ShvJournalGetLogParams::HeaderOptions::TypeInfo);
	if(ui->chkPathsDict->isChecked())
		header_opts |= static_cast<unsigned>(shv::core::utils::ShvJournalGetLogParams::HeaderOptions::PathsDict);
	params.headerOptions = header_opts;
	return params.toRpcValue();
}

void DlgLogInspector::downloadLog()
{
	std::string shv_path = shvPath().toStdString();
	if(ui->chkUseHistoryProvider->isChecked()) {
		if(shv::core::String::startsWith(shv_path, "shv/"))
			shv_path = "history" + shv_path.substr(3);
	}
	showInfo(QString::fromStdString("Downloading data from " + shv_path));
	cp::RpcValue params = getLogParams();
	shv::iotqt::rpc::ClientConnection *conn = rpcConnection();
	int rq_id = conn->nextRequestId();
	shv::iotqt::rpc::RpcResponseCallBack *cb = new shv::iotqt::rpc::RpcResponseCallBack(conn, rq_id, this);
	cb->setTimeout(ui->edTimeout->value() * 1000);
	cb->start(this, [this, shv_path](const cp::RpcResponse &resp) {
		if(resp.isValid()) {
			if(resp.isError()) {
				showInfo(QString::fromStdString("GET " + shv_path + " RPC request error: " + resp.error().toString()), true);
			}
			else {
				showInfo();
				this->parseLog(resp.result());
			}
		}
		else {
			showInfo(QString::fromStdString("GET " + shv_path + " RPC request timeout"), true);
		}
	});
	conn->callShvMethod(rq_id, shv_path, cp::Rpc::METH_GET_LOG, params);
}

void DlgLogInspector::parseLog(shv::chainpack::RpcValue log)
{
	{
		std::string str = log.metaData().toString("\t");
		ui->edInfo->setPlainText(QString::fromStdString(str));
	}
	{
		m_logModel->setLog(log);
		ui->tblData->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	}
	{
		struct ShortTime {
			int64_t msec_sum = 0;
			uint16_t last_msec = 0;

			int64_t addShortTime(uint16_t msec)
			{
				msec_sum += static_cast<uint16_t>(msec - last_msec);
				last_msec = msec;
				return msec_sum;
			}
		};
		QMap<std::string, ShortTime> short_times;
		const shv::chainpack::RpcValue::IMap &dict = log.metaValue("pathsDict").toIMap();
		const shv::chainpack::RpcValue::List lst = log.toList();
		m_graphModel->clear();
		m_graphModel->beginAppendValues();
		ShortTime anca_hook_short_time;
		for(const cp::RpcValue &rec : lst) {
			const cp::RpcValue::List &row = rec.toList();

			cp::RpcValue::DateTime dt = row.value(0).toDateTime();
			cp::RpcValue rv_path = row.value(1);
			if(rv_path.isUInt() || rv_path.isInt())
				rv_path = dict.value(rv_path.toInt());
			const shv::chainpack::RpcValue::String &path = rv_path.toString();
			if(path.empty()) {
				shvError() << "invalid entry path:" << rec.toCpon();
				continue;
			}
			cp::RpcValue rv = row.value(2);
			int64_t msec = dt.msecsSinceEpoch();
			cp::RpcValue short_time = row.value(3);
			if(short_time.isUInt() || short_time.isInt()) {
				uint16_t short_msec = static_cast<uint16_t>(short_time.toUInt());
				ShortTime &st = short_times[path];
				if(st.msec_sum == 0)
					st.msec_sum = msec;
				msec = st.addShortTime(short_msec);
			}
			QVariant v = shv::iotqt::Utils::rpcValueToQVariant(rv);
			if(v.isValid()) {
				shvDebug() << path << v.typeName();
				if(path == "data" && v.type() == QVariant::List) {
					// Anca hook
					QVariantList vl = v.toList();
					uint16_t short_msec = static_cast<uint16_t>(vl.value(0).toInt());
					if(anca_hook_short_time.msec_sum == 0)
						anca_hook_short_time.msec_sum = msec;
					msec = anca_hook_short_time.addShortTime(short_msec);
					m_graphModel->appendValueShvPath("U", tl::Sample{msec, vl.value(1)});
					m_graphModel->appendValueShvPath("I", tl::Sample{msec, vl.value(2)});
					m_graphModel->appendValueShvPath("P", tl::Sample{msec, vl.value(3)});
				}
				else {
					m_graphModel->appendValueShvPath(path, tl::Sample{msec, v});
				}
			}
		}
		m_graphModel->endAppendValues();
	}
	m_graph->createChannelsFromModel();
	ui->graphView->makeLayout();
}

void DlgLogInspector::showInfo(const QString &msg, bool is_error)
{
	if(msg.isEmpty()) {
		ui->lblInfo->hide();
	}
	else {
		QString ss = is_error? QString("background: salmon"): QString("background: lime");
		ui->lblInfo->setStyleSheet(ss);
		ui->lblInfo->setText(msg);
		ui->lblInfo->show();
	}
}

}}}