#pragma once
#include <QWidget>
class QStackedWidget;
class QLineEdit;
class QLabel;
class QPushButton;
class EmployeWindow;
class EquipePage;
class EvaluationWindow;                     // module Evaluations (Amin)
namespace ModParticipants { class MainWindow; }   // module Participants (Nour)
class MainWindow;      // module Projets (Jihene)

class AppShell : public QWidget {
    Q_OBJECT
public:
    explicit AppShell(QWidget *parent = nullptr);
private slots:
    void verifierConnexion();
    void afficherEmployes();
    void afficherDashboard();
    void deconnecter();
    void allerVers(const QString &module);   // navigation entre les modules
private:
    QWidget *creerLoginPage();
    QWidget *creerDashboardPage();
    void     appliquerTheme();
    void     loadDashboardStats();   // runs Oracle queries and updates stat cards

    QStackedWidget *pages        = nullptr;
    QLineEdit      *emailEdit    = nullptr;   // used for ID_EMPLOYE input
    QLineEdit      *passwordEdit = nullptr;
    QLabel         *loginError   = nullptr;
    QLabel         *welcomeLabel = nullptr;
    EmployeWindow  *employePage  = nullptr;
    EquipePage     *equipesPage  = nullptr;   // module Equipes (Alaa), cree au premier clic
    MainWindow     *projetsPage  = nullptr;   // module Projets (Jihene), cree au premier clic
    ModParticipants::MainWindow *participantsPage = nullptr;   // module Participants (Nour)
    EvaluationWindow            *evaluationsPage  = nullptr;   // module Evaluations (Amin)

    // Dashboard stat-card value labels (updated after login)
    QLabel         *m_statTotal  = nullptr;   // COUNT(*) EMPLOYES
    QLabel         *m_statEV01   = nullptr;   // Jury IA
    QLabel         *m_statEV02   = nullptr;   // Jury Web
    QLabel         *m_statEquipe = nullptr;   // COUNT(*) EQUIPES
};
