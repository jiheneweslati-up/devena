#include "projet.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QDebug>

// Le pilote ODBC Oracle ajoute des caractères parasites après le message :
// on n'affiche que le code ORA, traduit quand il est connu.
static QString messageOracle(const QSqlError &e)
{
    const QRegularExpressionMatch m = QRegularExpression("ORA-(\\d{5})").match(e.databaseText());
    if (!m.hasMatch())
        return "Erreur de base de données. Vérifiez la connexion Oracle.";

    const QString code = m.captured(1);
    if (code == "00001")
        return "Cet identifiant existe déjà (ORA-00001).";
    if (code == "02290")
        return "Une contrainte de la table n'est pas respectée (ORA-02290).";
    if (code == "02292")
        return "Des équipes sont encore liées à ce projet (ORA-02292).";
    if (code == "12899")
        return "Une valeur est trop longue pour la colonne (ORA-12899).";
    return "Erreur Oracle ORA-" + code + ".";
}

QString Projet::dernierErreur() const
{
    return erreur;
}

bool Projet::controleSaisie()
{
    ID_Projet = ID_Projet.trimmed();
    titre = titre.trimmed();
    description = description.trimmed();
    thematique = thematique.trimmed();

    if (ID_Projet.isEmpty() || titre.isEmpty() || description.isEmpty()) {
        erreur = "ID_Projet, Titre et Description sont obligatoires.";
        return false;
    }

    if (ID_Projet.length() > 30) {
        erreur = "ID_Projet ne peut pas dépasser 30 caractères.";
        return false;
    }

    if (titre.length() > 80) {
        erreur = "Le titre ne peut pas dépasser 80 caractères.";
        return false;
    }

    if (description.length() > 400) {
        erreur = "La description ne peut pas dépasser 400 caractères.";
        return false;
    }

    const QStringList themes = {"IA", "Web", "IoT"};
    if (!themes.contains(thematique)) {
        erreur = "La thématique doit être IA, Web ou IoT.";
        return false;
    }

    if (noteFinale < 0.0 || noteFinale > 20.0) {
        erreur = "La note finale doit être comprise entre 0 et 20.";
        return false;
    }

    return true;
}

bool Projet::existe(const QString &id)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM projets WHERE UPPER(id_projet) = UPPER(:id)");
    query.bindValue(":id", id);

    if (!query.exec()) {
        erreur = messageOracle(query.lastError());
        qDebug() << "Échec de la vérification du projet :" << query.lastError().text();
        return false;
    }

    return query.next();
}

QStringList Projet::equipesDuProjet(const QString &id)
{
    QStringList noms;

    QSqlQuery query;
    query.prepare("SELECT nom_equipe FROM equipes WHERE id_projet = :id ORDER BY nom_equipe");
    query.bindValue(":id", id);

    if (!query.exec()) {
        erreur = messageOracle(query.lastError());
        qDebug() << "Échec de la lecture des équipes du projet :" << query.lastError().text();
        return noms;
    }

    while (query.next())
        noms.append(query.value(0).toString());

    return noms;
}

bool Projet::ajouter()
{
    erreur.clear();

    if (!controleSaisie())
        return false;

    const bool dejaPresent = existe(ID_Projet);
    if (!erreur.isEmpty())
        return false;
    if (dejaPresent) {
        erreur = "Cet ID_Projet existe déjà.";
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO projets (id_projet, titre, description, thematique, note_finale) "
        "VALUES (:id, :titre, :description, :thematique, :note)"
        );
    query.bindValue(":id", ID_Projet);
    query.bindValue(":titre", titre);
    query.bindValue(":description", description);
    query.bindValue(":thematique", thematique);
    query.bindValue(":note", noteFinale);

    if (!query.exec()) {
        erreur = messageOracle(query.lastError());
        qDebug() << "Échec de l'ajout du projet :" << query.lastError().text();
        return false;
    }

    return true;
}

bool Projet::modifier()
{
    erreur.clear();

    if (!controleSaisie())
        return false;

    QSqlQuery query;
    query.prepare(
        "UPDATE projets "
        "SET titre = :titre, description = :description, "
        "thematique = :thematique, note_finale = :note "
        "WHERE id_projet = :id"
        );
    query.bindValue(":titre", titre);
    query.bindValue(":description", description);
    query.bindValue(":thematique", thematique);
    query.bindValue(":note", noteFinale);
    query.bindValue(":id", ID_Projet);

    if (!query.exec()) {
        erreur = messageOracle(query.lastError());
        qDebug() << "Échec de la modification du projet :" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        erreur = "Le projet à modifier est introuvable.";
        return false;
    }

    return true;
}

bool Projet::supprimer(const QString &id)
{
    erreur.clear();

    const QStringList equipesLiees = equipesDuProjet(id);
    if (!erreur.isEmpty())
        return false;
    if (!equipesLiees.isEmpty()) {
        erreur = "Ce projet est développé par : " + equipesLiees.join(", ") + ".\n"
                 "Affectez d'abord ces équipes à un autre projet (module Équipes).";
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM projets WHERE id_projet = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        erreur = messageOracle(query.lastError());
        qDebug() << "Échec de la suppression du projet :" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        erreur = "Le projet à supprimer est introuvable.";
        return false;
    }

    return true;
}

QSqlQueryModel *Projet::afficher()
{
    erreur.clear();

    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT p.id_projet, p.titre, p.description, p.thematique, p.note_finale, "
        "COUNT(e.id_equipe), "
        "LISTAGG(e.nom_equipe, ', ') WITHIN GROUP (ORDER BY e.nom_equipe) "
        "FROM projets p "
        "LEFT JOIN equipes e ON e.id_projet = p.id_projet "
        "GROUP BY p.id_projet, p.titre, p.description, p.thematique, p.note_finale "
        "ORDER BY p.id_projet"
        );

    if (model->lastError().isValid()) {
        erreur = messageOracle(model->lastError());
        qDebug() << "Échec de l'affichage des projets :" << model->lastError().text();
        return model;
    }

    while (model->canFetchMore())
        model->fetchMore();

    model->setHeaderData(0, Qt::Horizontal, "ID Projet");
    model->setHeaderData(1, Qt::Horizontal, "Titre");
    model->setHeaderData(2, Qt::Horizontal, "Description");
    model->setHeaderData(3, Qt::Horizontal, "Thématique");
    model->setHeaderData(4, Qt::Horizontal, "Note finale");
    model->setHeaderData(5, Qt::Horizontal, "Nb équipes");
    model->setHeaderData(6, Qt::Horizontal, "Équipes");

    return model;
}
