
#ifndef EQUIPE_H
#define EQUIPE_H

#include <QString>
#include <QVector>
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
private:
    int m_id;
    QString m_nom;
    QVector<Participant> m_membres;

public:
    // Constructeur
    Equipe(int id = 0, const QString &nom = "");

    // Getters
    int id() const;
    QString nom() const;

    // Setters
    void setNom(const QString &nom);

    // Gestion des membres
    bool ajouter(const Participant &p);
    bool supprimer(int id);
    void vider();

    int taille() const;
    bool pleine() const;

    int hommes() const;
    int femmes() const;
    double ageMoyen() const;
    bool aCompetence(const QString &c) const;
    QString statut() const;

    const QVector<Participant> &membres() const;

    // Fonctionnalites CRUD avec Oracle
    bool ajouterBD();
    bool modifierBD();
    bool supprimerBD(int id);
    QSqlQueryModel * afficherBD();
};

#endif // EQUIPE_H
