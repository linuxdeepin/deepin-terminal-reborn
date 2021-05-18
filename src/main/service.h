/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
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
#ifndef SERVICE_H
#define SERVICE_H

#include "termproperties.h"
#include "windowsmanager.h"
#include "shortcutmanager.h"
#include "settings.h"
#include "customthemesettingdialog.h"

#include <com_deepin_wmswitcher.h>

#include <DSettingsDialog>
#include <DDialog>

#include <QObject>
#include <QSharedMemory>
#include <QMap>

DWIDGET_USE_NAMESPACE

using WMSwitcher = com::deepin::WMSwitcher;
#define WMSwitcherService "com.deepin.WMSwitcher"
#define WMSwitcherPath "/com/deepin/WMSwitcher"

/**
 * @brief 共享内存的数据结构
 */
struct ShareMemoryInfo {
    int     enableCreateTerminal = 0; //是否允许创建终端
    int     terminalsCount       = 0; //当前终端启东的数量
    qint64  appStartTime         = 0; //sub app 启动的时间
};

/**
 * @brief 全局的后台服务类
 *        管理所有底层数据的加载
 *        管理所有特殊的通知信号
 *        管理所有程序入口
 */
class Service : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Service的单例
     * @return
     */
    static Service *instance();

    /**
     * @brief 释放
     */
    static void releaseInstance();

    /**
     * @brief 初始化
     */
    void init();

    /**
     * @brief 初始化设置框，在窗口现实后初始化，使第一次出现设置框不至于卡顿
     */
    void initSetting();

    /**
     * @brief 显示设置框
     */
    void showSettingDialog(MainWindow *pOwner);

    /**
     * @brief 隐藏设置框
     */
    void hideSettingDialog();

    /**
     * @brief 显示自定义主题设置框
     */
    void showCustomThemeSettingDialog(MainWindow *pOwner);

    /**
     * @brief 显示设置快捷键冲突弹窗
     */
    void showShortcutConflictMsgbox(QString txt);

    /**
     * @brief 设置框是否显示
     */
    inline bool isSettingDialogVisible()
    {
        return m_settingDialog ? m_settingDialog->isVisible() : false;
    }

    /**
     * @brief 获取设置框的所有者
     */
    inline MainWindow *getSettingOwner()
    {
        return m_settingOwner;
    }

    /**
     * @brief 重置设置框的所有者
     */
    inline void resetSettingOwner()
    {
        m_settingOwner = nullptr;
    }

    /**
     * @brief i从term数量的角度判断是否允许继续创建
     * @return
     */
    bool isCountEnable();

    /**
     * @brief 获取是否是对话框显示
     * @return
     */
    bool getIsDialogShow() const;

    /**
     * @brief 设置是否对话框显示
     * @param parent
     * @param isDialogShow
     */
    void setIsDialogShow(QWidget *parent, bool isDialogShow);

    /**
     * @brief 子进程获取是否可以创建窗口许可，获取到权限立即将标志位置为false
              增加子进程启动的时间，如果可以创建，把该时间写入共享内存，当创建mainwindow的时候，取走这个数据
     * @param time
     * @return
     */
    bool getEnable(qint64 time);

    /**
     * @brief 设置子进程启动时间到共享内存
     */
    void setSubAppStartTime(qint64);

    /**
     * @brief 获取子进程在共享的启动时间
     * @return
     */
    qint64 getSubAppStartTime();

    /**
     * @brief ShareMemoryCount这个为当前总的终端数，但是雷神创建的第一个不包括在内
     * @param count 总的终端数
     */
    void updateShareMemoryCount(int count);

    /**
     * @brief 获取共享内存计数
     * @return
     */
    int  getShareMemoryCount();

    /**
     * @brief 设置共享内存信息
     * @param enable 1=true(主进程), 0=false(主进程首次或子进程获得许可以后)
     * @return
     */
    bool setMemoryEnable(bool enable);

    /**
     * @brief 获取共享内存标志， 1=true,0=false
     * @return
     */
    bool getMemoryEnable();

    /**
     * @brief 释放共享内存连接
     */
    void releaseShareMemory();

    /**
     * @brief 判断当前是否开启窗口特效,  开启-true 关闭-false
     * @return
     */
    bool isWindowEffectEnabled();

    /**
     * @brief 获取主程序初始进入的时间
     */
    qint64 getEntryTime();

    /**
     * @brief 从/etc/shells获取shell列表
     */
    QMap<QString, QString> getShells();

    /**
     * @brief 获取shells的map,不读取/etc/shells
     */
    QMap<QString, QString> shellsMap();

signals:
    /**
     * @brief 刷新面板
     * @param oldCmdName:旧的名称
     * @param newCmdName:新的名称
     */
    void refreshCommandPanel(QString oldCmdName, QString newCmdName);

    /**
     * @brief 切换编码列表的编码
     * @param encode:编码
     */
    void checkEncode(QString encode);

    /**
     * @brief 当前终端切换
     * @param term:当前终端
     */
    void currentTermChange(QWidget *term);

    /**
     * @brief 窗口特效是否开启
     * @param isEnabled
     */
    void onWindowEffectEnabled(bool isEnabled);

    /**
     * @brief 触控板事件
     * @param name:事件名称
     * @param direction:手势方向
     * @param fingers:手指个数
     */
    void touchPadEventSignal(QString name, QString direction, int fingers);

    /**
     * @brief hostnameChanged
     */
    void hostnameChanged();

public slots:
    /**
     * @brief 创建窗口的入口,Service进入接口
     * @param arguments
     */
    void Entry(QStringList arguments);

    /**
     * @brief 对桌面工作区切换事件的处理
     * @param curDesktop 当前工作区
     * @param nextDesktop 下一个工作区
     */
    void onDesktopWorkspaceSwitched(int curDesktop, int nextDesktop);

    /**
     * @brief slotShowSettingsDialog
     */
    void slotShowSettingsDialog();

    /**
     * @brief DSettingsDialog对话框关闭后的处理
     * @param result
     */
    void slotSettingsDialogFinished(int result);

    /**
     * @brief 处理窗口特效打开/关闭时，相关设置项目的显示/隐藏
     * @param wmName
     */
    void slotWMChanged(const QString &wmName);

    /**
     * @brief 自定义主题对话框关闭后的处理
     * @param result:DialogCode { Rejected, Accepted }
     */
    void slotCustomThemeSettingDialogFinished(int result);

    /**
     * @brief 快捷键冲突弹窗的完成信号
     */
    void slotSettingShortcutConflictDialogFinished();

private:
    explicit Service(QObject *parent = nullptr);

    ~Service() override;

    /**
     * @brief 显示/隐藏设置的透明度和背景模糊选项-- 仅UOS服务器版本使用
     * @param isShow:显示
     */
    void showHideOpacityAndBlurOptions(bool isShow);

    /**
     * @brief 监听窗口特效开关对应DBus信号，并实时显示/隐藏设置的透明度和背景模糊选项
     */
    void listenWindowEffectSwitcher();

private:
    static Service             *g_pService;
    DSettingsDialog            *m_settingDialog                     = nullptr;// 设置框 全局唯一显示
    CustomThemeSettingDialog   *m_customThemeSettingDialog          = nullptr;// 自定义主题设置对话框 全局唯一
    WMSwitcher                 *m_wmSwitcher                        = nullptr;
    MainWindow                 *m_settingOwner                      = nullptr;// 设置框的所有者
    DDialog                    *m_settingShortcutConflictDialog     = nullptr;// 设置框，快捷键冲突弹窗
    bool                        m_isDialogShow                      = false;  // 雷神用来判断是否有弹窗显示
    bool                        m_enable                            = false;  // 是否允许创建新的窗口
    QSharedMemory              *m_enableShareMemory                 = nullptr;
    ShareMemoryInfo            *m_pShareMemoryInfo                  = nullptr;// 这个指针实际上与上面指针指向同一地址，不需要二次释放
    QMap<QString, QString>      m_shellsMap;
    qint64                      m_entryTime                         = 0;      // 记录进入的时间，只有创建窗口时，才会来取用这个时间
};

#endif // SERVICE_H
