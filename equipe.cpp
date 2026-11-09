
#include "equipe.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QStringList>
#include <QDebug>

// ==========================================
// CONSTRUCTEURS
// ==========================================

Equipe::Equipe(int id, const QString &nom)
    : m_id(id),
    m_nom(nom),
    m_dateCreation(QDate::currentDate()),
    m_ageMoyen(0.0),
    m_idProjet(0),
    m_idEvaluation(0)
{
}

Equipe::Equipe(int id, const QString &nom,
               int idProjet, int idEvaluation)
    : m_id(id),
    m_nom(nom),
    m_dateCreation(QDate::currentDate()),
    m_ageMoyen(0.0),
    m_idProjet(idProjet),
    m_idEvaluation(idEvaluation)
{
}

// ==========================================
// GETTERS
// ==========================================

int Equipe::id() const
{
    return m_id;
}

QString Equipe::nom() const
{
    return m_nom;
}

QDate Equipe::dateCreation() const
{
    return m_dateCreation;
}

int Equipe::idProjet() const
{
    return m_idProjet;
}

int Equipe::idEvaluation() const
{
    return m_idEvaluation;
}

double Equipe::ageMoyen() const
{
    return m_ageMoyen;
}

const QVector<Participant> &Equipe::membres() const
{
    return m_membres;
}

// ==========================================
// SETTERS
// ==========================================

void Equipe::setNom(const QString &nom)
{
    m_nom = nom;
}

void Equipe::setIdProjet(int idProjet)
{
    m_idProjet = idProjet;
}

void Equipe::setIdEvaluation(int idEvaluation)
{
    m_idEvaluation = idEvaluation;
}

void Equipe::setAgeMoyen(double age)
{
    m_ageMoyen = age;
}

// ==========================================
// AJOUTER MEMBRE
// ==========================================

bool Equipe::ajouter(const Participant &p)
{
    if (pleine())
        return false;

    for (const Participant &m : m_membres)
    {
        if (m.id == p.id)
            return false;
    }

    m_membres.append(p);

    int totalAge = 0;

    for (const Participant &m : m_membres)
        totalAge += m.age;

    m_ageMoyen = m_membres.isEmpty()
                     ? 0.0
                     : double(totalAge) / m_membres.size();

    return true;
}

// ==========================================
// SUPPRIMER MEMBRE
// ==========================================

bool Equipe::supprimerMembre(int id)
{
    for (int i = 0; i < m_membres.size(); ++i)
    {
        if (m_membres[i].id == id)
        {
            m_membres.removeAt(i);
            return true;
        }
    }

    return false;
}

// ==========================================
// VIDER EQUIPE
// ==========================================

void Equipe::vider()
{
    m_membres.clear();
    m_ageMoyen = 0.0;
}

// ==========================================
// NOMBRE DE MEMBRES
// ==========================================

int Equipe::taille() const
{
    return m_membres.size();
}

// ==========================================
// VERIFIER SI EQUIPE PLEINE
// ==========================================

bool Equipe::pleine() const
{
    return taille() >= 6;
}

// ==========================================
// NOMBRE HOMMES
// ==========================================

int Equipe::hommes() const
{
    int total = 0;

    for (const Participant &p : m_membres)
    {
        if (p.sexe.compare("Homme", Qt::CaseInsensitive) == 0)
            total++;
    }

    return total;
}

// ==========================================
// NOMBRE FEMMES
// ==========================================

int Equipe::femmes() const
{
    int total = 0;

    for (const Participant &p : m_membres)
    {
        if (p.sexe.compare("Femme", Qt::CaseInsensitive) == 0)
            total++;
    }

    return total;
}

// ==========================================
// VERIFIER COMPETENCE
// ==========================================

bool Equipe::aCompetence(const QString &c) const
{
    for (const Participant &p : m_membres)
    {
        if (p.competence.compare(c, Qt::CaseInsensitive) == 0)
            return true;
    }

    return false;
}

// ==========================================
// STATUT EQUIPE
// ==========================================

QString Equipe::statut() const
{
    if (m_membres.isEmpty())
        return "Vide";

    if (pleine())
        return "Complete";

    return "Active";
}

// ==========================================
// AJOUTER EQUIPE : MODELE SQL
// ==========================================

bool Equipe::ajouter()
{
    QSqlQuery query;

    QString res = "e" + QString::number(m_id);

    QString sexe = QString("%1 H / %2 F")
                       .arg(hommes())
                       .arg(femmes());

    QStringList competences;

    for (const Participant &p : m_membres)
    {
        QString comp = p.competence.trimmed();

        if (!comp.isEmpty() &&
            !competences.contains(comp, Qt::CaseInsensitive))
        {
            competences.append(comp);
        }
    }

    QString competenceText = competences.isEmpty()
                                 ? "-"
                                 : competences.join(", ");

    query.prepare(
        "INSERT INTO EQUIPES "
        "(ID_EQUIPE, NOM_EQUIPE, NB_MEMBRES, "
        "CAPACITE_MAX, SEXE, AGE, COMPETENCES, DATE_CREATION) "
        "VALUES (:id, :nom, :nb, :capacite, "
        ":sexe, :age, :competences, :date)"
        );

    query.bindValue(":id", res);
    query.bindValue(":nom", m_nom);
    query.bindValue(":nb", taille());
    query.bindValue(":capacite", 6);
    query.bindValue(":sexe", sexe);
    query.bindValue(":age", m_ageMoyen);
    query.bindValue(":competences", competenceText);
    query.bindValue(":date", m_dateCreation);

    if (!query.exec())
    {
        qDebug() << "Erreur SQL Ajouter Equipe :"
                 << query.lastError().text();

        return false;
    }

    return true;
}

// ==========================================
// MODIFIER EQUIPE : MODELE SQL
// ==========================================

bool Equipe::modifier()
{
    QSqlQuery query;

    QString res = "e" + QString::number(m_id);

    query.prepare(
        "UPDATE EQUIPES "
        "SET NOM_EQUIPE = :nom "
        "WHERE ID_EQUIPE = :id"
        );

    query.bindValue(":nom", m_nom);
    query.bindValue(":id", res);

    return query.exec();
}

// ==========================================
// SUPPRIMER EQUIPE : MODELE SQL
// ==========================================

bool Equipe::supprimer(int id)
{
    QSqlDatabase db = QSqlDatabase::database();

    QString res = "e" + QString::number(id);

    // Début de la transaction
    if (!db.transaction())
        return false;

    // 1. Supprimer les participants liés à l'équipe
    QSqlQuery queryParticipants(db);

    queryParticipants.prepare(
        "DELETE FROM PARTICIPANTS WHERE ID_EQUIPE = :id"
        );

    queryParticipants.bindValue(":id", res);

    if (!queryParticipants.exec())
    {
        qDebug() << "Erreur suppression participants :"
                 << queryParticipants.lastError().text();

        db.rollback();
        return false;
    }

    // 2. Supprimer l'équipe
    QSqlQuery queryEquipe(db);

    queryEquipe.prepare(
        "DELETE FROM EQUIPES WHERE ID_EQUIPE = :id"
        );

    queryEquipe.bindValue(":id", res);

    if (!queryEquipe.exec())
    {
        qDebug() << "Erreur suppression equipe :"
                 << queryEquipe.lastError().text();

        db.rollback();
        return false;
    }

    // Valider la transaction
    if (!db.commit())
    {
        qDebug() << "Erreur commit :"
                 << db.lastError().text();

        db.rollback();
        return false;
    }

    return true;
}


// ==========================================
// AFFICHER EQUIPES : MODELE SQL
// ==========================================

QSqlQueryModel *Equipe::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery("SELECT * FROM EQUIPES");

    model->setHeaderData(
        0, Qt::Horizontal, QObject::tr("ID Equipe"));

    model->setHeaderData(
        1, Qt::Horizontal, QObject::tr("Nom Equipe"));

    model->setHeaderData(
        2, Qt::Horizontal, QObject::tr("Nombre de membres"));

    model->setHeaderData(
        3, Qt::Horizontal, QObject::tr("Capacite maximale"));

    model->setHeaderData(
        4, Qt::Horizontal, QObject::tr("Sexe"));

    model->setHeaderData(
        5, Qt::Horizontal, QObject::tr("Age moyen"));

    model->setHeaderData(
        6, Qt::Horizontal, QObject::tr("Competences"));

    model->setHeaderData(
        7, Qt::Horizontal, QObject::tr("Date de creation"));

    model->setHeaderData(
        8, Qt::Horizontal, QObject::tr("ID Projet"));

    model->setHeaderData(
        9, Qt::Horizontal, QObject::tr("ID Evaluation"));

    return model;
}
