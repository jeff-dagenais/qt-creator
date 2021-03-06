/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://www.qt.io/licensing.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "watchwindow.h"

#include "breakhandler.h"
#include "registerhandler.h"
#include "debuggeractions.h"
#include "debuggerinternalconstants.h"
#include "debuggercore.h"
#include "debuggerdialogs.h"
#include "debuggerengine.h"
#include "watchdelegatewidgets.h"
#include "watchhandler.h"
#include "debuggertooltipmanager.h"
#include "memoryagent.h"

#include <texteditor/syntaxhighlighter.h>

#include <utils/qtcassert.h>
#include <utils/savedaction.h>
#include <utils/fancylineedit.h>

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemDelegate>
#include <QMenu>
#include <QMessageBox>
#include <QMetaProperty>
#include <QMimeData>
#include <QScrollBar>
#include <QTimer>

// For InputDialog, move to Utils?
#include <coreplugin/helpmanager.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QDialogButtonBox>

//#define USE_WATCH_MODEL_TEST 1

#if USE_WATCH_MODEL_TEST
#include <modeltest.h>
#endif

Q_DECLARE_METATYPE(QModelIndex)

/////////////////////////////////////////////////////////////////////
//
// WatchDelegate
//
/////////////////////////////////////////////////////////////////////

namespace Debugger {
namespace Internal {

const char CurrentIndex[] = "CurrentIndex";

static DebuggerEngine *currentEngine()
{
    return debuggerCore()->currentEngine();
}

class WatchDelegate : public QItemDelegate
{
public:
    explicit WatchDelegate(WatchTreeView *parent)
        : QItemDelegate(parent), m_watchWindow(parent)
    {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
        const QModelIndex &index) const
    {
        // Value column: Custom editor. Apply integer-specific settings.
        if (index.column() == 1) {
            const QVariant::Type type =
                static_cast<QVariant::Type>(index.data(LocalsEditTypeRole).toInt());
            switch (type) {
            case QVariant::Bool:
                return new BooleanComboBox(parent);
            default:
                break;
            }
            WatchLineEdit *edit = WatchLineEdit::create(type, parent);
            edit->setFrame(false);
            IntegerWatchLineEdit *intEdit
                = qobject_cast<IntegerWatchLineEdit *>(edit);
            if (intEdit)
                intEdit->setBase(index.data(LocalsIntegerBaseRole).toInt());
            return edit;
        }

        // Standard line edits for the rest.
        Utils::FancyLineEdit *lineEdit = new Utils::FancyLineEdit(parent);
        lineEdit->setFrame(false);
        lineEdit->setHistoryCompleter(QLatin1String("WatchItems"));
        return lineEdit;
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const
    {
        // Standard handling for anything but the watcher name column (change
        // expression), which removes/recreates a row, which cannot be done
        // in model->setData().
        if (index.column() != 0) {
            QItemDelegate::setModelData(editor, model, index);
            return;
        }
        const QMetaProperty userProperty = editor->metaObject()->userProperty();
        QTC_ASSERT(userProperty.isValid(), return);
        const QString value = editor->property(userProperty.name()).toString();
        const QString exp = index.data(LocalsExpressionRole).toString();
        if (exp == value)
            return;
        WatchHandler *handler = currentEngine()->watchHandler();
        handler->removeData(index.data(LocalsINameRole).toByteArray());
        m_watchWindow->watchExpression(value);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
        const QModelIndex &) const
    {
        editor->setGeometry(option.rect);
    }

private:
    WatchTreeView *m_watchWindow;
};

// Watch model query helpers.
static inline quint64 addressOf(const QModelIndex &m)
{
    return m.data(LocalsObjectAddressRole).toULongLong();
}

static inline quint64 pointerAddressOf(const QModelIndex &m)
{
    return m.data(LocalsPointerAddressRole).toULongLong();
}

static inline QString nameOf(const QModelIndex &m)
{
    return m.data().toString();
}

static inline QString typeOf(const QModelIndex &m)
{
    return m.data(LocalsTypeRole).toString();
}

static inline uint sizeOf(const QModelIndex &m)
{
    return m.data(LocalsSizeRole).toUInt();
}

// Create a map of value->name for register markup.
typedef QMap<quint64, QString> RegisterMap;
typedef RegisterMap::const_iterator RegisterMapConstIt;

RegisterMap registerMap(const DebuggerEngine *engine)
{
    RegisterMap result;
    foreach (const Register &reg, engine->registerHandler()->registers()) {
        const QVariant v = reg.editValue();
        if (v.type() == QVariant::ULongLong)
            result.insert(v.toULongLong(), QString::fromLatin1(reg.name));
    }
    return result;
}

// Helper functionality to indicate the area of a member variable in
// a vector representing the memory area by a unique color
// number and tooltip. Parts of it will be overwritten when recursing
// over the children.

typedef QPair<int, QString> ColorNumberToolTip;
typedef QVector<ColorNumberToolTip> ColorNumberToolTips;

static QString variableToolTip(const QString &name, const QString &type,
    quint64 offset)
{
    return offset ?
           //: HTML tooltip of a variable in the memory editor
           WatchTreeView::tr("<i>%1</i> %2 at #%3").
               arg(type, name).arg(offset) :
           //: HTML tooltip of a variable in the memory editor
           WatchTreeView::tr("<i>%1</i> %2").arg(type, name);
}

static int memberVariableRecursion(const QAbstractItemModel *model,
                                   const QModelIndex &modelIndex,
                                   const QString &name,
                                   quint64 start, quint64 end,
                                   int *colorNumberIn,
                                   ColorNumberToolTips *cnmv)
{
    int childCount = 0;
    QTC_ASSERT(modelIndex.isValid(), return childCount );
    const int rowCount = model->rowCount(modelIndex);
    if (!rowCount)
        return childCount;
    const QString nameRoot = name.isEmpty() ? name : name +  QLatin1Char('.');
    for (int r = 0; r < rowCount; r++) {
        const QModelIndex childIndex = modelIndex.child(r, 0);
        const quint64 childAddress = addressOf(childIndex);
        const uint childSize = sizeOf(childIndex);
        if (childAddress && childAddress >= start
                && (childAddress + childSize) <= end) { // Non-static, within area?
            const QString childName = nameRoot + nameOf(childIndex);
            const quint64 childOffset = childAddress - start;
            const QString toolTip
                = variableToolTip(childName, typeOf(childIndex), childOffset);
            const ColorNumberToolTip colorNumberNamePair((*colorNumberIn)++, toolTip);
            const ColorNumberToolTips::iterator begin = cnmv->begin() + childOffset;
            qFill(begin, begin + childSize, colorNumberNamePair);
            childCount++;
            childCount += memberVariableRecursion(model, childIndex,
                            childName, start, end, colorNumberIn, cnmv);
        }
    }
    return childCount;
}

typedef QList<MemoryMarkup> MemoryMarkupList;

/*!
    Creates markup for a variable in the memory view.

    Marks the visible children with alternating colors in the parent, that is, for
    \code
    struct Foo {
    char c1
    char c2
    int x2;
    QPair<int, int> pair
    }
    \endcode
    create something like:
    \code
    0 memberColor1
    1 memberColor2
    2 base color (padding area of parent)
    3 base color
    4 member color1
    ...
    8 memberColor2 (pair.first)
    ...
    12 memberColor1 (pair.second)
    \endcode

    In addition, registers pointing into the area are shown as 1 byte-markers.

   Fixme: When dereferencing a pointer, the size of the pointee is not
   known, currently. So, we take an area of 1024 and fill the background
   with the default color so that just the members are shown
   (sizeIsEstimate=true). This could be fixed by passing the pointee size
   as well from the debugger, but would require expensive type manipulation.

   \note To recurse over the top level items of the model, pass an invalid model
   index.

    \sa Debugger::Internal::MemoryViewWidget
*/
static MemoryMarkupList
    variableMemoryMarkup(const QAbstractItemModel *model,
                         const QModelIndex &modelIndex,
                         const QString &rootName,
                         const QString &rootToolTip,
                         quint64 address, quint64 size,
                         const RegisterMap &registerMap,
                         bool sizeIsEstimate,
                         const QColor &defaultBackground)
{
    enum { debug = 0 };
    enum { registerColorNumber = 0x3453 };

    if (debug)
        qDebug() << address << ' ' << size << rootName << rootToolTip;
    // Starting out from base, create an array representing the area
    // filled with base color. Fill children with some unique color numbers,
    // leaving the padding areas of the parent colored with the base color.
    MemoryMarkupList result;
    int colorNumber = 0;
    ColorNumberToolTips ranges(size, ColorNumberToolTip(colorNumber, rootToolTip));
    colorNumber++;
    const int childCount = memberVariableRecursion(model, modelIndex,
                                                   rootName, address, address + size,
                                                   &colorNumber, &ranges);
    if (sizeIsEstimate && !childCount)
        return result; // Fixme: Exact size not known, no point in filling if no children.
    // Punch in registers as 1-byte markers on top.
    const RegisterMapConstIt regcEnd = registerMap.constEnd();
    for (RegisterMapConstIt it = registerMap.constBegin(); it != regcEnd; ++it) {
        if (it.key() >= address) {
            const quint64 offset = it.key() - address;
            if (offset < size) {
                ranges[offset] = ColorNumberToolTip(registerColorNumber,
                           WatchTreeView::tr("Register <i>%1</i>").arg(it.value()));
            } else {
                break; // Sorted.
            }
        }
    } // for registers.
    if (debug) {
        QDebug dbg = qDebug().nospace();
        dbg << rootToolTip << ' ' << address << ' ' << size << '\n';
        QString name;
        for (unsigned i = 0; i < size; ++i)
            if (name != ranges.at(i).second) {
                dbg << ",[" << i << ' ' << ranges.at(i).first << ' '
                    << ranges.at(i).second << ']';
                name = ranges.at(i).second;
            }
    }

    // Assign colors from a list, use base color for 0 (contrast to black text).
    // Overwrite the first color (which is usually very bright) by the base color.
    QList<QColor> colors = TextEditor::SyntaxHighlighter::generateColors(colorNumber + 2,
                                                                         QColor(Qt::black));
    colors[0] = sizeIsEstimate ? defaultBackground : Qt::lightGray;
    const QColor registerColor = Qt::green;
    int lastColorNumber = 0;
    for (unsigned i = 0; i < size; ++i) {
        const ColorNumberToolTip &range = ranges.at(i);
        if (result.isEmpty() || lastColorNumber != range.first) {
            lastColorNumber = range.first;
            const QColor color = range.first == registerColorNumber ?
                         registerColor : colors.at(range.first);
            result.push_back(MemoryMarkup(address + i, 1, color, range.second));
        } else {
            result.back().length++;
        }
    }

    if (debug) {
        QDebug dbg = qDebug().nospace();
        dbg << rootName << ' ' << address << ' ' << size << '\n';
        QString name;
        for (unsigned i = 0; i < size; ++i)
            if (name != ranges.at(i).second) {
                dbg << ',' << i << ' ' << ranges.at(i).first << ' '
                    << ranges.at(i).second;
                name = ranges.at(i).second;
            }
        dbg << '\n';
        foreach (const MemoryMarkup &m, result)
            dbg << m.address <<  ' ' << m.length << ' '  << m.toolTip << '\n';
    }

    return result;
}

// Convenience to create a memory view of a variable.
static void addVariableMemoryView(DebuggerEngine *engine, bool separateView,
    const QModelIndex &m, bool atPointerAddress,
    const QPoint &p, QWidget *parent)
{
    const QColor background = parent->palette().color(QPalette::Normal, QPalette::Base);
    const quint64 address = atPointerAddress ? pointerAddressOf(m) : addressOf(m);
    // Fixme: Get the size of pointee (see variableMemoryMarkup())?
    const QString rootToolTip = variableToolTip(nameOf(m), typeOf(m), 0);
    const quint64 typeSize = sizeOf(m);
    const bool sizeIsEstimate = atPointerAddress || !typeSize;
    const quint64 size    = sizeIsEstimate ? 1024 : typeSize;
    if (!address)
         return;
    const QList<MemoryMarkup> markup =
        variableMemoryMarkup(m.model(), m, nameOf(m), rootToolTip,
                             address, size,
                             registerMap(engine),
                             sizeIsEstimate, background);
    const unsigned flags = separateView
        ? DebuggerEngine::MemoryView|DebuggerEngine::MemoryReadOnly : 0;
    const QString title = atPointerAddress
        ?  WatchTreeView::tr("Memory at Pointer's Address \"%1\" (0x%2)")
                .arg(nameOf(m)).arg(address, 0, 16)
        : WatchTreeView::tr("Memory at Object's Address \"%1\" (0x%2)")
                .arg(nameOf(m)).arg(address, 0, 16);
    engine->openMemoryView(address, flags, markup, p, title, parent);
}

// Add a memory view of the stack layout showing local variables
// and registers.
static void addStackLayoutMemoryView(DebuggerEngine *engine, bool separateView,
    const QAbstractItemModel *m, const QPoint &p, QWidget *parent)
{
    QTC_ASSERT(engine && m, return);

    // Determine suitable address range from locals.
    quint64 start = Q_UINT64_C(0xFFFFFFFFFFFFFFFF);
    quint64 end = 0;
    const QModelIndex localsIndex = m->index(0, 0);
    QTC_ASSERT(localsIndex.data(LocalsINameRole).toString() == QLatin1String("local"), return);
    const int localsItemCount = m->rowCount(localsIndex);
    // Note: Unsorted by default. Exclude 'Automatically dereferenced
    // pointer' items as they are outside the address range.
    for (int r = 0; r < localsItemCount; r++) {
        const QModelIndex idx = localsIndex.child(r, 0);
        const quint64 pointerAddress = pointerAddressOf(idx);
        if (pointerAddress == 0) {
            const quint64 address = addressOf(idx);
            if (address) {
                if (address < start)
                    start = address;
                const uint size = qMax(1u, sizeOf(idx));
                if (address + size > end)
                    end = address + size;
            }
        }
    }
    if (const quint64 remainder = end % 8)
        end += 8 - remainder;
    // Anything found and everything in a sensible range (static data in-between)?
    if (end <= start || end - start > 100 * 1024) {
        QMessageBox::information(parent,
            WatchTreeView::tr("Cannot Display Stack Layout"),
            WatchTreeView::tr("Could not determine a suitable address range."));
        return;
    }
    // Take a look at the register values. Extend the range a bit if suitable
    // to show stack/stack frame pointers.
    const RegisterMap regMap = registerMap(engine);
    const RegisterMapConstIt regcEnd = regMap.constEnd();
    for (RegisterMapConstIt it = regMap.constBegin(); it != regcEnd; ++it) {
        const quint64 value = it.key();
        if (value < start && start - value < 512)
            start = value;
        else if (value > end && value - end < 512)
            end = value + 1;
    }
    // Indicate all variables.
    const QColor background = parent->palette().color(QPalette::Normal, QPalette::Base);
    const MemoryMarkupList markup =
        variableMemoryMarkup(m, localsIndex, QString(),
                             QString(), start, end - start,
                             regMap, true, background);
    const unsigned flags = separateView
        ? (DebuggerEngine::MemoryView|DebuggerEngine::MemoryReadOnly) : 0;
    const QString title =
        WatchTreeView::tr("Memory Layout of Local Variables at 0x%1").arg(start, 0, 16);
    engine->openMemoryView(start, flags, markup, p, title, parent);
}

/////////////////////////////////////////////////////////////////////
//
// WatchWindow
//
/////////////////////////////////////////////////////////////////////

WatchTreeView::WatchTreeView(WatchType type)
  : m_type(type)
{
    setObjectName(QLatin1String("WatchWindow"));
    m_grabbing = false;
    setWindowTitle(tr("Locals and Expressions"));
    setIndentation(indentation() * 9/10);
    setUniformRowHeights(true);
    setItemDelegate(new WatchDelegate(this));
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    connect(this, SIGNAL(expanded(QModelIndex)),
        SLOT(expandNode(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)),
        SLOT(collapseNode(QModelIndex)));
}

void WatchTreeView::expandNode(const QModelIndex &idx)
{
    setModelData(LocalsExpandedRole, true, idx);
}

void WatchTreeView::collapseNode(const QModelIndex &idx)
{
    setModelData(LocalsExpandedRole, false, idx);
}

void WatchTreeView::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Delete && m_type == WatchersType) {
        WatchHandler *handler = currentEngine()->watchHandler();
        foreach (const QModelIndex &idx, activeRows())
            handler->removeData(idx.data(LocalsINameRole).toByteArray());
    } else if (ev->key() == Qt::Key_Return
            && ev->modifiers() == Qt::ControlModifier
            && m_type == LocalsType) {
        QModelIndex idx = currentIndex();
        QModelIndex idx1 = idx.sibling(idx.row(), 0);
        QString exp = model()->data(idx1).toString();
        watchExpression(exp);
    }
    BaseTreeView::keyPressEvent(ev);
}

void WatchTreeView::dragEnterEvent(QDragEnterEvent *ev)
{
    //BaseTreeView::dragEnterEvent(ev);
    if (ev->mimeData()->hasText()) {
        ev->setDropAction(Qt::CopyAction);
        ev->accept();
    }
}

void WatchTreeView::dragMoveEvent(QDragMoveEvent *ev)
{
    //BaseTreeView::dragMoveEvent(ev);
    if (ev->mimeData()->hasText()) {
        ev->setDropAction(Qt::CopyAction);
        ev->accept();
    }
}

void WatchTreeView::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasText()) {
        QString exp;
        QString data = ev->mimeData()->text();
        foreach (const QChar c, data)
            exp.append(c.isPrint() ? c : QChar(QLatin1Char(' ')));
        currentEngine()->watchHandler()->watchVariable(exp);
        //ev->acceptProposedAction();
        ev->setDropAction(Qt::CopyAction);
        ev->accept();
    }
    //BaseTreeView::dropEvent(ev);
}

void WatchTreeView::mouseDoubleClickEvent(QMouseEvent *ev)
{
    const QModelIndex idx = indexAt(ev->pos());
    if (!idx.isValid()) {
        inputNewExpression();
        return;
    }
    BaseTreeView::mouseDoubleClickEvent(ev);
}

// Text for add watch action with truncated expression.
static QString addWatchActionText(QString exp)
{
    if (exp.isEmpty())
        return WatchTreeView::tr("Add Expression Evaluator");
    if (exp.size() > 30) {
        exp.truncate(30);
        exp.append(QLatin1String("..."));
    }
    return WatchTreeView::tr("Add Expression Evaluator for \"%1\"").arg(exp);
}

// Text for add watch action with truncated expression.
static QString removeWatchActionText(QString exp)
{
    if (exp.isEmpty())
        return WatchTreeView::tr("Remove Expression Evaluator");
    if (exp.size() > 30) {
        exp.truncate(30);
        exp.append(QLatin1String("..."));
    }
    return WatchTreeView::tr("Remove Expression Evaluator for \"%1\"")
        .arg(exp.replace(QLatin1Char('&'), QLatin1String("&&")));
}

static void copyToClipboard(const QString &clipboardText)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(clipboardText, QClipboard::Selection);
    clipboard->setText(clipboardText, QClipboard::Clipboard);
}

void WatchTreeView::fillFormatMenu(QMenu *formatMenu, const QModelIndex &mi)
{
    QTC_CHECK(mi.isValid());

    const QModelIndex mi2 = mi.sibling(mi.row(), 2);
    const QString type = mi2.data().toString();

    const TypeFormatList alternativeFormats =
        mi.data(LocalsTypeFormatListRole).value<TypeFormatList>();
    int typeFormat =
        mi.data(LocalsTypeFormatRole).toInt();
    const int individualFormat =
        mi.data(LocalsIndividualFormatRole).toInt();
    const int unprintableBase = WatchHandler::unprintableBase();

    QAction *showUnprintableUnicode = 0;
    QAction *showUnprintableEscape = 0;
    QAction *showUnprintableOctal = 0;
    QAction *showUnprintableHexadecimal = 0;
    showUnprintableUnicode =
        formatMenu->addAction(tr("Treat All Characters as Printable"));
    showUnprintableUnicode->setCheckable(true);
    showUnprintableUnicode->setChecked(unprintableBase == 0);
    showUnprintableUnicode->setData(0);
    showUnprintableEscape =
        formatMenu->addAction(tr("Show Unprintable Characters as Escape Sequences"));
    showUnprintableEscape->setCheckable(true);
    showUnprintableEscape->setChecked(unprintableBase == -1);
    showUnprintableEscape->setData(-1);
    showUnprintableOctal =
        formatMenu->addAction(tr("Show Unprintable Characters as Octal"));
    showUnprintableOctal->setCheckable(true);
    showUnprintableOctal->setChecked(unprintableBase == 8);
    showUnprintableOctal->setData(8);
    showUnprintableHexadecimal =
        formatMenu->addAction(tr("Show Unprintable Characters as Hexadecimal"));
    showUnprintableHexadecimal->setCheckable(true);
    showUnprintableHexadecimal->setChecked(unprintableBase == 16);
    showUnprintableHexadecimal->setData(16);

    connect(showUnprintableUnicode, SIGNAL(triggered()), SLOT(onShowUnprintable()));
    connect(showUnprintableEscape, SIGNAL(triggered()), SLOT(onShowUnprintable()));
    connect(showUnprintableOctal, SIGNAL(triggered()), SLOT(onShowUnprintable()));
    connect(showUnprintableHexadecimal, SIGNAL(triggered()), SLOT(onShowUnprintable()));


    const QString spacer = QLatin1String("     ");
    formatMenu->addSeparator();
    QAction *dummy = formatMenu->addAction(
        tr("Change Display for Object Named \"%1\":").arg(mi.data().toString()));
    dummy->setEnabled(false);
    QString msg = (individualFormat == AutomaticFormat && typeFormat != AutomaticFormat)
        ? tr("Use Format for Type (Currently %1)")
            .arg(alternativeFormats.find(typeFormat).display)
        : tr("Use Display Format Based on Type") + QLatin1Char(' ');

    QAction *clearIndividualFormatAction = formatMenu->addAction(spacer + msg);
    clearIndividualFormatAction->setCheckable(true);
    clearIndividualFormatAction->setChecked(individualFormat == AutomaticFormat);
    connect(clearIndividualFormatAction, SIGNAL(triggered()),
        SLOT(onClearIndividualFormat()));

    for (int i = 0; i != alternativeFormats.size(); ++i) {
        const QString display = spacer + alternativeFormats.at(i).display;
        const int format = alternativeFormats.at(i).format;
        QAction *act = new QAction(display, formatMenu);
        act->setData(format);
        act->setCheckable(true);
        act->setChecked(format == individualFormat);
        act->setProperty(CurrentIndex, QVariant::fromValue(mi));
        formatMenu->addAction(act);
        connect(act, SIGNAL(triggered()), SLOT(onIndividualFormatChange()));
    }

    formatMenu->addSeparator();
    dummy = formatMenu->addAction(tr("Change Display for Type \"%1\":").arg(type));
    dummy->setEnabled(false);

    QAction *clearTypeFormatAction = formatMenu->addAction(spacer + tr("Automatic"));
    clearTypeFormatAction->setCheckable(true);
    clearTypeFormatAction->setChecked(typeFormat == AutomaticFormat);
    connect(clearTypeFormatAction, SIGNAL(triggered()), SLOT(onClearTypeFormat()));

    for (int i = 0; i != alternativeFormats.size(); ++i) {
        const QString display = spacer + alternativeFormats.at(i).display;
        QAction *act = new QAction(display, formatMenu);
        const int format = alternativeFormats.at(i).format;
        act->setData(format);
        act->setCheckable(true);
        act->setChecked(format == typeFormat);
        act->setProperty(CurrentIndex, QVariant::fromValue(mi));
        formatMenu->addAction(act);
        connect(act, SIGNAL(triggered()), SLOT(onTypeFormatChange()));
    }
}

void WatchTreeView::onClearTypeFormat()
{
    const QModelIndexList active = activeRows();
    foreach (const QModelIndex &idx, active)
        setModelData(LocalsTypeFormatRole, AutomaticFormat, idx);
}

void WatchTreeView::onClearIndividualFormat()
{
    const QModelIndexList active = activeRows();
    foreach (const QModelIndex &idx, active)
        setModelData(LocalsIndividualFormatRole, AutomaticFormat, idx);
}

void WatchTreeView::onShowUnprintable()
{
    QAction *act = qobject_cast<QAction *>(sender());
    QTC_ASSERT(act, return);
    DebuggerEngine *engine = currentEngine();
    WatchHandler *handler = engine->watchHandler();
    handler->setUnprintableBase(act->data().toInt());
}

void WatchTreeView::onTypeFormatChange()
{
    QAction *act = qobject_cast<QAction *>(sender());
    QTC_ASSERT(act, return);
    QModelIndex idx = act->property(CurrentIndex).value<QModelIndex>();
    setModelData(LocalsTypeFormatRole, act->data(), idx);
}

void WatchTreeView::onIndividualFormatChange()
{
    QAction *act = qobject_cast<QAction *>(sender());
    QTC_ASSERT(act, return);
    QModelIndex idx = act->property(CurrentIndex).value<QModelIndex>();
    setModelData(LocalsIndividualFormatRole, act->data(), idx);
}

void WatchTreeView::contextMenuEvent(QContextMenuEvent *ev)
{
    DebuggerEngine *engine = currentEngine();
    WatchHandler *handler = engine->watchHandler();

    const QModelIndex idx = indexAt(ev->pos());
    const QModelIndex mi0 = idx.sibling(idx.row(), 0);
    const QModelIndex mi1 = idx.sibling(idx.row(), 1);
    const quint64 address = addressOf(mi0);
    const uint size = sizeOf(mi0);
    const quint64 pointerAddress = pointerAddressOf(mi0);
    const QString exp = mi0.data(LocalsExpressionRole).toString();
    const QString name = mi0.data(LocalsNameRole).toString();

    // Offer to open address pointed to or variable address.
    const bool createPointerActions = pointerAddress && pointerAddress != address;

    const bool actionsEnabled = engine->debuggerActionsEnabled();
    const bool canHandleWatches = engine->hasCapability(AddWatcherCapability);
    const DebuggerState state = engine->state();
    const bool canInsertWatches = state == InferiorStopOk
        || state == DebuggerNotReady
        || state == InferiorUnrunnable
        || (state == InferiorRunOk && engine->hasCapability(AddWatcherWhileRunningCapability));

    QAction actSetWatchpointAtObjectAddress(0);
    QAction actSetWatchpointAtPointerAddress(0);
    actSetWatchpointAtPointerAddress.setText(tr("Add Data Breakpoint at Pointer's Address"));
    actSetWatchpointAtPointerAddress.setEnabled(false);
    const bool canSetWatchpoint = engine->hasCapability(WatchpointByAddressCapability);
    if (canSetWatchpoint && address) {
        actSetWatchpointAtObjectAddress
            .setText(tr("Add Data Breakpoint at Object's Address (0x%1)").arg(address, 0, 16));
        actSetWatchpointAtObjectAddress
            .setChecked(mi0.data(LocalsIsWatchpointAtObjectAddressRole).toBool());
        if (createPointerActions) {
            actSetWatchpointAtPointerAddress
                .setText(tr("Add Data Breakpoint at Pointer's Address (0x%1)")
                    .arg(pointerAddress, 0, 16));
            actSetWatchpointAtPointerAddress
                .setChecked(mi0.data(LocalsIsWatchpointAtPointerAddressRole).toBool());
            actSetWatchpointAtPointerAddress.setEnabled(true);
        }
    } else {
        actSetWatchpointAtObjectAddress.setText(tr("Add Data Breakpoint"));
        actSetWatchpointAtObjectAddress.setEnabled(false);
    }
    actSetWatchpointAtObjectAddress.setToolTip(
        tr("Setting a data breakpoint on an address will cause the program "
           "to stop when the data at the address is modified."));

    QAction actSetWatchpointAtExpression(0);
    const bool canSetWatchpointAtExpression = engine->hasCapability(WatchpointByExpressionCapability);
    if (name.isEmpty() || !canSetWatchpointAtExpression) {
        actSetWatchpointAtExpression.setText(tr("Add Data Breakpoint at Expression"));
        actSetWatchpointAtExpression.setEnabled(false);
    } else {
        actSetWatchpointAtExpression.setText(tr("Add Data Breakpoint at Expression \"%1\"").arg(name));
    }
    actSetWatchpointAtExpression.setToolTip(
        tr("Setting a data breakpoint on an expression will cause the program "
           "to stop when the data at the address given by the expression "
           "is modified."));

    QAction actInsertNewWatchItem(tr("Add New Expression Evaluator..."), 0);
    actInsertNewWatchItem.setEnabled(canHandleWatches && canInsertWatches);

    QAction actSelectWidgetToWatch(tr("Select Widget to Add into Expression Evaluator"), 0);
    actSelectWidgetToWatch.setEnabled(canHandleWatches && canInsertWatches
           && engine->hasCapability(WatchWidgetsCapability));

    QAction actWatchExpression(addWatchActionText(exp), 0);
    actWatchExpression.setEnabled(
        canHandleWatches && !exp.isEmpty() && m_type == LocalsType);

    // Can remove watch if engine can handle it or session engine.
    QModelIndex p = mi0;
    while (true) {
        QModelIndex pp = p.parent();
        if (!pp.isValid() || !pp.parent().isValid())
            break;
        p = pp;
    }
    QString removeExp = p.data(LocalsExpressionRole).toString();
    QAction actRemoveWatchExpression(removeWatchActionText(removeExp), 0);
    actRemoveWatchExpression.setEnabled(
        (canHandleWatches || state == DebuggerNotReady)
                && !exp.isEmpty() && m_type == WatchersType);

    QMenu formatMenu(tr("Change Local Display Format..."));
    if (mi0.isValid())
        fillFormatMenu(&formatMenu, mi0);

    QMenu memoryMenu(tr("Open Memory Editor..."));
    QAction actOpenMemoryEditAtObjectAddress(0);
    QAction actOpenMemoryEditAtPointerAddress(0);
    QAction actOpenMemoryEditor(0);
    QAction actOpenMemoryEditorStackLayout(0);
    QAction actOpenMemoryViewAtObjectAddress(0);
    QAction actOpenMemoryViewAtPointerAddress(0);
    if (engine->hasCapability(ShowMemoryCapability)) {
        actOpenMemoryEditor.setText(tr("Open Memory Editor..."));
        if (address) {
            actOpenMemoryEditAtObjectAddress.setText(
                tr("Open Memory Editor at Object's Address (0x%1)")
                    .arg(address, 0, 16));
            actOpenMemoryViewAtObjectAddress.setText(
                    tr("Open Memory View at Object's Address (0x%1)")
                        .arg(address, 0, 16));
        } else {
            actOpenMemoryEditAtObjectAddress.setText(
                tr("Open Memory Editor at Object's Address"));
            actOpenMemoryEditAtObjectAddress.setEnabled(false);
            actOpenMemoryViewAtObjectAddress.setText(
                    tr("Open Memory View at Object's Address"));
            actOpenMemoryViewAtObjectAddress.setEnabled(false);
        }
        if (createPointerActions) {
            actOpenMemoryEditAtPointerAddress.setText(
                tr("Open Memory Editor at Pointer's Address (0x%1)")
                    .arg(pointerAddress, 0, 16));
            actOpenMemoryViewAtPointerAddress.setText(
                tr("Open Memory View at Pointer's Address (0x%1)")
                    .arg(pointerAddress, 0, 16));
        } else {
            actOpenMemoryEditAtPointerAddress.setText(
                tr("Open Memory Editor at Pointer's Address"));
            actOpenMemoryEditAtPointerAddress.setEnabled(false);
            actOpenMemoryViewAtPointerAddress.setText(
                tr("Open Memory View at Pointer's Address"));
            actOpenMemoryViewAtPointerAddress.setEnabled(false);
        }
        actOpenMemoryEditorStackLayout.setText(
            tr("Open Memory Editor Showing Stack Layout"));
        actOpenMemoryEditorStackLayout.setEnabled(m_type == LocalsType);
        memoryMenu.addAction(&actOpenMemoryViewAtObjectAddress);
        memoryMenu.addAction(&actOpenMemoryViewAtPointerAddress);
        memoryMenu.addAction(&actOpenMemoryEditAtObjectAddress);
        memoryMenu.addAction(&actOpenMemoryEditAtPointerAddress);
        memoryMenu.addAction(&actOpenMemoryEditorStackLayout);
        memoryMenu.addAction(&actOpenMemoryEditor);
    } else {
        memoryMenu.setEnabled(false);
    }

    QMenu breakpointMenu;
    breakpointMenu.setTitle(tr("Add Data Breakpoint..."));
    breakpointMenu.addAction(&actSetWatchpointAtObjectAddress);
    breakpointMenu.addAction(&actSetWatchpointAtPointerAddress);
    breakpointMenu.addAction(&actSetWatchpointAtExpression);

    QAction actCopy(tr("Copy View Contents to Clipboard"), 0);
    QAction actCopyValue(tr("Copy Value to Clipboard"), 0);
    actCopyValue.setEnabled(idx.isValid());

    QAction actShowInEditor(tr("Open View Contents in Editor"), 0);
    actShowInEditor.setEnabled(actionsEnabled);
    QAction actCloseEditorToolTips(tr("Close Editor Tooltips"), 0);
    actCloseEditorToolTips.setEnabled(DebuggerToolTipManager::hasToolTips());

    QMenu menu;
    menu.addAction(&actInsertNewWatchItem);
    menu.addAction(&actWatchExpression);
    menu.addAction(&actRemoveWatchExpression);
    menu.addAction(&actSelectWidgetToWatch);
    menu.addSeparator();

    menu.addMenu(&formatMenu);
    menu.addMenu(&memoryMenu);
    menu.addMenu(&breakpointMenu);
    menu.addSeparator();

    menu.addAction(&actCloseEditorToolTips);
    menu.addAction(&actCopy);
    menu.addAction(&actCopyValue);
    menu.addAction(&actShowInEditor);
    menu.addSeparator();

    menu.addAction(debuggerCore()->action(UseDebuggingHelpers));
    menu.addAction(debuggerCore()->action(UseToolTipsInLocalsView));
    menu.addAction(debuggerCore()->action(AutoDerefPointers));
    menu.addAction(debuggerCore()->action(SortStructMembers));
    menu.addAction(debuggerCore()->action(UseDynamicType));
    menu.addAction(debuggerCore()->action(SettingsDialog));

    menu.addSeparator();
    menu.addAction(debuggerCore()->action(SettingsDialog));

    QAction *act = menu.exec(ev->globalPos());

    if (!act) {
        ;
    } else if (act == &actInsertNewWatchItem) {
        inputNewExpression();
    } else if (act == &actOpenMemoryEditAtObjectAddress) {
        addVariableMemoryView(currentEngine(), false, mi0, false, ev->globalPos(), this);
    } else if (act == &actOpenMemoryEditAtPointerAddress) {
        addVariableMemoryView(currentEngine(), false, mi0, true, ev->globalPos(), this);
    } else if (act == &actOpenMemoryEditor) {
        AddressDialog dialog;
        if (address)
            dialog.setAddress(address);
        if (dialog.exec() == QDialog::Accepted)
            currentEngine()->openMemoryView(dialog.address(), false, MemoryMarkupList(), QPoint());
    } else if (act == &actOpenMemoryViewAtObjectAddress) {
        addVariableMemoryView(currentEngine(), true, mi0, false, ev->globalPos(), this);
    } else if (act == &actOpenMemoryViewAtPointerAddress) {
        addVariableMemoryView(currentEngine(), true, mi0, true, ev->globalPos(), this);
    } else if (act == &actOpenMemoryEditorStackLayout) {
        addStackLayoutMemoryView(currentEngine(), false, model(), ev->globalPos(), this);
    } else if (act == &actSetWatchpointAtObjectAddress) {
        breakHandler()->setWatchpointAtAddress(address, size);
    } else if (act == &actSetWatchpointAtPointerAddress) {
        breakHandler()->setWatchpointAtAddress(pointerAddress, sizeof(void *)); // FIXME: an approximation..
    } else if (act == &actSetWatchpointAtExpression) {
        breakHandler()->setWatchpointAtExpression(name);
    } else if (act == &actSelectWidgetToWatch) {
        grabMouse(Qt::CrossCursor);
        m_grabbing = true;
    } else if (act == &actWatchExpression) {
        watchExpression(exp, name);
    } else if (act == &actRemoveWatchExpression) {
        handler->removeData(p.data(LocalsINameRole).toByteArray());
    } else if (act == &actCopy) {
        copyToClipboard(DebuggerToolTipManager::treeModelClipboardContents(model()));
    } else if (act == &actCopyValue) {
        copyToClipboard(mi1.data().toString());
    } else if (act == &actShowInEditor) {
        QString contents = handler->editorContents();
        debuggerCore()->openTextEditor(tr("Locals & Expressions"), contents);
    } else if (act == &actCloseEditorToolTips) {
        DebuggerToolTipManager::closeAllToolTips();
    }
}

bool WatchTreeView::event(QEvent *ev)
{
    if (m_grabbing && ev->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mev = static_cast<QMouseEvent *>(ev);
        m_grabbing = false;
        releaseMouse();
        currentEngine()->watchPoint(mapToGlobal(mev->pos()));
    }
    return BaseTreeView::event(ev);
}

void WatchTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit currentIndexChanged(current);
    BaseTreeView::currentChanged(current, previous);
}

void WatchTreeView::editItem(const QModelIndex &idx)
{
    Q_UNUSED(idx) // FIXME
}

void WatchTreeView::setModel(QAbstractItemModel *model)
{
    BaseTreeView::setModel(model);
    setRootIndex(model->index(m_type, 0, QModelIndex()));
    setRootIsDecorated(true);
    if (header()) {
        header()->setDefaultAlignment(Qt::AlignLeft);
        if (m_type != LocalsType && m_type != InspectType)
            header()->hide();
    }

    connect(model, SIGNAL(layoutChanged()), SLOT(resetHelper()));
    connect(model, SIGNAL(currentIndexRequested(QModelIndex)),
            SLOT(setCurrentIndex(QModelIndex)));
    connect(model, SIGNAL(itemIsExpanded(QModelIndex)),
            SLOT(handleItemIsExpanded(QModelIndex)));
#if USE_WATCH_MODEL_TEST
    (void) new ModelTest(&m_filter, this);
#endif
}

void WatchTreeView::rowActivated(const QModelIndex &index)
{
    currentEngine()->watchDataSelected(index.data(LocalsINameRole).toByteArray());
}

void WatchTreeView::handleItemIsExpanded(const QModelIndex &idx)
{
    bool on = idx.data(LocalsExpandedRole).toBool();
    QTC_ASSERT(on, return);
    if (!isExpanded(idx))
        expand(idx);
}

void WatchTreeView::reexpand(QTreeView *view, const QModelIndex &idx)
{
    if (idx.data(LocalsExpandedRole).toBool()) {
        //qDebug() << "EXPANDING " << view->model()->data(idx, LocalsINameRole);
        if (!view->isExpanded(idx)) {
            view->expand(idx);
            for (int i = 0, n = view->model()->rowCount(idx); i != n; ++i) {
                QModelIndex idx1 = view->model()->index(i, 0, idx);
                reexpand(view, idx1);
            }
        }
    } else {
        //qDebug() << "COLLAPSING " << view->model()->data(idx, LocalsINameRole);
        if (view->isExpanded(idx))
            view->collapse(idx);
    }
}

void WatchTreeView::resetHelper()
{
    QModelIndex idx = model()->index(m_type, 0);
    reexpand(this, idx);
    expand(idx);
}

void WatchTreeView::reset()
{
    BaseTreeView::reset();
    setRootIndex(model()->index(m_type, 0));
    resetHelper();
}

void WatchTreeView::doItemsLayout()
{
    if (m_sliderPosition == 0)
        m_sliderPosition = verticalScrollBar()->sliderPosition();
    Utils::BaseTreeView::doItemsLayout();
    if (m_sliderPosition)
        QTimer::singleShot(0, this, SLOT(adjustSlider()));
}

void WatchTreeView::adjustSlider()
{
    if (m_sliderPosition) {
        verticalScrollBar()->setSliderPosition(m_sliderPosition);
        m_sliderPosition = 0;
    }
}

void WatchTreeView::watchExpression(const QString &exp)
{
    watchExpression(exp, QString());
}

void WatchTreeView::watchExpression(const QString &exp, const QString &name)
{
    currentEngine()->watchHandler()->watchExpression(exp, name);
}

void WatchTreeView::setModelData
    (int role, const QVariant &value, const QModelIndex &index)
{
    QTC_ASSERT(model(), return);
    model()->setData(index, value, role);
}


// FIXME: Move to Utils?
class InputDialog : public QDialog
{
public:
    InputDialog()
    {
        m_label = new QLabel(this);
        m_hint = new QLabel(this);
        m_lineEdit = new Utils::FancyLineEdit(this);
        m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(m_label, Qt::AlignLeft);
        layout->addWidget(m_hint, Qt::AlignLeft);
        layout->addWidget(m_lineEdit);
        layout->addSpacing(10);
        layout->addWidget(m_buttons);
        setLayout(layout);

        connect(m_buttons, SIGNAL(accepted()), m_lineEdit, SLOT(onEditingFinished()));
        connect(m_buttons, SIGNAL(accepted()), SLOT(accept()));
        connect(m_buttons, SIGNAL(rejected()), SLOT(reject()));
        connect(m_hint, SIGNAL(linkActivated(QString)),
            Core::HelpManager::instance(), SLOT(handleHelpRequest(QString)));
    }

    void setLabelText(const QString &text)
    {
        m_label->setText(text);
    }

    void setHintText(const QString &text)
    {
        m_hint->setText(QString::fromLatin1("<html>%1</html>").arg(text));
    }

    void setHistoryCompleter(const QString &key)
    {
        m_lineEdit->setHistoryCompleter(key);
        m_lineEdit->setText(QString()); // Undo "convenient" population with history item.
    }

    QString text() const
    {
        return m_lineEdit->text();
    }

public:
    QLabel *m_label;
    QLabel *m_hint;
    Utils::FancyLineEdit *m_lineEdit;
    QDialogButtonBox *m_buttons;
};

void WatchTreeView::inputNewExpression()
{
    InputDialog dlg;
    dlg.setWindowTitle(tr("New Evaluated Expression"));
    dlg.setLabelText(tr("Enter an expression to evaluate."));
    dlg.setHintText(tr("Note: Evaluators will be re-evaluated after each step. "
       "For details check the <a href=\""
        "qthelp://org.qt-project.qtcreator/doc/creator-debug-mode.html#locals-and-expressions"
        "\">documentation</a>."));
    dlg.setHistoryCompleter(QLatin1String("WatchItems"));
    if (dlg.exec() == QDialog::Accepted) {
        QString exp = dlg.text();
        if (!exp.isEmpty())
            watchExpression(exp, exp);
    }
}

} // namespace Internal
} // namespace Debugger
