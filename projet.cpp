#include "projet.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

// Les dates passent par TO_DATE / TO_CHAR avec un format fixe : le resultat
// ne depend donc ni des reglages regionaux de Windows ni de ceux d'Oracle.
// Une date vide est envoyee comme chaine vide, qu'Oracle enregistre en NULL.
static QString versOracle(const QDate &d)
{
    return d.isValid() ? d.toString("yyyy-MM-dd") : QString();
}

static QDate depuisOracle(const QVariant &v)
{
    return QDate::fromString(v.toString(), "yyyy-MM-dd");
}

static void noterErreur(QString *erreur, const QSqlQuery &q)
{
    if (erreur)
        *erreur = q.lastError().text();
}

// Transaction : soit toutes les requetes passent, soit aucune n'est gardee
// (ex. on ne supprime pas le projet si le detachement des equipes echoue).
static bool debutTransaction()
{
    return QSqlDatabase::database().transaction();
}

static bool finTransaction(bool enCours, bool ok)
{
    if (!enCours)
        return ok;
    if (ok)
        return QSqlDatabase::database().commit();
    QSqlDatabase::database().rollback();
    return false;
}

// ===== Lien avec la table EQUIPES =====
// EQUIPES.ID_PROJET pointe vers le projet (cle FK_EQUIPE_PROJET).
// Avant de supprimer un projet, on detache ses equipes (ID_PROJET remis a NULL),
// sinon Oracle refuse la suppression avec ORA-02292.
static bool detacherEquipes(const QString &idProjet, QString *erreur)
{
    QSqlQuery q;
    q.prepare("UPDATE EQUIPES SET ID_PROJET = NULL WHERE ID_PROJET = :id");
    q.bindValue(":id", idProjet);
    if (!q.exec()) {
        noterErreur(erreur, q);
        return false;
    }
    return true;
}

// Colonnes lues par lister() et charger(), toujours dans cet ordre
static const char *COLONNES =
    "SELECT ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE, STATUT, "
    "TO_CHAR(DATE_CREATION, 'YYYY-MM-DD'), TO_CHAR(DATE_LIMITE, 'YYYY-MM-DD') "
    "FROM PROJETS ";

static Projet lireLigne(const QSqlQuery &q)
{
    return Projet(q.value(0).toString(), q.value(1).toString(),
                  q.value(2).toString(), q.value(3).toString(),
                  q.value(4).toString(), depuisOracle(q.value(5)),
                  depuisOracle(q.value(6)));
}

Projet::Projet(const QString &id, const QString &titre, const QString &description,
               const QString &thematique, const QString &statut,
               const QDate &dateCreation, const QDate &dateLimite)
    : m_id(id), m_titre(titre), m_description(description),
    m_thematique(thematique), m_statut(statut),
    m_dateCreation(dateCreation), m_dateLimite(dateLimite)
{
}

// ===== Create =====
bool Projet::ajouter(QString *erreur) const
{
    QSqlQuery q;
    q.prepare("INSERT INTO PROJETS (ID_PROJET, TITRE, DESCRIPTION, THEMATIQUE, STATUT, "
              "DATE_CREATION, DATE_LIMITE) "
              "VALUES (:id, :titre, :descr, :theme, :statut, "
              "TO_DATE(:creation, 'YYYY-MM-DD'), TO_DATE(:limite, 'YYYY-MM-DD'))");
    q.bindValue(":id", m_id);
    q.bindValue(":titre", m_titre);
    q.bindValue(":descr", m_description);
    q.bindValue(":theme", m_thematique);
    q.bindValue(":statut", m_statut);
    q.bindValue(":creation", versOracle(m_dateCreation));
    q.bindValue(":limite", versOracle(m_dateLimite));

    if (!q.exec()) {
        noterErreur(erreur, q);
        return false;
    }
    return true;
}

// ===== Read : liste, avec recherche optionnelle =====
// La recherche porte sur l'ID, le titre, la description, la thematique et le statut.
QList<Projet> Projet::lister(const QString &recherche, QString *erreur)
{
    QList<Projet> liste;
    QSqlQuery q;
    const QString motif = recherche.trimmed();

    if (motif.isEmpty()) {
        q.prepare(QString(COLONNES) + "ORDER BY ID_PROJET");
    } else {
        q.prepare(QString(COLONNES) +
                  "WHERE UPPER(ID_PROJET || ' ' || TITRE || ' ' || DESCRIPTION || ' ' "
                  "|| THEMATIQUE || ' ' || STATUT) LIKE UPPER(:motif) "
                  "ORDER BY ID_PROJET");
        q.bindValue(":motif", "%" + motif + "%");
    }

    if (!q.exec()) {
        noterErreur(erreur, q);
        return liste;
    }
    while (q.next())
        liste.append(lireLigne(q));
    return liste;
}

// ===== Read : un seul projet =====
bool Projet::charger(const QString &id, Projet &resultat, QString *erreur)
{
    QSqlQuery q;
    q.prepare(QString(COLONNES) + "WHERE ID_PROJET = :id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        noterErreur(erreur, q);
        return false;
    }
    if (!q.next()) {
        if (erreur)
            *erreur = QObject::tr("Aucun projet avec l'ID %1.").arg(id);
        return false;
    }
    resultat = lireLigne(q);
    return true;
}

// ===== Update =====
bool Projet::modifier(QString *erreur) const
{
    QSqlQuery q;
    q.prepare("UPDATE PROJETS SET TITRE = :titre, DESCRIPTION = :descr, "
              "THEMATIQUE = :theme, STATUT = :statut, "
              "DATE_CREATION = TO_DATE(:creation, 'YYYY-MM-DD'), "
              "DATE_LIMITE = TO_DATE(:limite, 'YYYY-MM-DD') "
              "WHERE ID_PROJET = :id");
    q.bindValue(":titre", m_titre);
    q.bindValue(":descr", m_description);
    q.bindValue(":theme", m_thematique);
    q.bindValue(":statut", m_statut);
    q.bindValue(":creation", versOracle(m_dateCreation));
    q.bindValue(":limite", versOracle(m_dateLimite));
    q.bindValue(":id", m_id);

    if (!q.exec()) {
        noterErreur(erreur, q);
        return false;
    }
    return true;
}

// ===== Delete =====
// Les equipes du projet sont d'abord detachees (voir detacherEquipes),
// le tout dans une transaction : si une etape echoue, rien n'est change.
bool Projet::supprimer(const QString &id, QString *erreur)
{
    const bool enCours = debutTransaction();

    bool ok = detacherEquipes(id, erreur);
    if (ok) {
        QSqlQuery q;
        q.prepare("DELETE FROM PROJETS WHERE ID_PROJET = :id");
        q.bindValue(":id", id);
        ok = q.exec();
        if (!ok)
            noterErreur(erreur, q);
    }
    return finTransaction(enCours, ok);
}

// ===== Read : le projet existe t il ? =====
bool Projet::existe(const QString &id)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM PROJETS WHERE ID_PROJET = :id");
    q.bindValue(":id", id);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}
