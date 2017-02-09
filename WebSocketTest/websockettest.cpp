#include "websockettest.h"
#include "ui_websockettest.h"

WebSocketTest::WebSocketTest(QWidget *parent) :
    QMainWindow(parent), server(new QWebSocketServer("WebSocketTest", QWebSocketServer::NonSecureMode, this)), ui(new Ui::WebSocketTest) {

    ui->setupUi(this);
    textBrowser = ui->textBrowser;
    textEdit = ui->textEdit;
    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+Return"), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(sendMessage()));

    if (server->listen(QHostAddress::Any, PORT)) {
        textBrowser->append("Server listening on port " + QString::number(PORT) + "\nPress Ctrl + Enter to send messages\n");
        QObject::connect(server, SIGNAL(newConnection()), this, SLOT(connected()));
        QObject::connect(server, SIGNAL(closed()), this, SLOT(disconnected()));
    }
    QTimer *timer = new QTimer;
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(TICK_LENGTH);
}

WebSocketTest::~WebSocketTest() {
    server->close();
    delete ui;
}

void WebSocketTest::connected() {
    QWebSocket *socket = server->nextPendingConnection();
    QObject::connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(messageReceived(QString)));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    clients << socket;
}

void WebSocketTest::messageReceived(QString message) {
    for (QWebSocket* client : clients) {
        client->sendTextMessage(message);
    }
    textBrowser->append("> " + message);
}

void WebSocketTest::sendMessage() {
    messageReceived(textEdit->toPlainText());
    textEdit->clear();
}

void WebSocketTest::disconnected() {
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    clients.removeAll(client);
    client->deleteLater();
}

void WebSocketTest::on_actionExit_triggered(){
    if (QMessageBox::question(this, APP_NAME, EXIT_CONFIRM_TEXT,
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) QApplication::quit();
}

void WebSocketTest::update() {
    textBrowser->resize(this->width(), this->height() / 2 - HEIGHT_OFFSET);
    textEdit->move(0, this->height() / 2 - HEIGHT_OFFSET);
    textEdit->resize(this->width(), this->height() / 2 - HEIGHT_OFFSET);
}