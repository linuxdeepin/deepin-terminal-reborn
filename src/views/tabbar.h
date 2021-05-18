/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TABBAR_H
#define TABBAR_H

#include <DTabBar>
#include <DMenu>

#include <QProxyStyle>

DWIDGET_USE_NAMESPACE

class TermWidgetPage;
class MainWindow;

/*******************************************************************************
 1. @类名:    TermTabStyle
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

//Tab文字颜色状态
enum TabTextColorStatus {
    //tab文字默认颜色
    TabTextColorStatus_Default = 0,
    //tab文字即将变色
    TabTextColorStatus_NeedChange = 1,
    //tab文字已经变色
    TabTextColorStatus_Changed = 2
};

class TermTabStyle : public QProxyStyle
{
    Q_OBJECT
public:
    TermTabStyle();
    virtual ~TermTabStyle();

    /**
     * @brief 设置标签文本颜色
     * @author ut000610 daizhengwen
     * @param color 颜色
     */
    void setTabTextColor(const QColor &color);
    /**
     * @brief 设置标签状态图
     * @author ut000610 daizhengwen
     * @param tabStatusMap 标签状态图
     */
    void setTabStatusMap(const QMap<QString, TabTextColorStatus> &tabStatusMap);

    /**
     * @brief 获取内容大小
     * @author ut000610 daizhengwen
     * @param type
     * @param option
     * @param size
     * @param widget
     * @return
     */
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const;
    /**
     * @brief 像素指标
     * @author ut000610 daizhengwen
     * @param metric
     * @param option
     * @param widget
     * @return
     */
    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;
    /**
     * @brief 绘制控件
     * @author ut000610 daizhengwen
     * @param element
     * @param option
     * @param painter
     * @param widget
     */
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    /**
     * @brief 绘制原始
     * @author ut000610 daizhengwen
     * @param element
     * @param option
     * @param painter
     * @param widget
     */
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = Q_NULLPTR) const;
private:
    int m_tabCount;
    QColor m_tabTextColor;
    QMap<QString, TabTextColorStatus> m_tabStatusMap;
};

/*******************************************************************************
 1. @类名:    TabBar
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class TabBar : public DTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);
    ~TabBar();

    /**
     * @brief 获取识别码
     * @author ut000610 daizhengwen
     * @param index
     * @return
     */
    const QString identifier(int index) const;

    /**
     * @brief 设置标签高度
     * @author ut000610 daizhengwen
     * @param tabHeight 标签高度
     */
    void setTabHeight(int tabHeight);
    /**
     * @brief 设置标签项最小宽度
     * @author ut000610 daizhengwen
     * @param tabItemMinWidth 标签项最小宽度
     */
    void setTabItemMinWidth(int tabItemMinWidth);
    /**
     * @brief 设置标签项最大宽度
     * @author ut000610 daizhengwen
     * @param tabItemMaxWidth 标签项最大宽度
     */
    void setTabItemMaxWidth(int tabItemMaxWidth);

    /**
     * @brief 增加标签
     * @author ut000610 daizhengwen
     * @param tabIdentifier 标签识别码
     * @param tabName 标签名称
     * @return
     */
    int addTab(const QString &tabIdentifier, const QString &tabName);
    /**
     * @brief 增加标签
     * @author ut000610 wangliang
     * @param index 标签索引
     * @param tabIdentifier 标签识别码
     * @param tabName 标签名称
     * @return
     */
    int insertTab(const int &index, const QString &tabIdentifier, const QString &tabName);
    /**
     * @brief 通过标识码删除标签
     * @author ut000610 daizhengwen
     * @param tabIdentifier 标签标识码
     */
    void removeTab(const QString &tabIdentifier);
    /**
     * @brief 移除标签，同时设置标签拖拽状态
     * @author ut000438 王亮
     * @param index 标签索引
     */
    void closeTab(const int &index);
    /**
     * @brief 设置标签文本
     * @author ut000610 daizhengwen
     * @param tabIdentifier 标签标识码
     * @param text 标签文本
     * @return
     */
    bool setTabText(const QString &tabIdentifier, const QString &text);

    /**
     * @brief 使用标签索引保存会话ID
     * @author ut000610 daizhengwen
     * @param sessionId 会话ID
     * @param index 标签索引
     */
    void saveSessionIdWithTabIndex(int sessionId, int index);
    /**
     * @brief 使用标签ID保存会话ID
     * @author ut000610 daizhengwen
     * @param sessionId 会话ID
     * @param tabIdentifier 标签ID
     */
    void saveSessionIdWithTabId(int sessionId, const QString &tabIdentifier);
    /**
     * @brief 获取会话ID和标签索引map
     * @author ut000610 daizhengwen
     * @return
     */
    QMap<int, int> getSessionIdTabIndexMap();

    /**
     * @brief 按会话ID查询索引
     * @author ut000610 daizhengwen
     * @param sessionId 会话ID
     * @return
     */
    int queryIndexBySessionId(int sessionId);
    /**
     * @brief 通过标识符获取索引
     * @author ut000610 daizhengwen
     * @param id 标识符
     * @return
     */
    int getIndexByIdentifier(QString id);

    /**
     * @brief 设置更改文字颜色
     * @author ut000610 daizhengwen
     * @param tabIdentifier 标签识别码
     */
    void setChangeTextColor(const QString &tabIdentifier);
    /**
     * @brief 设置需要更改文本颜色
     * @author ut000610 daizhengwen
     * @param tabIdentifier 标签识别码
     * @param color 颜色
     */
    void setNeedChangeTextColor(const QString &tabIdentifier, const QColor &color);
    /**
     * @brief 删除需要更改文本颜色
     * @author ut000610 daizhengwen
     * @param tabIdentifier 标签识别码
     */
    void removeNeedChangeTextColor(const QString &tabIdentifier);
    /**
     * @brief 是否需要更改文字颜色
     * @author ut000610 daizhengwen
     * @param tabIdentifier 标签识别码
     * @return
     */
    bool isNeedChangeTextColor(const QString &tabIdentifier);
    /**
     * @brief 设置清除标签颜色
     * @author ut000610 daizhengwen
     * @param tabIdentifier 标签识别码
     */
    void setClearTabColor(const QString &tabIdentifier);
    /**
     * @brief 设置标签状态图
     * @author ut000610 daizhengwen
     * @param tabStatusMap 标签状态图
     */
    void setTabStatusMap(const QMap<QString, TabTextColorStatus> &tabStatusMap);

    /**
     * @brief 设置是否启用关闭tab动画效果
     * @author ut000610 daizhengwen
     * @param isEnableCloseTabAnimation 是否启用关闭tab动画效果
     */
    void setEnableCloseTabAnimation(bool isEnableCloseTabAnimation);
    /**
     * @brief 是启用关闭标签动画
     * @author ut000610 daizhengwen
     * @return
     */
    bool isEnableCloseTabAnimation();

    /**
     * @brief 用于标记当前tab是否为雷神窗口的tab
     * @author ut000438 王亮
     * @param isQuakeWindowTab 是否为雷神窗口的tab
     */
    void setIsQuakeWindowTab(bool isQuakeWindowTab);
    /**
     * @brief 用于设置tab拖拽状态，仅当窗口为雷神模式且标签页数量为1时不允许拖拽
     * @author ut000438 王亮
     */
    void updateTabDragMoveStatus();

public slots:
    /**
     * @brief 切换到对应index的标签，并移除标签文字颜色
     * @author ut000438 王亮
     * @param index 标签索引
     */
    void setCurrentIndex(int index);

protected:
    /**
     * @brief 事件过滤器
     * @author ut000610 daizhengwen
     * @param watched
     * @param event 事件
     * @return
     */
    bool eventFilter(QObject *watched, QEvent *event) override;
    /**
     * @brief 返回最小标签的大小
     * @author ut000610 daizhengwen
     * @param index 标签索引
     * @return
     */
    QSize minimumTabSizeHint(int index) const override;
    /**
     * @brief 返回最大标签的大小
     * @author ut000610 daizhengwen
     * @param index 标签索引
     * @return
     */
    QSize maximumTabSizeHint(int index) const override;

    /**
     * @brief 将标签对应的TermWidgetPage控件转化为QPixmap图像，用于在拖拽过程中显示
     * @author ut000438 王亮
     * @param index 标签页索引
     * @param option
     * @param hotspot
     * @return
     */
    QPixmap createDragPixmapFromTab(int index, const QStyleOptionTab &option, QPoint *hotspot) const override;
    /**
     * @brief 自定义QMimeData数据用于拖放数据的存储
     * @author ut000438 王亮
     * @param index 标签页索引
     * @param option
     * @return
     */
    QMimeData *createMimeDataFromTab(int index, const QStyleOptionTab &option) const override;
    /**
     * @brief  拖拽过程中将从QMimeData中取出拖拽的标签页相关数据，插入标签页到当前窗口，此时插入的标签为拖拽过程中的“虚拟标签”
     * @author ut000438 王亮
     * @param index 标签页索引
     * @param source
     */
    void insertFromMimeDataOnDragEnter(int index, const QMimeData *source) override;
    /**
     * @brief 拖拽结束后将从QMimeData中取出拖拽的标签页相关数据，插入标签页到当前窗口
     * @author ut000438 王亮
     * @param index 标签页索引
     * @param source
     */
    void insertFromMimeData(int index, const QMimeData *source) override;
    /**
     * @brief 用于判断是否能插入标签
     * @author ut000438 王亮
     * @param index 标签页索引
     * @param source
     * @return
     */
    bool canInsertFromMimeData(int index, const QMimeData *source) const override;

    /**
     * @brief 根据对应标签名称、标签对应的工作区创建新的MainWindow
     * @author ut000438 王亮
     * @param tabName 标签名称
     * @param termPage 标签对应的工作区
     * @param isActiveTab
     */
    void createWindowFromTermPage(const QString &tabName, TermWidgetPage *termPage, bool isActiveTab);
    /**
     * @brief 创建一个用于标签页拖拽的新窗口
     * @author ut000438 王亮
     * @return
     */
    MainWindow *createNormalWindow();

    /**
     * @brief 用于计算拖拽窗口结束鼠标释放后的窗口位置
     * @author ut000438 王亮
     * @param window 窗口
     * @return
     */
    QPoint calculateDragDropWindowPosition(MainWindow *window);

signals:
    void tabBarClicked(int index, QString Identifier);
    void menuCloseTab(QString Identifier);
    void menuCloseOtherTab(QString Identifier);
    void showRenameTabDialog(QString Identifier);

private slots:
    /**
     * @brief 点击某个标签触发，并发出tabBarClicked信号，传递index和标签identifier参数
     * @author ut000438 王亮
     * @param index
     */
    void handleTabBarClicked(int index);
    /**
     * @brief 标签左右移动后，交换移动的两个标签对应的标签identifier
     * @author ut000438 王亮
     * @param fromIndex 原标签索引
     * @param toIndex 目标标签索引
     */
    void handleTabMoved(int fromIndex, int toIndex);
    /**
     * @brief 标签拖拽释放后，使用拖拽出的TermWidgetPage页面新建窗口，关闭原窗口拖出的标签页并移除对应页面
     * @author ut000438 王亮
     * @param index 标签页索引
     */
    void handleTabReleased(int index);
    /**
     * @brief 移除标签操作，同时移除对应的工作区页面TermWidgetPage
     * @author ut000438 王亮
     * @param index 标签索引
     */
    void handleTabIsRemoved(int index);
    /**
     * @brief 处理拖入/拖出标签drop后，关闭之前窗口标签/新建MainWindow窗口显示相关逻辑
     * @author ut000438 王亮
     * @param index 标签索引
     * @param dropAction 拖拽行为
     * @param target
     */
    void handleTabDroped(int index, Qt::DropAction dropAction, QObject *target);
    /**
     * @brief 拖动过程中动态改变鼠标光标样式
     * @author ut000438 王亮
     * @param action
     */
    void handleDragActionChanged(Qt::DropAction action);

    /**
     * @brief 触发关闭标签页的Action
     * @author ut000438 王亮
     */
    void onCloseTabActionTriggered();
    /**
     * @brief 触发关闭其他标签页的Action
     * @author ut000438 王亮
     */
    void onCloseOtherTabActionTriggered();
    /**
     * @brief 触发重命名标签页的Action
     * @author ut000438 王亮
     */
    void onRenameTabActionTriggered();

    /**
     * @brief 窗口关闭处理
     * @author ut000438 王亮
     */
    void handleWindowClose();

private:
    /**
     * @brief 处理标签鼠标中键点击
     * @author ut000438 王亮
     * @param mouseEvent 鼠标事件
     */
    void handleMiddleButtonClick(QMouseEvent *mouseEvent);
    /**
     * @brief 处理标签鼠标右键点击
     * @author ut000438 王亮
     * @param mouseEvent 鼠标事件
     * @return
     */
    bool handleRightButtonClick(QMouseEvent *mouseEvent);

    QAction *m_closeOtherTabAction = nullptr;
    QAction *m_closeTabAction = nullptr;
    QAction *m_renameTabAction = nullptr;

    DMenu *m_rightMenu = nullptr;
    int m_rightClickTab;
    int m_tabHeight;
    int m_tabItemMinWidth;
    int m_tabItemMaxWidth;

    QStringList m_tabIdentifierList;

    QMap<int, int> m_sessionIdTabIndexMap; // key--sessionId, value--tabIndex
    QMap<int, QString> m_sessionIdTabIdMap; // key--sessionId, value--tabIdentifier

    QMap<QString, TabTextColorStatus> m_tabStatusMap;
    QColor m_tabChangedTextColor;

    bool m_isEnableCloseTabAnimation;
    bool m_isQuakeWindowTab;

    TermTabStyle *m_termTabStyle = nullptr;
};

#endif  // TABBAR_H
