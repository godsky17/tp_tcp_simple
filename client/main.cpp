// client : se connecte a 127.0.0.1:9001, envoie des lignes, lit les acquittements.
// Les exercices indiquent etape par etape ce qu'il reste a ecrire.
#include <QCoreApplication>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>                       // NOUVEAU
#include <QByteArray>                   // NOUVEAU
#include <memory>                       // NOUVEAU

// NOUVEAU : état de réception (même convention que le serveur)
struct Etat {
    QByteArray tampon;                  // octets reçus pas encore traités
    int acquittements = 0;              // nombre d'accusés complets reçus
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTcpSocket socket;
    // Exercice 1 : connexion, envoi d'un message, fermeture propre.
    socket.connectToHost(QHostAddress::LocalHost, 9001);
    if(!socket.waitForConnected(3000)){
        qInfo() << socket.errorString();
        return 1;
    }else{
        qInfo() << "Adresse : " << QHostAddress::LocalHost << " Port : " << 9001;
        qInfo() << "Port local : " << socket.localPort();   // NOUVEAU : demandé par la consigne
        qInfo() << "Adresse distant : " << socket.peerAddress() << "Port distant : " << socket.peerPort();
    }
    // Exercice 2 : variable d'environnement COUNT, envoi de COUNT lignes,
    //            tampon de reception et decompte des acquittements recus.

    bool ok = false;
    int count = qEnvironmentVariableIntValue("COUNT", &ok);
    if (!ok || count <= 0)
        count = 3;

    // NOUVEAU : variable WAIT_MS (défaut 1000)
    int waitMs = qEnvironmentVariableIntValue("WAIT_MS", &ok);
    if (!ok || waitMs < 0)
        waitMs = 1000;

    // NOUVEAU : réception, branchée AVANT les écritures
    auto etat = std::make_shared<Etat>();
    QObject::connect(&socket, &QTcpSocket::readyRead, [&socket, etat]() {
        etat->tampon += socket.readAll();

        int pos;
        while ((pos = etat->tampon.indexOf('\n')) != -1) {
            QByteArray ligne = etat->tampon.left(pos);
            etat->tampon.remove(0, pos + 1);
            ++etat->acquittements;
            qInfo() << "Accuse recu :" << ligne.trimmed();
        }
    });

    int ecrits = 0;                     // NOUVEAU
    for (int i = 1; i <= count; ++i) {
        socket.write(QString("ping %1\n").arg(i).toUtf8());
        ++ecrits;
    }

    socket.waitForBytesWritten(3000);
    // Exercice 3 : fermeture propre, puis connexion sur un port sans serveur.

    // NOUVEAU : on rend la main à la boucle d'événements le temps de l'attente,
    // puis on affiche le bilan et on quitte.
    QTimer::singleShot(waitMs, &app, [etat, ecrits]() {
        qInfo() << "Bilan : messages ecrits =" << ecrits
                << "| accuses recus =" << etat->acquittements;
        qApp->quit();
    });

    qInfo() << "client demarre";
    return app.exec();                  // NOUVEAU : remplace return 0
}