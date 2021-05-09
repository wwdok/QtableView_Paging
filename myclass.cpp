#include "myclass.h"

#include <QtGui>
#include <QRegExp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <Qstring>
#include <QScrollBar>


CArrayModel::CArrayModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

//设置Model数据
void CArrayModel::SetArrayData(const std::map<int, QString> &map)
{
    m_mpData = map;
}

//获得总数据
std::map<int, QString> CArrayModel::GetArrayData()
{
    return m_mpData;
}

//总行数
int CArrayModel::RowCount() const
{
    return m_mpData.size();
}

//设置页数据
void CArrayModel::SetCurPage(int iPage)
{
    //当前页必须小于总页数
    if (iPage < GetPageCount())
    {
        m_iCurPage = iPage;

        //查询起始索引
        int iStart = m_iPageSize * m_iCurPage;
        //查询结束索引
        int iend = 0;
        //如果本页可以填满
        if (iStart + m_iPageSize < RowCount())
        {
            iend = iStart + m_iPageSize;
        }
        //如果本页不可以填满
        else
        {
            iend = RowCount() - 1;
        }

        //填充当前页数据
        m_mpPageData.clear();
        for (int i = iStart; i <= iend; ++i)
        {
            auto it = m_mpData.find(i);
            if (it == m_mpData.end())
            {
                return;
            }

            m_mpPageData.insert(std::pair<int, QString>(i, it->second));
        }
    }

    return;
}

//获得当前页
int CArrayModel::GetCurPage()
{
    return m_iCurPage;
}

//获得总页数
int CArrayModel::GetPageCount()
{
    return (RowCount() % m_iPageSize == 0)
        ? (RowCount() / m_iPageSize)
        : (RowCount() / m_iPageSize + 1);
}

//设置每页数据条数
void CArrayModel::SetPageSize(int iPageSize)
{
    if (iPageSize <= 0)
    {
        return;
    }

    m_iPageSize = iPageSize;
    SetCurPage(0);

    //刷新Model，否则TableView不会刷新显示
    refrushModel();
}


//获得每页数据条数
int CArrayModel::GetPageSize()
{
    return m_iPageSize;
}

//行数
int CArrayModel::rowCount(const QModelIndex & parent) const
{
    return m_iPageSize;
}

//列数
int CArrayModel::columnCount(const QModelIndex & parent) const
{
    //仅仅有两列数据
    return 2;
}


void CArrayModel::refrushModel()
{
    beginResetModel();
    endResetModel();
}

QVariant CArrayModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        return int(Qt::AlignCenter | Qt::AlignVCenter);
    }
    else if (role == Qt::DisplayRole)
    {
        if ( 0 == index.column())
        {
            //如果处于最后一页，索引没必要全部列出，只列出范围内的
            if (index.row() + m_iCurPage * m_iPageSize > this->RowCount())
            {
                return QVariant();
            }

            return index.row() + m_iCurPage * m_iPageSize;
        }
        else if (1 == index.column())
        {
            auto it = m_mpPageData.find(index.row() + m_iCurPage * m_iPageSize);
            if (it != m_mpPageData.end())
            {
                return it->second;
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags CArrayModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool CArrayModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        QVariant oldData = data(index, Qt::EditRole);
        QString strold = oldData.toString();
        QString strnew = value.toString();
        //相同则不编辑
        if (strnew.compare(strold) == 0)
        {
            return true;
        }

        //计算实际数据的下标
        int dataindex = index.row() + m_iCurPage * m_iPageSize;

        //改变总数据集
        auto it  = m_mpData.find(dataindex);
        if (it != m_mpData.end())
        {
            it->second = strnew;
        }

        //改变当页数据集
        auto itcur = m_mpPageData.find(dataindex);
        if (itcur != m_mpPageData.end())
        {
            itcur->second = strnew;
        }

        return true;
    }
    return false;
}

QVariant CArrayModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (0 == section)
    {
        return QStringLiteral("索引");
    }
    else if (1 == section)
    {
        return QStringLiteral("值");
    }

    return QVariant();
}


QString CArrayModel::currencyAt(int offset) const
{
    auto it = m_mpData.find(offset);
    if (it != m_mpData.end())
    {
        return it->second;
    }

    return QString();
}

//样式定制
void ValueDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
    {
        // 如果当前项具有焦点，它就绘制一个焦点矩形(不重要)
        drawFocus(painter, option, option.rect);
    }
    else
    {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget *ValueDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,  const QModelIndex &index) const
{
    QLineEdit *editor = new QLineEdit(parent);
    return editor;
}

void ValueDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(text);
}

void ValueDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,  const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QString text = lineEdit->text();
    model->setData(index, text, Qt::EditRole);
}

void ValueDelegate::updateEditorGeometry(QWidget *editor,  const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

MyMainWindow::MyMainWindow(QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
    //创建窗口
    MyCreateWindow();
    //设置表格
    SetTableView();

    //翻页按钮
    connect((const QObject *)m_pFirstPageBtn, SIGNAL(clicked()),this, SLOT(OnFirstButtonClick()));
    connect((const QObject *)m_pLastPageBtn, SIGNAL(clicked()),this, SLOT(OnLastButtonClick()));
    connect((const QObject *)prevButton, SIGNAL(clicked()),this, SLOT(OnPrevButtonClick()));
    connect((const QObject *)nextButton, SIGNAL(clicked()),this, SLOT(OnNextButtonClick()));
    connect((const QObject *)m_pGoIndexBtn, SIGNAL(clicked()),this, SLOT(OnIndexButtonClick()));
    connect(m_pIndexEdit, SIGNAL(returnPressed()), this, SLOT(OnIndexButtonClick()));
    //跳转按钮和跳转到页回车相应
    connect((const QObject *)switchPageButton,SIGNAL(clicked()), this, SLOT(OnSwitchPageButtonClick()));
    connect(switchPageLineEdit, SIGNAL(returnPressed()), this, SLOT(OnSwitchPageButtonClick()));
    //设置每页显示行数按钮
    connect(m_pPerPageCountEdit, SIGNAL(returnPressed()), this, SLOT(OnSetPageSizeButtonClick()));
    connect((const QObject *)m_pSetPerPageCountBtn, SIGNAL(clicked()), this, SLOT(OnSetPageSizeButtonClick()));
}

MyMainWindow::~MyMainWindow()
{
}

//对键盘事件处理
void MyMainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        //进行界面退出，重写Esc键，否则重写reject()方法
    case Qt::Key_Escape:
        this->close();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

//创建窗口
void MyMainWindow::MyCreateWindow()
{
    //设置窗口属性
    setMinimumSize(600,400);
    setWindowTitle(QStringLiteral("查看数组"));
    m_pFirstPageBtn = new QPushButton(QStringLiteral("首页"));
    prevButton = new QPushButton(QStringLiteral("上一页"));
    nextButton = new QPushButton(QStringLiteral("下一页"));
    m_pLastPageBtn = new QPushButton(QStringLiteral("末页"));
    //一直按下持续执行功能打开，提升浏览体验
    prevButton->setAutoRepeat(true);
    nextButton->setAutoRepeat(true);

    QLabel *switchPage = new QLabel(QStringLiteral("    第"));
    switchPageLineEdit = new QLineEdit;
    switchPageLineEdit->setFixedWidth(40);
    QLabel *page = new QLabel(QStringLiteral("页"));
    switchPageButton = new QPushButton(QStringLiteral("转到"));
    switchPageButton->setFixedWidth(40);

    QLabel *pIndexLabel = new QLabel(QStringLiteral("    索引"));
    m_pIndexEdit = new QLineEdit;
    m_pIndexEdit->setFixedWidth(40);
    m_pGoIndexBtn = new QPushButton(QStringLiteral("转到"));
    m_pGoIndexBtn->setFixedWidth(40);

    //操作布局
    QHBoxLayout *operatorLayout = new QHBoxLayout;
    operatorLayout->addWidget(m_pFirstPageBtn);
    operatorLayout->addWidget(prevButton);
    operatorLayout->addWidget(nextButton);
    operatorLayout->addWidget(m_pLastPageBtn);
    operatorLayout->addWidget(switchPage);
    operatorLayout->addWidget(switchPageLineEdit);
    operatorLayout->addWidget(page);
    operatorLayout->addWidget(switchPageButton);
    operatorLayout->addWidget(pIndexLabel);
    operatorLayout->addWidget(m_pIndexEdit);
    operatorLayout->addWidget(m_pGoIndexBtn);
    operatorLayout->addWidget(new QSplitter());

    //状态
    totalPageLabel = new QLabel;
    totalPageLabel->setFixedWidth(90);
    currentPageLabel = new QLabel;
    currentPageLabel->setFixedWidth(90);
    QLabel *pPerPageCountDisLabel = new QLabel(QStringLiteral("每页显示"));
    m_pPerPageCountEdit = new QLineEdit;
    m_pPerPageCountEdit->setFixedWidth(40);
    QLabel *pPerPageCountLineLabel = new QLabel(QStringLiteral("行"));
    m_pSetPerPageCountBtn = new QPushButton(QStringLiteral("设置"));
    m_pSetPerPageCountBtn->setFixedWidth(40);

    //状态布局
    QHBoxLayout *statusLayout = new QHBoxLayout;
    statusLayout->addWidget(totalPageLabel);
    statusLayout->addWidget(currentPageLabel);
    statusLayout->addWidget(pPerPageCountDisLabel);
    statusLayout->addWidget(m_pPerPageCountEdit);
    statusLayout->addWidget(pPerPageCountLineLabel);
    statusLayout->addWidget(m_pSetPerPageCountBtn);
    statusLayout->addWidget(new QSplitter());

    //设置表格属性
    tableView = new QTableView;
    tableView->verticalHeader()->hide();//隐藏垂直索引
    tableView->setEditTriggers( QAbstractItemView::AllEditTriggers);
    tableView->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
    //tableView->setSelectionBehavior(QAbstractItemView::SelectRows); //整行选中的方式
    tableView->setAlternatingRowColors(true);
    tableView->setStyleSheet("QTableView{background-color: rgb(186, 202, 224);"
        "alternate-background-color: rgb(212, 212, 212);}");

    tableView->horizontalHeader()->resizeSection(0,70); //设置表头第一列的宽度为150
    tableView->horizontalHeader()->setFixedHeight(25); //设置表头的高度
    tableView->setStyleSheet("selection-background-color:skyblue;"); //设置选中背景色
    tableView->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色

    //窗口布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(operatorLayout);
    mainLayout->addWidget(tableView);
    mainLayout->addLayout(statusLayout);
}

// 设置表格
void MyMainWindow::SetTableView()
{
    //声明查询模型
    m_pDataModel = new CArrayModel;

    std::map<int, QString> map;

    for (int i = 0; i < 100; ++i)
    {
        QString str = QString("%1").arg(QString::number(i));
        map.insert(std::pair<int, QString>(i, str));
    }

    m_pDataModel->SetArrayData(map);

    //设置每页数据条数
    m_pDataModel->SetPageSize(PageRecordCount);

    //设置模型
    tableView->setModel(m_pDataModel);

    //设置委托
    tableView->setItemDelegateForColumn(0, new ReadOnlyDelegate(this));
    tableView->setItemDelegateForColumn(1, new ValueDelegate(this));

    //刷新状态
    UpdateStatus();

    return;
}

//刷新界面状态
void MyMainWindow::UpdateStatus()
{
    //刷新表格
    tableView->reset();

    //总页数
    QString szPageCountText = QString(QStringLiteral("总共%1页")).arg(QString::number(m_pDataModel->GetPageCount()));
    totalPageLabel->setText(szPageCountText);

    //设置当前页文本
    int iCurPage = m_pDataModel->GetCurPage() + 1;
    QString szCurrentText = QString(QStringLiteral("当前第%1页")).arg(QString::number(iCurPage));
    currentPageLabel->setText(szCurrentText);

    //每页显示行数
    QString strPerPageCount = QString(QStringLiteral("%1")).arg(QString::number(m_pDataModel->GetPageSize()));
    m_pPerPageCountEdit->setText(strPerPageCount);

    //当前第一页，且总共只有一页
    if (1 == iCurPage && 1 == m_pDataModel->GetPageCount())
    {
        m_pFirstPageBtn->setEnabled(false);
        m_pLastPageBtn->setEnabled(false);
        prevButton->setEnabled(false);
        nextButton->setEnabled(false);

    }
    //当前第一页，且总页数大于1页
    else if(1 == iCurPage && m_pDataModel->GetPageCount() > 1)
    {
        m_pFirstPageBtn->setEnabled(false);
        m_pLastPageBtn->setEnabled(true);
        prevButton->setEnabled(false);
        nextButton->setEnabled(true);
    }
    //当前是最后一页
    else if(iCurPage == m_pDataModel->GetPageCount())
    {
        m_pFirstPageBtn->setEnabled(true);
        m_pLastPageBtn->setEnabled(false);
        prevButton->setEnabled(true);
        nextButton->setEnabled(false);
    }
    //中间页
    else
    {
        m_pFirstPageBtn->setEnabled(true);
        m_pLastPageBtn->setEnabled(true);
        prevButton->setEnabled(true);
        nextButton->setEnabled(true);
    }

    return;
}

//首页按钮按下
void MyMainWindow::OnFirstButtonClick()
{
    m_pDataModel->SetCurPage(0);
    UpdateStatus();
}

//末页按钮按下
void MyMainWindow::OnLastButtonClick()
{
    m_pDataModel->SetCurPage(m_pDataModel->GetPageCount() - 1);
    UpdateStatus();
}

//前一页按钮按下
void MyMainWindow::OnPrevButtonClick()
{
    m_pDataModel->SetCurPage(m_pDataModel->GetCurPage() - 1);
    UpdateStatus();
}

//后一页按钮按下
void MyMainWindow::OnNextButtonClick()
{
    m_pDataModel->SetCurPage(m_pDataModel->GetCurPage() + 1);
    UpdateStatus();
}

//转到索引按钮按下
void MyMainWindow::OnIndexButtonClick()
{
    //得到输入字符串
    QString szText = m_pIndexEdit->text();
    //数字正则表达式
    QRegExp regExp("-?[0-9]*");
    //判断是否为数字
    if(!regExp.exactMatch(szText))
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入数字"));

        return;
    }
    //是否为空
    if(szText.isEmpty())
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入跳转索引"));

        return;
    }
    //得到页数
    int Index = szText.toInt();
    //判断是否有指定页
    if(Index >= m_pDataModel->RowCount() || Index < 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("索引超出范围，请重新输入"));

        return;
    }
    //记录查询
    m_pDataModel->SetCurPage(Index / m_pDataModel->GetPageSize());

    //刷新状态
    UpdateStatus();

    return;
}

//转到页按钮按下
void MyMainWindow::OnSwitchPageButtonClick()
{
    //得到输入字符串
    QString szText = switchPageLineEdit->text();
    //数字正则表达式
    QRegExp regExp("-?[0-9]*");
    //判断是否为数字
    if(!regExp.exactMatch(szText))
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入数字"));

        return;
    }
    //是否为空
    if(szText.isEmpty())
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入跳转页面"));

        return;
    }
    //得到页数
    int pageIndex = szText.toInt();
    //判断是否有指定页
    if(pageIndex > m_pDataModel->GetPageCount() || pageIndex < 1)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("页面超出范围，请重新输入"));

        return;
    }
    //记录查询
    m_pDataModel->SetCurPage(pageIndex - 1);
    //刷新状态
    UpdateStatus();

    return;
}

//设置每页显示行数
void MyMainWindow::OnSetPageSizeButtonClick()
{
    //得到输入字符串
    QString szText = m_pPerPageCountEdit->text();
    //数字正则表达式
    QRegExp regExp("-?[0-9]*");
    //判断是否为数字
    if(!regExp.exactMatch(szText))
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入数字"));

        return;
    }
    //是否为空
    if(szText.isEmpty())
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入设置的行数"));

        return;
    }
    //得到行数
    int Index = szText.toInt();
    //判断范围是否合理
    if(Index > m_pDataModel->RowCount() || Index <= 0)
    {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("设置值超出范围，请重新输入"));

        return;
    }

    //设置每页容量
    m_pDataModel->SetPageSize(Index);

    //刷新状态
    UpdateStatus();

    return;
}
