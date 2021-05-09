#ifndef MYCLASS_H
#define MYCLASS_H

#include <QtWidgets/QWidget>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <vector>

class QTableView;
class QSqlQueryModel;
class QLabel;
class QLineEdit;
class QPushButton;

class CArrayModel : public QAbstractTableModel
{
public:
    CArrayModel(QObject *parent = 0);
    //设置总数据
    void SetArrayData(const std::map<int, QString> &map);

    //获得总数据
    std::map<int, QString> GetArrayData();

    //设置页数据
    void SetCurPage(int iPage);
    //获得当前页
    int GetCurPage();
    //获得总页数
    int GetPageCount();
    //设置每页数据条数
    void SetPageSize(int iPageSize);
    //获得每页数据条数
    int GetPageSize();

    //总行数
    int RowCount() const;
public:
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void refrushModel();
private:
    QString currencyAt(int offset) const;
    std::map<int, QString> m_mpData;    //总数据
    std::map<int, QString> m_mpPageData;//每页数据
    int m_iPageSize;                    //每页数据条数
    int m_iCurPage;                     //当前页
};

//只读委托(给索引列使用)
class ReadOnlyDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ReadOnlyDelegate(QObject *parent = 0): QItemDelegate(parent) { }
    QWidget *createEditor(QWidget*parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const
    {
        return NULL;
    }
};

//值列
class ValueDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ValueDelegate(QObject *parent = 0): QItemDelegate(parent) { }
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,  const QModelIndex &index) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,  const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,  const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

//自定义窗口类
class MyMainWindow : public QWidget
{
    Q_OBJECT

public:
    MyMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~MyMainWindow();

    void keyPressEvent(QKeyEvent *event);

    //槽函数
    private slots:
        void OnFirstButtonClick();           //首页按钮按下
        void OnLastButtonClick();           //末页按钮按下
        void OnPrevButtonClick();            //前一页按钮按下
        void OnNextButtonClick();            //后一页按钮按下
        void OnSwitchPageButtonClick();      //转到页按钮按下
        void OnIndexButtonClick();           //转到索引按钮按下
        void OnSetPageSizeButtonClick();     //设置每页显示行数按钮按下

private :
    void MyCreateWindow();                   //创建窗口
    void SetTableView();                     //设置表格
    void UpdateStatus();                     //刷新状态

private:
    CArrayModel       *m_pDataModel;        //数据模型
    QTableView        *tableView;           //数据表
    QLineEdit         *switchPageLineEdit;  //转到页输入框
    QPushButton       *m_pFirstPageBtn;     //首页按钮
    QPushButton       *m_pLastPageBtn;      //末页按钮
    QPushButton       *prevButton;          //前一页按钮
    QPushButton       *nextButton;          //下一页按钮
    QPushButton       *switchPageButton;    //转到页按钮
    QLineEdit         *m_pIndexEdit;        //索引输入框
    QPushButton       *m_pGoIndexBtn;       //按索引跳转按钮
    QLabel            *totalPageLabel;      //总数页文本
    QLabel            *currentPageLabel;    //当前页文本
    QLineEdit         *m_pPerPageCountEdit; //每页显示行数
    QPushButton       *m_pSetPerPageCountBtn;//设置每页显示行数按钮
    enum      {PageRecordCount = 10};       //默认每页显示记录数
};
#endif

