#pragma once

#include "channelfilter.h"
#include "graphchannel.h"
#include "graphbuttonbox.h"
#include "sample.h"
#include "../shvvisuglobal.h"

#include <shv/coreqt/utils.h>
#include <shv/core/exception.h>

#include <QObject>
#include <QVector>
#include <QVariantMap>
#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QRect>
#include <QTimeZone>

namespace shv {
namespace visu {
namespace timeline {

class GraphModel;

class SHVVISU_DECL_EXPORT Graph : public QObject
{
	Q_OBJECT
public:
	struct SHVVISU_DECL_EXPORT DataRect
	{
		XRange xRange;
		YRange yRange;

		DataRect() {}
		DataRect(const XRange &xr, const YRange &yr) : xRange(xr), yRange(yr) {}

		bool isValid() const { return xRange.isValid() && yRange.isValid(); }
		//bool isNull() const { return xRange.isNull() && yRange.isNull(); }
	};

	using WidgetRange = Range<int>;

	class SHVVISU_DECL_EXPORT Style : public QVariantMap
	{
		SHV_VARIANTMAP_FIELD2(int, u, setU, nitSize, 20) // px
		SHV_VARIANTMAP_FIELD2(double, h, setH, eaderInset, 0.2) // units
		SHV_VARIANTMAP_FIELD2(double, b, setB, uttonSize, 1.2) // units
		SHV_VARIANTMAP_FIELD2(double, l, setL, eftMargin, 0.3) // units
		SHV_VARIANTMAP_FIELD2(double, r, setR, ightMargin, 0.3) // units
		SHV_VARIANTMAP_FIELD2(double, t, setT, opMargin, 0.3) // units
		SHV_VARIANTMAP_FIELD2(double, b, setb, ottomMargin, 0.3) // units
		SHV_VARIANTMAP_FIELD2(double, c, setC, hannelSpacing, 0.1) // units
		SHV_VARIANTMAP_FIELD2(double, x, setX, AxisHeight, 1.5) // units
		SHV_VARIANTMAP_FIELD2(double, y, setY, AxisWidth, 2.5) // units
		SHV_VARIANTMAP_FIELD2(double, m, setM, iniMapHeight, 2) // units
		SHV_VARIANTMAP_FIELD2(double, v, setV, erticalHeaderWidth, 10) // units
		SHV_VARIANTMAP_FIELD2(bool, s, setS, eparateChannels, true)

		SHV_VARIANTMAP_FIELD2(QColor, c, setC, olor, QColor("#c8c8c8"))
		SHV_VARIANTMAP_FIELD2(QColor, c, setC, olorPanel, QColor("#414343"))
		SHV_VARIANTMAP_FIELD2(QColor, c, setC, olorBackground, QColor(Qt::black))
		//SHV_VARIANTMAP_FIELD2(QColor, c, setC, olorGrid, QColor(Qt::darkGreen))
		SHV_VARIANTMAP_FIELD2(QColor, c, setC, olorAxis, QColor(Qt::gray))
		SHV_VARIANTMAP_FIELD2(QColor, c, setC, olorCurrentTime, QColor(QStringLiteral("#cced5515")))
		SHV_VARIANTMAP_FIELD2(QColor, c, setC, olorCrossBar, QColor(QStringLiteral("white")))
		//SHV_VARIANTMAP_FIELD2(QColor, c, setC, olorCrossBar2, QColor(QStringLiteral("salmon")))
		SHV_VARIANTMAP_FIELD2(QColor, c, setC, olorSelection, QColor(QStringLiteral("deepskyblue")))

		SHV_VARIANTMAP_FIELD(QFont, f, setF, ont)
	public:
		void init(QWidget *widget);

		//double buttonSpacing() const { return buttonSize() / 5; }
	};
public:
	Graph(QObject *parent = nullptr);
	virtual ~Graph();

	const Style& effectiveStyle() const { return  m_effectiveStyle; }

	void setModel(GraphModel *model);
	GraphModel *model() const;

	void setTimeZone(const QTimeZone &tz);
	QTimeZone timeZone() const;

	void createChannelsFromModel();

	int channelCount() const { return  m_channels.count(); }
	void clearChannels();
	GraphChannel* appendChannel(int model_index = -1);
	GraphChannel* channelAt(int ix, bool throw_exc = shv::core::Exception::Throw);
	const GraphChannel* channelAt(int ix, bool throw_exc = shv::core::Exception::Throw) const;
	int channelMetaTypeId(int ix) const;

	void showAllChannels();
	QStringList channelPaths();
	void hideFlatChannels();
	const ChannelFilter& channelFilter() const { return m_channelFilter; }
	void setChannelFilter(const ChannelFilter &filter);
	void setChannelVisible(int channel_ix, bool is_visible);
	void setChannelMaximized(int channel_ix, bool is_maximized);
	//DataRect dataRect(int channel_ix) const;

	timemsec_t miniMapPosToTime(int pos) const;
	int miniMapTimeToPos(timemsec_t time) const;

	timemsec_t posToTime(int pos) const;
	int timeToPos(timemsec_t time) const;
	Sample timeToSample(int channel_ix, timemsec_t time) const;
	int posToChannel(const QPoint &pos) const;
	Sample posToData(const QPoint &pos) const;
	//QVariant posToValue(const QPoint &pos) const;
	QPoint dataToPos(int ch_ix, const Sample &s) const;

	const QRect& rect() const { return  m_layout.rect; }
	const QRect& miniMapRect() const { return  m_layout.miniMapRect; }
	const QRect& cornerCellRect() const { return  m_layout.cornerCellRect; }
	QRect southFloatingBarRect() const;
	//bool isCrossBarVisible() const {return !m_state.crossBarPos.isNull() && m_state.crossBarChannel >= 0;}
	struct SHVVISU_DECL_EXPORT CrossHairPos
	{
		int channelIndex = -1;
		QPoint possition;

		CrossHairPos() {}
		CrossHairPos(int ch_ix, const QPoint &pos) : channelIndex(ch_ix), possition(pos) {}

		bool isValid() const { return channelIndex >= 0 && !possition.isNull(); }
	};
	CrossHairPos crossHairPos() const {return m_state.crossHairPos;}
	void setCrossHairPos(const CrossHairPos &pos);
	//void setCrossBarPos2(const QPoint &pos);

	void setCurrentTime(timemsec_t time);
	timemsec_t currentTime() const { return m_state.currentTime; }

	void setSelectionRect(const QRect &rect);

	XRange xRange() const { return m_state.xRange; }
	XRange xRangeZoom() const { return m_state.xRangeZoom; }
	void setXRange(const XRange &r, bool keep_zoom = false);
	void setXRangeZoom(const XRange &r);

	void setYRange(int channel_ix, const YRange &r);
	void enlargeYRange(int channel_ix, double step);
	void setYRangeZoom(int channel_ix, const YRange &r);
	void resetZoom(int channel_ix);
	void zoomToSelection();

	const Style& style() const { return m_style; }
	void setStyle(const Style &st);
	void setDefaultChannelStyle(const GraphChannel::Style &st);
	GraphChannel::Style defaultChannelStyle() const { return m_defaultChannelStyle; }

	void makeLayout(const QRect &pref_rect);
	void draw(QPainter *painter, const QRect &dirty_rect, const QRect &view_rect);

	int u2px(double u) const;
	double u2pxf(double u) const;
	double px2u(int px) const;

	static std::function<QPoint (const Sample &s, int meta_type_id)> dataToPointFn(const DataRect &src, const QRect &dest);
	static std::function<Sample (const QPoint &)> pointToDataFn(const QRect &src, const DataRect &dest);
	static std::function<timemsec_t (int)> posToTimeFn(const QPoint &src, const XRange &dest);
	static std::function<int (timemsec_t)> timeToPosFn(const XRange &src, const WidgetRange &dest);
	static std::function<int (double)> valueToPosFn(const YRange &src, const WidgetRange &dest);
	static std::function<double (int)> posToValueFn(const WidgetRange &src, const YRange &dest);

	void processEvent(QEvent *ev);

	Q_SIGNAL void presentationDirty(const QRect &rect);
	void emitPresentationDirty(const QRect &rect) { emit presentationDirty(rect); }
	Q_SIGNAL void layoutChanged();
	Q_SIGNAL void channelFilterChanged();
	Q_SIGNAL void channelContextMenuRequest(int channel_index, const QPoint &mouse_pos);
	void emitChannelContextMenuRequest(int channel_index, const QPoint &mouse_pos) { emit channelContextMenuRequest(channel_index, mouse_pos); }
	Q_SIGNAL void graphContextMenuRequest(const QPoint &mouse_pos);

	static QString rectToString(const QRect &r);
protected:
	void sanityXRangeZoom();
	//void onModelXRangeChanged(const timeline::XRange &range);

	void clearMiniMapCache();

	void drawRectText(QPainter *painter, const QRect &rect, const QString &text, const QFont &font, const QColor &color, const QColor &background = QColor());
	void drawCenteredRectText(QPainter *painter, const QPoint &top_center, const QString &text, const QFont &font, const QColor &color, const QColor &background = QColor());

	QVector<int> visibleChannels();
	int maximizedChannelIndex();

	bool isChannelFlat(GraphChannel *ch);

	void drawBackground(QPainter *painter, const QRect &dirty_rect);
	virtual void drawCornerCell(QPainter *painter);
	virtual void drawMiniMap(QPainter *painter);
	virtual void drawXAxis(QPainter *painter);

	virtual void drawVerticalHeader(QPainter *painter, int channel);
	virtual void drawBackground(QPainter *painter, int channel);
	virtual void drawGrid(QPainter *painter, int channel);
	virtual void drawYAxis(QPainter *painter, int channel);
	virtual void drawSamples(QPainter *painter, int channel_ix
			, const DataRect &src_rect = DataRect()
			, const QRect &dest_rect = QRect()
			, const GraphChannel::Style &channel_style = GraphChannel::Style());
	virtual void drawCrossHair(QPainter *painter, int channel_ix);
	virtual void drawSelection(QPainter *painter);
	virtual void drawCurrentTime(QPainter *painter, int channel_ix);
	void drawXAxisTimeMark(QPainter *painter, time_t time, const QColor &color);

	QVariantMap mergeMaps(const QVariantMap &base, const QVariantMap &overlay) const;
	void makeXAxis();
	void makeYAxis(int channel);

	void moveSouthFloatingBarBottom(int bottom);
protected:
	void onButtonBoxClicked(int button_id);
protected:
	GraphModel *m_model = nullptr;

	QTimeZone m_timeZone;

	Style m_effectiveStyle;

	Style m_style;
	GraphChannel::Style m_defaultChannelStyle;

	QVector<GraphChannel*> m_channels;
	ChannelFilter m_channelFilter;

	struct SHVVISU_DECL_EXPORT XAxis
	{
		enum class LabelFormat {MSec, Sec, Min, Hour, Day, Month, Year};
		timemsec_t tickInterval = 0;
		int subtickEvery = 1;
		double tickLen = 0.15;
		LabelFormat labelFormat = LabelFormat::MSec;

		XAxis() {}
		XAxis(timemsec_t t, int se, LabelFormat f)
			: tickInterval(t)
			, subtickEvery(se)
			, labelFormat(f)
		{}
	};
	struct
	{
		XRange xRange;
		XRange xRangeZoom;
		CrossHairPos crossHairPos;
		timemsec_t currentTime = 0;
		QRect selectionRect;
		XAxis xAxis;
	} m_state;

	struct
	{
		QRect rect;
		QRect miniMapRect;
		QRect xAxisRect;
		QRect cornerCellRect;
	} m_layout;

	QPixmap m_miniMapCache;
	GraphButtonBox *m_cornerCellButtonBox = nullptr;
};

}}}
