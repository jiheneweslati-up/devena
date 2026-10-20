#ifndef PROJET_H
#define PROJET_H

#include <QString>
#include <QStringList>
#include <QSqlQueryModel>

class Projet
{
public:
    Projet() = default;

    QString ID_Projet;
    QString titre;
    QString description;
    QString thematique;
    double noteFinale = 0.0;

    // Lecture seule : remplis par afficher() depuis EQUIPES.ID_PROJET,
    // jamais enregistrés par ajouter() ou modifier().
    QString equipes;
    int nbEquipes = 0;

    bool ajouter();
    bool modifier();
    bool supprimer(const QString &id);
    QSqlQueryModel *afficher();

    QString dernierErreur() const;

private:
    QString erreur;

    bool controleSaisie();
    bool existe(const QString &id);
    QStringList equipesDuProjet(const QString &id);
};

#endif // PROJET_H
