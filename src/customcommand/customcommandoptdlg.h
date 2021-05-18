/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: sunchengxi <sunchengxi@uniontech.com>
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

/*******************************************************************************
 1. @类名:    CustomCommandOptDlg
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-07-31
 4. @说明:    自定义命令操作窗口，进行删除和修改操作
*******************************************************************************/

#ifndef CUSTOMCOMMANDOPTDLG_H
#define CUSTOMCOMMANDOPTDLG_H

#include "termbasedialog.h"
#include "shortcutmanager.h"

#include <DDialog>
#include <DAbstractDialog>
#include <DLineEdit>
#include <DKeySequenceEdit>
#include <DLabel>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DSuggestButton>
#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QAction>
#include <QWidget>

DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(CustomCommandData)

class CustomCommandOptDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    enum CustomCmdOptType {
        CCT_ADD,     // the add type of custom command operation
        CCT_MODIFY,  // the modify type of custom command operation
    };

    explicit CustomCommandOptDlg(CustomCmdOptType type = CCT_ADD,
                                 CustomCommandData *currItemData = nullptr,
                                 QWidget *parent = nullptr);
    ~CustomCommandOptDlg()override;

    /**
     * @brief 增加内容布局
     * @author sunchengxi
     * @param content 内容布局
     */
    void addContent(QWidget *content);
    /**
     * @brief 操作界面增加取消和确认按钮布局
     * @author sunchengxi
     */
    void addCancelConfirmButtons();

    /**
     * @brief 设置标题图标
     * @author sunchengxi
     * @param iconPixmap 标题图标
     */
    void setIconPixmap(const QPixmap &iconPixmap);
    /**
     * @brief 设置取消按钮的内容文本
     * @author sunchengxi
     * @param strCancel 取消按钮的内容文本
     */
    void setCancelBtnText(const QString &strCancel);
    /**
     * @brief 设置确认按钮的内容文本
     * @author sunchengxi
     * @param strConfirm 确认按钮的内容文本
     */
    void setConfirmBtnText(const QString &strConfirm);
    /**
     * @brief 显示快捷键冲突弹窗
     * @author ut000610 戴正文
     * @param txt
     */
    void showShortcutConflictMsgbox(QString txt);
    /**
     * @brief 关闭信号槽关联
     * @author sunchengxi
     */
    void closeRefreshDataConnection();
    /**
     * @brief 快捷键冲突判断
     * @author sunchengxi
     * @param sequence
     * @return
     */
    bool checkSequence(const QKeySequence &sequence);
    /**
     * @brief 设置模型索引
     * @author sunchengxi
     * @param mi
     */
    void setModelIndex(QModelIndex mi);

    /**
     * @brief 获取返回结果
     * @author sunchengxi
     * @return
     */
    QDialog::DialogCode getConfirmResult();
    /**
     * @brief 获取主布局
     * @author sunchengxi
     * @return
     */
    QVBoxLayout *getMainLayout();

    /**
     * @brief 获取当前快捷键
     * @author sunchengxi
     * @return
     */
    QAction *getCurCustomCmd();
    /**
     * @brief 判断是否是删除操作
     * @author sunchengxi
     * @return
     */
    bool isDelCurCommand();
    CustomCommandData *m_currItemData = nullptr;    //当前自定义列表项数据指针
    QModelIndex modelIndex;                             //模型索引
    DDialog *m_dlgDelete = nullptr;                     //删除确认窗口指针
    unsigned int  m_iTabModifyTime = 0;                 //键盘操作进行修改的时间戳

protected:
    /**
     * @brief 初始化操作界面的标题布局
     * @author sunchengxi
     */
    void initUITitle();
    /**
     * @brief 字体颜色跟随主题变化响应槽初始化
     * @author sunchengxi
     */
    void initTitleConnections();
    //此接口暂时注释保留不删除，2020.12.23
    //void setLogoVisable(bool visible = true);
    /**
     * @brief 设置自定义命令操作窗口的标题
     * @author sunchengxi
     * @param title 标题
     */
    void setTitle(const QString &title);
    /**
     * @brief 获取自定义命令操作窗口的内容布局
     * @author sunchengxi
     * @return
     */
    QLayout *getContentLayout();

    //Overrides
    /**
     * @brief 自定义命令操作关闭事件
     * @author sunchengxi
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent *event) override;
signals:
    void closed();
    void confirmBtnClicked();

private slots:
    /**
     * @brief 自定义命令操作确认按钮响应槽
     * @author sunchengxi
     */
    void slotAddSaveButtonClicked();
    /**
     * @brief 删除按钮触发响应槽
     * @author sunchengxi
     */
    void slotDelCurCustomCommand();
    /**
     * @brief 刷新数据槽
     * @author sunchengxi
     * @param oldCmdName 旧的自定义命令的名称
     * @param newCmdName 新的自定义命令的名称
     */
    void slotRefreshData(QString oldCmdName, QString newCmdName);

    void slotNameLineEditingFinished();
    void slotCommandLineEditingFinished();
    void slotShortCutLineEditingFinished(const QKeySequence &sequence);

    void slotCloseButtonClicked();
    void slotThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);

    void slotCancelBtnClicked();
    void slotConfirmBtnClicked();
    void slotShortCutLineEditFinished();
    void slotShortcutConflictDialogFinished();

    void slotSetShortCutLineEditFocus();

private:
    /**
     * @brief 初始化自定义命令操作窗口界面布局
     * @author sunchengxi
     */
    void initUI();
    /**
     * @brief 终端中选中内容，创建自定义命令，选中内容没有自动粘贴到命令输入框
     * @author sunchengxi
     */
    void initCommandFromClipBoardText();

    CustomCmdOptType m_type;
    QAction *m_newAction = nullptr;
    DLineEdit *m_nameLineEdit = nullptr;
    DLineEdit *m_commandLineEdit = nullptr;
    DKeySequenceEdit *m_shortCutLineEdit = nullptr;
    QString m_lastCmdShortcut;
    bool m_bDelOpt = false;

    QWidget *m_titleBar = nullptr;
    DLabel  *m_logoIcon = nullptr;
    DLabel  *m_titleText = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;

    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    DPushButton *m_cancelBtn = nullptr;
    DSuggestButton *m_confirmBtn = nullptr;

    QDialog::DialogCode m_confirmResultCode;
    // 快捷键冲突弹窗
    DDialog *m_shortcutConflictDialog = nullptr;
    bool m_bRefreshCheck = false;


    //数字常量
    const int m_iLayoutSizeZero = 0;
    const int m_iLayoutLeftSize = 28;
    const int m_iLayoutRightSize = 30;

    const int m_iFixedWidth = 459;
    const int m_iFixedHeight = 54;

    const int m_iFixedHeightAddSize = 262;
    const int m_iFixedHeightEditSize = 296;

    const int m_iLineEditWidth = 285;

    const int m_iSpaceSizeZero = 0;
    const int m_iSpaceSizeTen = 10;
    const int m_iSpaceSizeEighteen = 18;

    const int m_iSingleShotTime = 30;

};

#endif  // CUSTOMCOMMANDOPTDLG_H
