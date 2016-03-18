#include "dfilemanagerwindow.h"
#include <QStatusBar>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QResizeEvent>

#include "utils/xutil.h"

#include "dtitlebar.h"
#include "dleftsidebar.h"
#include "dtoolbar.h"
#include "dfileview.h"
#include "ddetailview.h"
#include "../app/global.h"

const int DFileManagerWindow::MinimumWidth = 540;

DFileManagerWindow::DFileManagerWindow(QWidget *parent) : DMovableMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowIcon(QIcon(":/images/images/system-file-manager.png"));
    initData();
    initUI();
    initConnect();
}

DFileManagerWindow::~DFileManagerWindow()
{

}

void DFileManagerWindow::initData()
{

}

void DFileManagerWindow::initUI()
{
    resize(950, 600);
    setMinimumWidth(MinimumWidth);
    moveCenter();
    initCentralWidget();
    //initStatusBar();
    setCentralWidget(m_centralWidget);
    setStyleSheet(getQssFromFile(":/qss/qss/filemanager.qss"));
}

void DFileManagerWindow::initTitleBar()
{
    m_titleBar = new DTitleBar;
    m_titleBar->setObjectName("TitleBar");
    m_titleBar->setFixedHeight(40);
    m_titleBar->setFocusPolicy(Qt::ClickFocus);
    setDragMovableHeight(m_titleBar->height());
}

void DFileManagerWindow::initSplitter()
{
    initLeftSideBar();
    initRightView();

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setFixedWidth(5);
    m_splitter->addWidget(m_leftSideBar);
    m_splitter->addWidget(m_rightView);
}

void DFileManagerWindow::initLeftSideBar()
{
    m_leftSideBar = new DLeftSideBar(this);
    m_leftSideBar->setObjectName("LeftSideBar");
    m_leftSideBar->setMinimumWidth(30);
    m_leftSideBar->setMaximumWidth(200);
    m_leftSideBar->setFixedWidth(160);
    m_leftSideBar->setFocusPolicy(Qt::ClickFocus);
}

void DFileManagerWindow::initRightView()
{
    initToolBar();
    initFileView();
    initDetailView();
    m_rightView = new QFrame;

    QHBoxLayout * titleLayout = new QHBoxLayout;
    titleLayout->addWidget(m_toolbar);
    titleLayout->addWidget(m_titleBar);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* viewLayout = new QHBoxLayout;
    viewLayout->addWidget(m_fileView);
    viewLayout->addWidget(m_detailView);
    viewLayout->setSpacing(0);
    viewLayout->setContentsMargins(0, 0, 0, 0);

    m_statusBar = new QStatusBar;
    m_statusBar->setFixedHeight(30);
    m_statusBar->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(titleLayout);
    mainLayout->addLayout(viewLayout);
    mainLayout->addWidget(m_statusBar);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_rightView->setLayout(mainLayout);
}

void DFileManagerWindow::initToolBar()
{
    m_toolbar = new DToolBar(this);
    m_toolbar->setObjectName("ToolBar");
    m_toolbar->setFixedHeight(40);
    m_toolbar->setFocusPolicy(Qt::ClickFocus);
}

void DFileManagerWindow::initFileView()
{
    m_fileView = new DFileView(this);
    m_fileView->setObjectName("FileView");
}

void DFileManagerWindow::initDetailView()
{
    m_detailView = new DDetailView(this);
    m_detailView->setObjectName("DetailView");
    m_detailView->setFixedWidth(200);
    m_detailView->hide();
}

void DFileManagerWindow::initCentralWidget()
{
    initTitleBar();
    initSplitter();

    m_centralWidget = new QFrame(this);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_splitter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_centralWidget->setLayout(mainLayout);
}

void DFileManagerWindow::initStatusBar()
{
    m_statusBar = new QStatusBar(this);
    m_statusBar->setFixedHeight(30);
    m_statusBar->setFocusPolicy(Qt::ClickFocus);
    setStatusBar(m_statusBar);
}

void DFileManagerWindow::initConnect()
{
    connect(m_titleBar, SIGNAL(minimuned()), this, SLOT(showMinimized()));
    connect(m_titleBar, SIGNAL(switchMaxNormal()), this, SLOT(toggleMaxNormal()));
    connect(m_titleBar, SIGNAL(closed()), this, SLOT(close()));
    connect(m_toolbar, SIGNAL(requestSwitchLayout()), this, SLOT(toggleLayout()));
    connect(m_toolbar, &DToolBar::backButtonClicked,
            this, [this] {
        QDir dir(QUrl(m_fileView->currentUrl()).toLocalFile());

        dir.cdUp();

        emit fileSignalManager->currentUrlChanged(QUrl::fromLocalFile(dir.absolutePath()).toString(QUrl::EncodeUnicode));
    });
    connect(m_toolbar, &DToolBar::switchLayoutMode,
            m_fileView, &DFileView::switchListMode);
    connect(m_toolbar, &DToolBar::refreshButtonClicked,
            this, [this] {
        emit fileSignalManager->refreshFolder(m_fileView->currentUrl());
    });
}

void DFileManagerWindow::toggleMaxNormal()
{
    if (isMaximized()){
        showNormal();
        m_titleBar->setNormalIcon();
    }else{
        showMaximized();
        m_titleBar->setMaxIcon();
    }
}

void DFileManagerWindow::toggleLayout()
{
    m_leftSideBar->setVisible(not m_leftSideBar->isVisible());
}

void DFileManagerWindow::resizeEvent(QResizeEvent *event)
{
    if (event->size().width() <= DFileManagerWindow::MinimumWidth){
        if (m_toolbar->getLayoutbuttonState() == DStateButton::stateA){
            if (m_leftSideBar->isVisible() && event->size().width() < event->oldSize().width()){
                m_leftSideBar->hide();
                m_toolbar->setLayoutButtonState(DStateButton::stateB);
            }
        }
    }else{
//        if (m_toolbar->getLayoutbuttonState() == DStateButton::stateB){
//            if (!m_leftSideBar->isVisible()){
//                m_leftSideBar->show();
//            }

//        }
    }
    DMovableMainWindow::resizeEvent(event);
}
