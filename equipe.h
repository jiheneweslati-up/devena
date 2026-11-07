
#ifndef EQUIPE_H
#define EQUIPE_H

#include <QString>
#include <QVector>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>

struct Participant
{
    int id;
    QString nom;
    QString prenom;
    QString sexe;
    QString competence;
    int age;
};

class Equipe
{
public:

    // Constructeurs
    Equipe(int id = 0, const QString &nom = "");

    Equipe(int id, const QString &nom,
           int idProjet, int idEvaluation);

    // Getters
    int id() const;
    QString nom() const;
    QDate dateCreation() const;

    int idProjet() const;
    int idEvaluation() const;
    double ageMoyen() const;

    // Setters
    void setNom(const QString &nom);
    void setIdProjet(int idProjet);
    void setIdEvaluation(int idEvaluation);
    void setAgeMoyen(double age);

    // Gestion des membres
    bool ajouter(const Participant &p);
    bool supprimerMembre(int id);
    void vider();

    // Informations équipe
    int taille() const;
    bool pleine() const;
    int hommes() const;
    int femmes() const;
    bool aCompetence(const QString &c) const;
    QString statut() const;

    const QVector<Participant> &membres() const;

    // CRUD - Atelier 3
    bool ajouter();
    bool modifier();
    bool supprimer(int id);
    QSqlQueryModel *afficher();

private:

    int m_id;
    QString m_nom;
    QDate m_dateCreation;
    double m_ageMoyen;
    int m_idProjet;
    int m_idEvaluation;

    QVector<Participant> m_membres;
};

#endif // EQUIPE_H
