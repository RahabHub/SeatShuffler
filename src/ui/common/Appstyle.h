/**
 * @file AppStyle.h
 * @brief 全局样式表定义与按钮样式辅助工具
 *
 * 提供类 Fluent Design / Ant Design 风格的现代化 QSS 样式。
 * 主色调采用 #1677ff (Ant Design Blue)，辅以圆角、悬停动效等。
 *
 * 注意:
 *   Qt6 中全局 QSS 的 property 选择器在父控件存在 inline stylesheet 时
 *   会因级联优先级失效，因此主要按钮 / 危险按钮的样式通过
 *   applyPrimaryStyle / applyDangerStyle 以 inline stylesheet 直接应用。
 *
 * 使用方式:
 *   qApp->setStyleSheet(seat::AppStyle::styleSheet());
 *   seat::AppStyle::applyPrimaryStyle(myButton);
 */

#ifndef SEATSHUFFLER_APPSTYLE_H
#define SEATSHUFFLER_APPSTYLE_H

#include <QPushButton>
#include <QString>

namespace seat {

    class AppStyle {
    public:
        // ============================================================
        // 按钮样式辅助方法
        // ============================================================

        /** 应用主操作按钮样式 (蓝底白字) */
        static void applyPrimaryStyle(QPushButton *btn) {
            btn->setStyleSheet(QStringLiteral("QPushButton {"
                                              "  background-color: #1677ff;"
                                              "  color: #ffffff;"
                                              "  border: 1px solid #1677ff;"
                                              "  border-radius: 6px;"
                                              "  padding: 5px 16px;"
                                              "  min-height: 28px;"
                                              "  font-weight: bold;"
                                              "  outline: none;"
                                              "}"
                                              "QPushButton:hover {"
                                              "  background-color: #4096ff;"
                                              "  border-color: #4096ff;"
                                              "  color: #ffffff;"
                                              "}"
                                              "QPushButton:pressed {"
                                              "  background-color: #0958d9;"
                                              "  border-color: #0958d9;"
                                              "}"
                                              "QPushButton:focus {"
                                              "  outline: none;"
                                              "}"));
        }

        /** 应用危险操作按钮样式 (红色边框/文字) */
        static void applyDangerStyle(QPushButton *btn) {
            btn->setStyleSheet(QStringLiteral("QPushButton {"
                                              "  color: #ff4d4f;"
                                              "  border: 1px solid #ff4d4f;"
                                              "  background-color: #ffffff;"
                                              "  border-radius: 6px;"
                                              "  padding: 5px 16px;"
                                              "  min-height: 28px;"
                                              "  outline: none;"
                                              "}"
                                              "QPushButton:hover {"
                                              "  color: #ff7875;"
                                              "  border-color: #ff7875;"
                                              "  background-color: #fff2f0;"
                                              "}"
                                              "QPushButton:pressed {"
                                              "  color: #d9363e;"
                                              "  border-color: #d9363e;"
                                              "}"
                                              "QPushButton:focus {"
                                              "  outline: none;"
                                              "}"));
        }

        // ============================================================
        // 全局 QSS 样式表
        // ============================================================

        static QString styleSheet() {
            return QStringLiteral(R"(

/* ============================================================
 * 全局基础样式
 * ============================================================ */

* {
    outline: none;
}

QWidget {
    font-family: "Microsoft YaHei UI", "Segoe UI", "PingFang SC", sans-serif;
    font-size: 13px;
    color: #1f1f1f;
}

QMainWindow {
    background-color: #f0f2f5;
}

/* ============================================================
 * 滚动区域 & 滚动条
 * ============================================================ */

QScrollArea {
    border: none;
    background-color: transparent;
}

QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 0;
    border-radius: 4px;
}

QScrollBar::handle:vertical {
    background: #c4c4c4;
    min-height: 30px;
    border-radius: 4px;
}

QScrollBar::handle:vertical:hover {
    background: #a0a0a0;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
    height: 0px;
}

QScrollBar:horizontal {
    background: transparent;
    height: 8px;
    margin: 0;
    border-radius: 4px;
}

QScrollBar::handle:horizontal {
    background: #c4c4c4;
    min-width: 30px;
    border-radius: 4px;
}

QScrollBar::handle:horizontal:hover {
    background: #a0a0a0;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
    width: 0px;
}

/* ============================================================
 * 按钮基础样式
 * ============================================================ */

QPushButton {
    background-color: #ffffff;
    color: #1f1f1f;
    border: 1px solid #d9d9d9;
    border-radius: 6px;
    padding: 5px 16px;
    min-height: 28px;
    font-size: 13px;
    outline: none;
}

QPushButton:hover {
    color: #1677ff;
    border-color: #1677ff;
    background-color: #ffffff;
}

QPushButton:pressed {
    color: #0958d9;
    border-color: #0958d9;
    background-color: #f0f5ff;
}

QPushButton:focus {
    outline: none;
}

/* 导航栏按钮 (可选中态) */
QPushButton:checked {
    color: #ffffff;
    background-color: #1677ff;
    border-color: #1677ff;
}

QPushButton:checked:hover {
    background-color: #4096ff;
    border-color: #4096ff;
    color: #ffffff;
}

/* ============================================================
 * 输入框样式
 * ============================================================ */

QLineEdit {
    border: 1px solid #d9d9d9;
    border-radius: 6px;
    padding: 5px 12px;
    min-height: 28px;
    background-color: #ffffff;
    selection-background-color: #bae0ff;
}

QLineEdit:focus {
    border-color: #1677ff;
}

QLineEdit:hover {
    border-color: #4096ff;
}

QTextEdit {
    border: 1px solid #d9d9d9;
    border-radius: 6px;
    padding: 6px 12px;
    background-color: #ffffff;
    selection-background-color: #bae0ff;
}

QTextEdit:focus {
    border-color: #1677ff;
}

/* ============================================================
 * 下拉框
 * ============================================================ */

QComboBox {
    border: 1px solid #d9d9d9;
    border-radius: 6px;
    padding: 5px 12px;
    min-height: 28px;
    background-color: #ffffff;
}

QComboBox:hover {
    border-color: #4096ff;
}

QComboBox:focus, QComboBox:on {
    border-color: #1677ff;
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 28px;
    border: none;
}

QComboBox::down-arrow {
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 5px solid #8c8c8c;
    margin-right: 8px;
}

QComboBox QAbstractItemView {
    border: 1px solid #d9d9d9;
    border-radius: 6px;
    background-color: #ffffff;
    selection-background-color: #e6f4ff;
    selection-color: #1677ff;
    padding: 4px;
    outline: none;
}

QComboBox QAbstractItemView::item {
    min-height: 32px;
    padding: 4px 12px;
    border-radius: 4px;
}

QComboBox QAbstractItemView::item:hover {
    background-color: #f5f5f5;
}

/* ============================================================
 * 数字输入框
 * ============================================================ */

QSpinBox {
    border: 1px solid #d9d9d9;
    border-radius: 6px;
    padding: 4px 8px;
    min-height: 28px;
    background-color: #ffffff;
}

QSpinBox:hover {
    border-color: #4096ff;
}

QSpinBox:focus {
    border-color: #1677ff;
}

QSpinBox::up-button, QSpinBox::down-button {
    width: 20px;
    border: none;
    background: transparent;
}

/* ============================================================
 * 分组框
 * ============================================================ */

QGroupBox {
    background-color: #ffffff;
    border: 1px solid #e8e8e8;
    border-radius: 8px;
    margin-top: 12px;
    padding-top: 24px;
    padding-left: 12px;
    padding-right: 12px;
    padding-bottom: 8px;
    font-weight: bold;
    color: #1f1f1f;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 2px 12px;
    color: #595959;
    font-size: 13px;
}

/* ============================================================
 * 卡片容器 (QFrame::StyledPanel)
 * ============================================================ */

QFrame[frameShape="6"] {
    background-color: #ffffff;
    border: 1px solid #e8e8e8;
    border-radius: 8px;
    padding: 4px;
}

QFrame[frameShape="6"]:hover {
    border-color: #bae0ff;
    background-color: #fafafa;
}

/* ============================================================
 * 表格样式
 * ============================================================ */

QTableWidget {
    background-color: #ffffff;
    border: 1px solid #e8e8e8;
    border-radius: 8px;
    gridline-color: #f0f0f0;
    selection-background-color: #e6f4ff;
    selection-color: #1f1f1f;
    outline: none;
}

QTableWidget::item {
    padding: 6px 10px;
    border-bottom: 1px solid #f5f5f5;
}

QTableWidget::item:selected {
    background-color: #e6f4ff;
    color: #1f1f1f;
}

QTableWidget::item:hover {
    background-color: #fafafa;
}

QHeaderView::section {
    background-color: #fafafa;
    color: #595959;
    border: none;
    border-bottom: 1px solid #e8e8e8;
    border-right: 1px solid #f0f0f0;
    padding: 8px 10px;
    font-weight: bold;
    font-size: 13px;
}

QHeaderView::section:hover {
    background-color: #f0f0f0;
}

/* ============================================================
 * 单选按钮 —— 固定 indicator 尺寸，防止选中后布局偏移
 * ============================================================ */

QRadioButton {
    spacing: 8px;
    color: #1f1f1f;
    padding: 4px 2px;
}

QRadioButton::indicator {
    width: 18px;
    height: 18px;
    border-radius: 9px;
    border: 2px solid #d9d9d9;
    background-color: #ffffff;
}

QRadioButton::indicator:hover {
    border-color: #1677ff;
}

QRadioButton::indicator:checked {
    border: 2px solid #1677ff;
    background-color: #ffffff;
    image: none;
}

/* 用内嵌圆点表示选中，通过 background 渐变模拟 */
QRadioButton::indicator:checked {
    background: qradialgradient(
        cx:0.5, cy:0.5, radius:0.4,
        fx:0.5, fy:0.5,
        stop:0 #1677ff,
        stop:0.6 #1677ff,
        stop:0.7 #ffffff,
        stop:1.0 #ffffff
    );
    border: 2px solid #1677ff;
}

/* ============================================================
 * 标签
 * ============================================================ */

QLabel {
    color: #1f1f1f;
    background: transparent;
}

/* ============================================================
 * 对话框 & 消息框
 * ============================================================ */

QDialog {
    background-color: #ffffff;
    border-radius: 8px;
}

QDialogButtonBox QPushButton {
    min-width: 80px;
}

QMessageBox {
    background-color: #ffffff;
}

QMessageBox QPushButton {
    min-width: 80px;
    padding: 6px 20px;
}

/* ============================================================
 * 工具提示
 * ============================================================ */

QToolTip {
    background-color: rgba(0, 0, 0, 0.75);
    color: #ffffff;
    border: none;
    border-radius: 4px;
    padding: 6px 10px;
    font-size: 12px;
}

            )");
        }

    private:
        AppStyle() = default;
    };

} // namespace seat

#endif // SEATSHUFFLER_APPSTYLE_H
