#ifndef CONNEXION_H
#define CONNEXION_H

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>

/* Classe Connection
 * Implémentation du pattern Singleton pour gérer une connexion unique à la base de données.
 * Utilise QSqlDatabase pour établir et fermer la connexion.
 * L'instance est accessible via la méthode statique createInstance().
 * La méthode createConnection() configure et ouvre la base de données.
 */

class Connection {
public:

    /******Méthodes publiques*****/
    /****************************/

    static Connection& createInstance(); // Méthode qui retourne une référence unique
    bool createConnection();             // Méthode utilisée pour établir la connexion à la base

    // Secours : base SQLite en mémoire (mêmes tables) si Oracle est indisponible
    bool createDemoConnection();
    static bool estModeDemo();

private:

    // attribut Connexion persistante: attribut d'une instance qui vit longtemps (le Singleton)
    QSqlDatabase db;

    /******Méthodes privées*****/
    /**************************/

    Connection();  // Constructeur privé pour éviter la création d'autres instances
    ~Connection(); // Destructeur privé pour fermer proprement la connexion

    // Interdire la copie & l'affectation ==> garantir l'unicité de l'instance de connexion
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};


// ===========================================================================
// Base de démonstration (secours si Oracle est indisponible)
// ===========================================================================
// ---------------------------------------------------------------------------
//  Base de démonstration (SQLite en mémoire)
//  Utilisée UNIQUEMENT si la connexion Oracle échoue, pour pouvoir tester
//  l'application. Elle reprend les tables de la base commune (mêmes noms de
//  tables et de colonnes) avec quelques données de test.
// ---------------------------------------------------------------------------
namespace BaseDemo {
bool initialiser();
}


#endif // CONNEXION_H
