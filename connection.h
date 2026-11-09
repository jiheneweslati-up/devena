#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>

// Ouvre la connexion Oracle une seule fois au demarrage de l'application.
// Toutes les QSqlQuery creees ensuite utilisent automatiquement cette connexion.
class Connection
{
public:
    bool ouvrir();
    void fermer();
    QString derniereErreur() const { return m_erreur; }

private:
    QString m_erreur;
};

#endif // CONNECTION_H
