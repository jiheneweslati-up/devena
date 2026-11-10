#include "SessionManager.h"

SessionManager& SessionManager::getInstance()
{
    static SessionManager instance;
    return instance;
}

void SessionManager::setEmployee(const QString& id, const QString& nom,
                                  const QString& prenom, const QString& poste)
{
    m_id       = id;
    m_nom      = nom;
    m_prenom   = prenom;
    m_poste    = poste;
    m_loggedIn = true;
}

void SessionManager::clear()
{
    m_id.clear();
    m_nom.clear();
    m_prenom.clear();
    m_poste.clear();
    m_loggedIn = false;
}

bool    SessionManager::isLoggedIn() const { return m_loggedIn; }
QString SessionManager::getId()      const { return m_id;       }
QString SessionManager::getNom()     const { return m_nom;      }
QString SessionManager::getPrenom()  const { return m_prenom;   }
QString SessionManager::getPoste()   const { return m_poste;    }

QString SessionManager::posteName() const
{
    if (m_poste == "EV01") return "Jury IA";
    if (m_poste == "EV02") return "Jury Web";
    if (m_poste == "EV03") return "Jury IoT";
    if (m_poste == "EV04") return "Jury Sécurité";
    return m_poste;   // fallback: return raw code
}
