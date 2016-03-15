#include "dfileitemdelegate.h"
#include "fileitem.h"
#include "../app/global.h"
#include "dfilesystemmodel.h"

#include <QLabel>
#include <QPainter>
#include <QTextEdit>

DFileItemDelegate::DFileItemDelegate(DFileView *parent) :
    QStyledItemDelegate(parent)
{
    focus_item = new FileIconItem(parent->viewport());
    focus_item->setAttribute(Qt::WA_TransparentForMouseEvents);
    focus_item->canDeferredDelete = false;
    focus_item->icon->setFixedSize(parent->iconSize());
    /// prevent flash when first call show()
    focus_item->setFixedWidth(0);

    connect(parent, &DListView::triggerEdit,
            this, [this, parent](const QModelIndex &index) {
        if(index == focus_index) {
            parent->setIndexWidget(index, 0);
            focus_item->hide();
            focus_index = QModelIndex();
            parent->edit(index);
        }
    });

    connect(parent, &DListView::iconSizeChanged,
            this, [this] {
        m_elideMap.clear();
        m_wordWrapMap.clear();
        m_textHeightMap.clear();
    });
}

void DFileItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if(parent()->isIconViewMode()) {
        paintIconItem(painter, option, index);
    } else {
        int column_count = parent()->model()->columnCount(index);
        int column_x = parent()->columnWidth(0);

        paintListItem(painter, option, index);

        for(int i = 1; i < column_count; ++i) {
            QStyleOptionViewItem opt;

            opt.rect = option.rect;
            opt.rect.setWidth(parent()->columnWidth(i));
            opt.rect.moveLeft(column_x);

            column_x += opt.rect.width();

            QModelIndex tmp_index = parent()->model()->index(index.row(), i, index.parent());

            paintListItem(painter, opt, tmp_index);
        }
    }
}

QSize DFileItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    if(parent()->isIconViewMode())
        return parent()->iconSize() * 1.8;
    else
        return QSize(-1, 30);
}

QWidget *DFileItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    FileIconItem *item = new FileIconItem(parent);

    editing_index = index;

    connect(item, &FileIconItem::destroyed, this, [this] {
        editing_index = QModelIndex();
    });

    return item;
}

void DFileItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->move(option.rect.topLeft());
    editor->setFixedWidth(option.rect.width());

    FileIconItem *item = static_cast<FileIconItem*>(editor);

    if(!item)
        return;

    item->icon->setFixedSize(parent()->iconSize());
}

void DFileItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    FileIconItem *item = static_cast<FileIconItem*>(editor);

    if(!item)
        return;

    QStyleOptionViewItem opt;

    initStyleOption(&opt, index);

    item->icon->setPixmap(opt.icon.pixmap(parent()->iconSize()));
    item->edit->setPlainText(index.data().toString());
    item->edit->setAlignment(Qt::AlignHCenter);
    item->edit->document()->setTextWidth(parent()->iconSize().width() * 1.8);
    item->edit->setFixedSize(item->edit->document()->size().toSize());
}

void DFileItemDelegate::paintIconItem(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    if(index == focus_index || index == editing_index)
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    /// init icon geomerty

    QRect icon_rect = opt.rect;

    icon_rect.setSize(parent()->iconSize());
    icon_rect.moveCenter(opt.rect.center());
    icon_rect.moveTop(opt.rect.top());

    /// init file name geometry

    QRect label_rect = opt.rect;

    label_rect.setTop(icon_rect.bottom() + 10);
    painter->setFont(opt.font);

    QString str = index.data(Qt::DisplayRole).toString();

    if(str.isEmpty()) {
        /// draw icon

        opt.icon.paint(painter, icon_rect);

        return;
    }

    /// if has focus show all file name else show elide file name.

    if(opt.state & QStyle::State_HasFocus) {
        if(focus_index.isValid()) {
            parent()->setIndexWidget(focus_index, 0);
            focus_item->hide();
            focus_index = QModelIndex();
        }

        int height = 0;

        /// init file name text

        if(m_wordWrapMap.contains(str)) {
            str = m_wordWrapMap.value(str);
            height = m_textHeightMap.value(str);
        } else {
            QString wordWrap_str = Global::wordWrapText(str, label_rect.width(), opt.fontMetrics,
                                                        QTextOption::WrapAtWordBoundaryOrAnywhere, &height);

            m_wordWrapMap[str] = wordWrap_str;
            m_textHeightMap[wordWrap_str] = height;
            str = wordWrap_str;
        }

        if(height > label_rect.height()) {
            /// use widget(FileIconItem) show file icon and file name label.

            focus_index = index;

            setEditorData(focus_item, index);
            parent()->setIndexWidget(index, focus_item);

            return;
        }
    } else {
        /// init file name text

        if(m_elideMap.contains(str)) {
            str = m_elideMap.value(str);
        } else {
            QString elide_str = Global::elideText(str, label_rect.size(),
                                                  opt.fontMetrics,
                                                  QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                  opt.textElideMode);

            m_elideMap[str] = elide_str;

            str = elide_str;
        }
    }

    /// draw background

    if((opt.state & QStyle::State_Selected) && opt.showDecorationSelected) {
        QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                      ? QPalette::Normal : QPalette::Disabled;
        QColor backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                     ? QPalette::Highlight : QPalette::Window);

        painter->fillRect(opt.rect, backgroundColor);
    }

    /// draw icon and file name label

    opt.icon.paint(painter, icon_rect);
    painter->drawText(label_rect, Qt::AlignHCenter, str);
}

void DFileItemDelegate::paintListItem(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;

    initStyleOption(&opt, index);

    /// draw background

    if((opt.state & QStyle::State_Selected) && opt.showDecorationSelected) {
        QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                      ? QPalette::Normal : QPalette::Disabled;
        QColor backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                     ? QPalette::Highlight : QPalette::Window);

        painter->fillRect(opt.rect, backgroundColor);
    }

    QRect name_rect = opt.rect;
    QString str = index.data(Qt::DisplayRole).toString();

    /// draw icon

    if(!opt.icon.isNull()) {
        QRect icon_rect = opt.rect;

        icon_rect.setSize(parent()->iconSize());
        opt.icon.paint(painter, icon_rect);

        name_rect.setLeft(icon_rect.right() + 10);
    }

    /// draw file name label

    painter->drawText(name_rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()), str);
}