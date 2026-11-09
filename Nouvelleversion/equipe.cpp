
#include "equipe.h"

#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>

// Constructeur
Equipe::Equipe(int id, const QString &nom)
    : m_id(id), m_nom(nom)
{
}

// Getters
int Equipe::id() const
{
    return m_id;
}

QString Equipe::nom() const
{
    return m_nom;
}

// Setter
void Equipe::setNom(const QString &nom)
{
    m_nom = nom;
}

// Ajouter un membre en memoire
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
    return true;
}

// Supprimer un membre en memoire
bool Equipe::supprimer(int id)
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

void Equipe::vider()
{
    m_membres.clear();
}

int Equipe::taille() const
{
    return m_membres.size();
}

bool Equipe::pleine() const
{
    return m_membres.size() >= 6;
}

int Equipe::hommes() const
{
    int total = 0;

    for (const Participant &p : m_membres)
    {
        if (p.sexe.compare("H", Qt::CaseInsensitive) == 0 ||
            p.sexe.compare("Homme", Qt::CaseInsensitive) == 0 ||
            p.sexe.compare("Masculin", Qt::CaseInsensitive) == 0)
        {
            ++total;
        }
    }

    return total;
}

int Equipe::femmes() const
{
    int total = 0;

    for (const Participant &p : m_membres)
    {
        if (p.sexe.compare("F", Qt::CaseInsensitive) == 0 ||
            p.sexe.compare("Femme", Qt::CaseInsensitive) == 0 ||
            p.sexe.compare("Feminin", Qt::CaseInsensitive) == 0 ||
            p.sexe.compare("Féminin", Qt::CaseInsensitive) == 0)
        {
            ++total;
        }
    }

    return total;
}

double Equipe::ageMoyen() const
{
    if (m_membres.isEmpty())
        return 0.0;

    int total = 0;

    for (const Participant &p : m_membres)
        total += p.age;

    return static_cast<double>(total) / m_membres.size();
}

bool Equipe::aCompetence(const QString &c) const
{
    for (const Participant &p : m_membres)
    {
        if (p.competence.compare(c, Qt::CaseInsensitive) == 0)
            return true;
    }

    return false;
}

QString Equipe::statut() const
{
    if (pleine())
        return "Complete";

    return "Incomplete";
}

const QVector<Participant> &Equipe::membres() const
{
    return m_membres;
}

// CRUD : Ajouter une equipe dans Oracle
bool Equipe::ajouterBD()
{
    QSqlQuery query;

    query.prepare(
        "INSERT INTO EQUIPE (ID_EQUIPE, NOM) "
        "VALUES (:id, :nom)"
        );

    query.bindValue(":id", m_id);
    query.bindValue(":nom", m_nom);

    if (!query.exec())
    {
        qDebug() << "Erreur ajout equipe :"
                 << query.lastError().text();
        return false;
    }

    return true;
}

// CRUD : Modifier une equipe dans Oracle
bool Equipe::modifierBD()
{
    QSqlQuery query;

    query.prepare(
        "UPDATE EQUIPE SET NOM = :nom "
        "WHERE ID_EQUIPE = :id"
        );

    query.bindValue(":nom", m_nom);
    query.bindValue(":id", m_id);

    if (!query.exec())
    {
        qDebug() << "Erreur modification equipe :"
                 << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

// CRUD : Supprimer une equipe dans Oracle
bool Equipe::supprimerBD(int id)
{
    QSqlQuery query;

    query.prepare(
        "DELETE FROM EQUIPE WHERE ID_EQUIPE = :id"
        );

    query.bindValue(":id", id);

    if (!query.exec())
    {
        qDebug() << "Erreur suppression equipe :"
                 << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

// CRUD : Afficher les equipes depuis Oracle
QSqlQueryModel * Equipe::afficherBD()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_EQUIPE, NOM FROM EQUIPE ORDER BY ID_EQUIPE"
        );

    if (model->lastError().isValid())
    {
        qDebug() << "Erreur affichage equipes :"
                 << model->lastError().text();
    }

    return model;
}
