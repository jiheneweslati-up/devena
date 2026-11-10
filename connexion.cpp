#include "connexion.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QVariant>
#include <QDebug>

// Ctor : initialise l'attribut `db` avec le driver ODBC (stocké dans le Singleton)
Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

// Dtor : ferme la connexion si elle est encore ouverte
Connection::~Connection() {
    if (db.isOpen())
        db.close();
}

// Accès à l'instance unique du Singleton (Meyers Singleton)
Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createConnection() {
    db.setDatabaseName("Devena");          // nom de la source de données ODBC (64 bits)
    db.setUserName("Devena");              // utilisateur Oracle (schéma DEVENA de la base commune)
    db.setPassword("esprit2006");          // mot de passe Oracle

    // Les NUMBER Oracle sont lus directement en double (évite « 4,6 » / « 4.6 »)
    db.setNumericalPrecisionPolicy(QSql::LowPrecisionDouble);

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }

    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}

// ---------------------------------------------------------------------------
// Mode démonstration : remplace la connexion ODBC par une base SQLite en
// mémoire contenant les mêmes tables (EVALUATION, PROJETS, EQUIPES, EMPLOYES, GERER, PARTICIPANTS).
// Le code de la classe Evaluation reste identique : seules les données changent.
// ---------------------------------------------------------------------------
bool Connection::createDemoConnection() {
    const QString nom = db.connectionName();
    if (db.isOpen())
        db.close();
    db = QSqlDatabase();                    // libérer la référence avant removeDatabase
    QSqlDatabase::removeDatabase(nom);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        qDebug() << "Base de démonstration impossible :" << db.lastError().text();
        return false;
    }
    return BaseDemo::initialiser();
}

bool Connection::estModeDemo() {
    return QSqlDatabase::database(QSqlDatabase::defaultConnection, false).driverName() == "QSQLITE";
}

// ===========================================================================
// Base de démonstration (SQLite en mémoire)
// ===========================================================================

namespace {

bool executer(const QString &sql)
{
    QSqlQuery q;
    if (!q.exec(sql)) {
        qDebug() << "BaseDemo :" << q.lastError().text() << sql;
        return false;
    }
    return true;
}

} // namespace

bool BaseDemo::initialiser()
{
    // ----- Tables : mêmes noms, colonnes et contraintes que database/devena.sql -----
    const char *tables[] = {
        "CREATE TABLE EVALUATION ("
        " ID_EVALUATION TEXT PRIMARY KEY,"
        " NOTE REAL NOT NULL CHECK (NOTE BETWEEN 0 AND 20),"
        " REMARQUE TEXT NOT NULL, DATE_EVALUATION DATE, NOM TEXT, TYPE TEXT)",

        "CREATE TABLE PROJETS ("
        " ID_PROJET TEXT PRIMARY KEY, TITRE TEXT NOT NULL, DESCRIPTION TEXT NOT NULL,"
        " THEMATIQUE TEXT NOT NULL CHECK (THEMATIQUE IN ('IA', 'Web', 'IoT')),"
        " STATUT TEXT, DATE_CREATION DATE, DATE_LIMITE DATE)",

        "CREATE TABLE EQUIPES ("
        " ID_EQUIPE TEXT PRIMARY KEY, NOM_EQUIPE TEXT NOT NULL,"
        " NB_MEMBRES INTEGER NOT NULL, CAPACITE_MAX INTEGER DEFAULT 6 NOT NULL,"
        " SEXE TEXT NOT NULL, AGE INTEGER NOT NULL, COMPETENCES TEXT,"
        " ID_EVALUATION TEXT NOT NULL REFERENCES EVALUATION(ID_EVALUATION),"
        " ID_PROJET TEXT REFERENCES PROJETS(ID_PROJET), DATE_CREATION DATE)",

        "CREATE TABLE EMPLOYES ("
        " ID_EMPLOYE TEXT PRIMARY KEY, NOM TEXT NOT NULL, PRENOM TEXT NOT NULL,"
        " MOT_DE_PASSE TEXT NOT NULL, POSTE TEXT NOT NULL)",

        "CREATE TABLE GERER ("
        " ID_EMPLOYE TEXT NOT NULL REFERENCES EMPLOYES(ID_EMPLOYE),"
        " ID_EVALUATION TEXT NOT NULL REFERENCES EVALUATION(ID_EVALUATION),"
        " ID_GERER TEXT PRIMARY KEY)",

        "CREATE TABLE PARTICIPANTS ("
        " ID_PARTICIPANT TEXT PRIMARY KEY, NOM TEXT NOT NULL, PRENOM TEXT NOT NULL,"
        " EMAIL TEXT NOT NULL UNIQUE, COMPETENCES TEXT NOT NULL,"
        " ID_EQUIPE TEXT NOT NULL REFERENCES EQUIPES(ID_EQUIPE), SEXE TEXT, NUM INTEGER)",
    };
    for (const char *sql : tables)
        if (!executer(QString::fromUtf8(sql)))
            return false;

    // ----- Données de test (reprises de database/devena.sql) -----
    const char *donnees[] = {
        "INSERT INTO EVALUATION VALUES ('EV01', 16.5, 'Jury IA : modeles convaincants, donnees d''entrainement bien preparees.', NULL, NULL, NULL)",
        "INSERT INTO EVALUATION VALUES ('EV02', 14.5, 'Jury Web : interfaces soignees, demonstrations a mieux preparer.', NULL, NULL, NULL)",
        "INSERT INTO EVALUATION VALUES ('EV03', 15.5, 'Jury IoT : prototypes fonctionnels, documentation technique a completer.', NULL, NULL, NULL)",
        "INSERT INTO EVALUATION VALUES ('EV04', 13, 'Jury Securite : bonne analyse des risques, tests d''intrusion insuffisants.', NULL, NULL, NULL)",

        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P001', 'Smart City', 'Eclairage public, trafic et stationnement', 'IA')",
        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P002', 'Health App', 'Suivi medical des etudiants', 'Web')",
        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P003', 'EduTech', 'Quiz adaptatifs', 'Web')",
        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P004', 'GreenRoute', 'Qualite de l''air', 'IoT')",
        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P005', 'SecureVote', 'Detection des votes suspects', 'IA')",
        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P006', 'CampusNav', 'Plan interactif du campus', 'Web')",
        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P007', 'MediScan', 'Analyse d''images medicales', 'IA')",
        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P009', 'FarmSense', 'Arrosage intelligent', 'IoT')",
        "INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE) VALUES ('P010', 'JobMatch', 'Etudiants et entreprises', 'Web')",

        "INSERT INTO EQUIPES VALUES ('e1', 'equipe degital', 5, 6, 'h2/3f', 22, 'Web, UI/UX, JavaScript', 'EV02', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e2', 'NovaLab', 4, 5, '2H/2F', 21, 'IA, Python, vision par ordinateur', 'EV01', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e3', 'Pixel Crew', 5, 6, '3H/2F', 22, 'Web, React, Node.js', 'EV02', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e4', 'DataMiners', 3, 4, '1H/2F', 23, 'Data science, Python, SQL', 'EV01', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e5', 'GreenCode', 4, 5, '2H/2F', 20, 'IoT, Arduino, capteurs', 'EV03', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e6', 'CyberShield', 4, 6, '3H/1F', 22, 'Cybersecurite, reseaux, Linux', 'EV04', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e7', 'ByteForce', 4, 6, '3H/1F', 21, 'Java, Spring Boot, Angular', 'EV02', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e8', 'AgriTech Squad', 3, 4, '2H/1F', 22, 'IoT, ESP32, MQTT', 'EV03', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e9', 'MedAI', 4, 5, '1H/3F', 23, 'IA, imagerie medicale, PyTorch', 'EV01', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e10', 'Code Phoenix', 5, 6, '3H/2F', 21, 'IA, Qt/C++, OpenCV', 'EV01', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e11', 'CloudNine', 3, 5, '2H/1F', 20, 'Web, Flutter, Firebase', 'EV02', NULL, NULL)",
        "INSERT INTO EQUIPES VALUES ('e12', 'Quantum Bits', 4, 6, '2H/2F', 22, 'IoT, Raspberry Pi, Python', 'EV03', NULL, NULL)",

        "INSERT INTO EMPLOYES VALUES ('001', 'aakari', 'mahdi', 'ai', 'EV01')",
        "INSERT INTO EMPLOYES VALUES ('002', 'Ben Salah', 'Amira', 'Data science', 'EV01')",
        "INSERT INTO EMPLOYES VALUES ('003', 'Gharbi', 'Walid', 'Developpement web', 'EV02')",
        "INSERT INTO EMPLOYES VALUES ('004', 'Trabelsi', 'Ines', 'UX design', 'EV02')",
        "INSERT INTO EMPLOYES VALUES ('005', 'Mansouri', 'Karim', 'Systemes embarques', 'EV03')",
        "INSERT INTO EMPLOYES VALUES ('006', 'Jebali', 'Sonia', 'Reseaux IoT', 'EV03')",
        "INSERT INTO EMPLOYES VALUES ('007', 'Haddad', 'Nizar', 'Cybersecurite', 'EV04')",
        "INSERT INTO EMPLOYES VALUES ('008', 'Karray', 'Leila', 'Securite reseau', 'EV04')",
    };
    for (const char *sql : donnees)
        if (!executer(QString::fromUtf8(sql)))
            return false;
    return true;
}
