#pragma once
#include <QString>

/**
 * SessionManager – Meyers Singleton
 * Stores the currently logged-in employee for the lifetime of the session.
 */
class SessionManager {
public:
    static SessionManager& getInstance();

    void setEmployee(const QString& id, const QString& nom,
                     const QString& prenom, const QString& poste);
    void clear();

    bool    isLoggedIn() const;
    QString getId()      const;
    QString getNom()     const;
    QString getPrenom()  const;
    QString getPoste()   const;

    /** Returns the human-readable jury name for the stored POSTE code. */
    QString posteName()  const;

private:
    SessionManager() = default;
    ~SessionManager()  = default;
    SessionManager(const SessionManager&)            = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    QString m_id;
    QString m_nom;
    QString m_prenom;
    QString m_poste;
    bool    m_loggedIn = false;
};
