#ifndef PROJET_H
#define PROJET_H

#include <QDate>
#include <QList>
#include <QString>

// Entite Projet : une ligne de la table PROJETS.
// Ce fichier contient uniquement le CRUD.
class Projet
{
public:
    Projet() = default;
    Projet(const QString &id, const QString &titre, const QString &description,
           const QString &thematique, const QString &statut,
           const QDate &dateCreation, const QDate &dateLimite);

    // Accesseurs
    QString id() const           { return m_id; }
    QString titre() const        { return m_titre; }
    QString description() const  { return m_description; }
    QString thematique() const   { return m_thematique; }
    QString statut() const       { return m_statut; }
    QDate   dateCreation() const { return m_dateCreation; }
    QDate   dateLimite() const   { return m_dateLimite; }

    void setId(const QString &v)           { m_id = v; }
    void setTitre(const QString &v)        { m_titre = v; }
    void setDescription(const QString &v)  { m_description = v; }
    void setThematique(const QString &v)   { m_thematique = v; }
    void setStatut(const QString &v)       { m_statut = v; }
    void setDateCreation(const QDate &v)   { m_dateCreation = v; }
    void setDateLimite(const QDate &v)     { m_dateLimite = v; }

    // CRUD : renvoie true si OK, sinon false avec le message Oracle dans *erreur
    bool ajouter(QString *erreur = nullptr) const;                          // Create
    static QList<Projet> lister(const QString &recherche = QString(),      // Read (liste)
                                QString *erreur = nullptr);
    static bool charger(const QString &id, Projet &resultat,                // Read (un seul)
                        QString *erreur = nullptr);
    static bool existe(const QString &id);                                  // Read (existe ?)
    bool modifier(QString *erreur = nullptr) const;                         // Update
    static bool supprimer(const QString &id, QString *erreur = nullptr);    // Delete

private:
    QString m_id;
    QString m_titre;
    QString m_description;
    QString m_thematique;
    QString m_statut;
    QDate   m_dateCreation;
    QDate   m_dateLimite;
};

#endif // PROJET_H
