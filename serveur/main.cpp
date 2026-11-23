#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QByteArray>
#include <memory>

struct Etat {
    QByteArray tampon;
    int compteur = 0;
    int lectures = 0;
    qint64 octets = 0;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTcpServer serveur;

    if (serveur.listen(QHostAddress::LocalHost, 9001)) { // ouverture de port et creation de fil d'attente (listen())
        qInfo() << "Adresse : " << QHostAddress::LocalHost << " Port : " << 9001;
    } else {
        qInfo() << serveur.errorString();
        return 1;
    }

    QObject::connect(&serveur, &QTcpServer::newConnection, [&serveur]() {
        while (serveur.hasPendingConnections()) {
            QTcpSocket *socket = serveur.nextPendingConnection();
            qInfo() << "Port distant : " << socket->peerPort()
                    << " | Adresse distant : " << socket->peerAddress()
                    << " | Port local : " << socket->localPort();

            auto etat = std::make_shared<Etat>();

            QObject::connect(socket, &QTcpSocket::readyRead, [socket, etat]() {
                ++etat->lectures;
                QByteArray bloc = socket->readAll();
                etat->octets += bloc.size();
                etat->tampon += bloc;

                int pos;
                while ((pos = etat->tampon.indexOf('\n')) != -1) {
                    QByteArray ligne = etat->tampon.left(pos);
                    etat->tampon.remove(0, pos + 1);
                    ++etat->compteur;

                    qInfo() << "Ligne reconnue :" << ligne.trimmed();
                    socket->write("ack " + QByteArray::number(etat->compteur) + "\n");
                }
            });

            QObject::connect(socket, &QTcpSocket::errorOccurred, [socket](QAbstractSocket::SocketError) {
                qInfo() << socket->errorString();
            });

            QObject::connect(socket, &QTcpSocket::disconnected, [socket, etat]() {
                qInfo() << "Bilan | octets recus :" << etat->octets
                        << "| lignes traitees :" << etat->compteur
                        << "| octets restants sans \\n :" << etat->tampon.size()
                        << "| readyRead :" << etat->lectures;
                socket->deleteLater();
            });
        }
    });

    int EXIT_MS = 10000;
    if (EXIT_MS > 0) {
        QTimer::singleShot(EXIT_MS, qApp, &QCoreApplication::quit);
    }

    return app.exec();
}