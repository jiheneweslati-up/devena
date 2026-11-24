#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColor>
#include <QDateTime>
#include <QList>
#include <QMainWindow>
#include <QPair>
#include <QPointer>

#include <functional>

class QComboBox;
class QFrame;
class QLabel;
class QPushButton;
class QStackedWidget;
class QVariantAnimation;
class QVBoxLayout;

#include "projet.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // Dessine le donut des statistiques et les confettis, gere les clics et le survol des notifications
    bool eventFilter(QObject *objet, QEvent *evenement) override;
    // Replace le centre de notifications et les toasts quand la fenetre change de taille
    void resizeEvent(QResizeEvent *evenement) override;

private slots:
    // Boutons CRUD (appellent les fonctions de Projet)
    void ajouterProjet();
    void voirProjet(const QString &id);
    void modifierProjet(const QString &id);
    void supprimerProjet(const QString &id);
    void rafraichir();

    // Autres fonctionnalites
    void annulerDerniereAction();
    void exporterPdf();                       // liste des projets affiches (sans les fiches)
    void exporterFichePdf(const QString &id); // fiche PDF d'un seul projet
    void afficherProjetGagnant();             // bouton Evaluation : classement par note et gagnant
    void basculerTheme();

private:
    // Une action enregistree pour pouvoir l'annuler avec Ctrl+Z
    struct Action {
        enum Type { Ajout, Modification, Suppression } type;
        Projet avant;   // etat avant l'action (Modification, Suppression)
        Projet apres;   // etat apres l'action (Ajout)
    };

    // Note d'un projet : moyenne des notes (table EVALUATION) de ses equipes
    struct ScoreProjet {
        QString id;
        QString titre;
        QString thematique;
        double moyenne = 0;
        double meilleure = 0;
        int nbEvaluations = 0;
    };

    // Couleurs du theme courant (sombre ou clair)
    struct Couleurs {
        QColor fond, panneau, carte, bordure, texte, discret, accent, accentTexte, badge, danger;
    };

    // Interface
    void construireMenu();
    void configurerTableau();
    void ajusterColonnes();                // largeurs des colonnes selon la place disponible
    void remplirTableau(const QList<Projet> &projets);
    void trierProjets(QList<Projet> &projets) const;       // selon la colonne choisie
    void mettreAJourEntetes();                             // fleche ▲ ▼ sur la colonne triee
    bool ouvrirFormulaire(Projet &p, bool modification, bool lectureSeule = false);   // fenetre Ajouter / Modifier
    QString prochainId() const;
    void empiler(const Action &a);

    // Statistiques (organigramme de 4 cartes)
    void mettreAJourStatistiques(bool forcer = false);
    void dessinerDonut(QWidget *zone);

    // Evaluation : classement des projets par note
    QList<ScoreProjet> classementProjets(QString *erreur = nullptr) const;
    void dessinerConfettis(QWidget *zone);

    // PDF : fiches des projets
    bool ecrirePdfProjets(const QList<Projet> &projets, const QString &chemin, bool listeSeule);

    // Animations
    void animerPage(QWidget *page);                    // fondu quand on ouvre une page
    void animerCompteur(QLabel *label, int valeur);    // les chiffres defilent jusqu'a la valeur
    void animerApparition(QWidget *fenetre, bool glisser = true);   // fondu (et glissement) d'une fenetre
    void animerLigne(const QString &id);               // la ligne modifiee s'illumine puis s'eteint
    void animerEntreeLignes();                         // les lignes apparaissent l'une apres l'autre
    void animerSuppression(const QString &id, const std::function<void()> &ensuite);   // balayage rouge
    void animerBarres();                               // les barres et le donut se remplissent
    void animerBoutonAjouter();                        // halo qui pulse autour de "+ Ajouter"


    // ===== Vue Kanban (glisser deposer pour changer le statut) =====
    void construireKanban();                                       // colonnes + boutons Tableau / Kanban
    void afficherVue(bool kanban, bool animer = true);             // tableau ou Kanban
    void remplirKanban();                                          // une carte par projet affiche
    void changerStatut(const QString &id, const QString &statut);  // UPDATE apres un depot
    bool gererKanban(QObject *objet, QEvent *evenement);           // souris et glisser deposer

    // ===== Systeme de notifications =====
    struct Notification {
        enum Type { Succes, Info, Alerte, Erreur, Suppression };
        Type type = Info;
        QString titre;
        QString message;
        QString idProjet;    // projet concerne (vide si aucun) : un clic y emmene
        QDateTime date;
        bool lue = false;
    };

    // Ajoute une notification a l'historique et, si demande, l'affiche en toast.
    // libelleAction / action : bouton optionnel dans le toast ("Annuler", "Ouvrir"...)
    void notifier(Notification::Type type, const QString &titre, const QString &message,
                  const QString &idProjet = QString(), bool afficher = true,
                  const QString &libelleAction = QString(),
                  const std::function<void()> &action = {});
    void creerCentreNotifications();       // cloche, compteur et panneau lateral
    void afficherToast(const Notification &n, const QString &libelleAction = QString(),
                       const std::function<void()> &action = {});
    void fermerToast(QFrame *toast);
    void placerToasts(QFrame *nouveau = nullptr);   // empile les toasts (anime)
    void ouvrirPanneau();
    void fermerPanneau();
    void remplirPanneau();                 // liste de l'historique, selon le filtre
    void mettreAJourBadge(bool animer = false);
    void allerAuProjet(const QString &id); // affiche et illumine la ligne du projet
    void verifierEcheances();              // alertes : dates limites depassees ou proches
    void chargerNotifications();           // historique sauvegarde entre deux lancements
    void sauverNotifications() const;
    std::function<void()> actionAnnuler(); // bouton "Annuler" des toasts
    QString styleNotifications() const;
    static QColor couleurType(Notification::Type type, const Couleurs &c);
    static QString iconeType(Notification::Type type);
    static QString tempsRelatif(const QDateTime &date);

    // Theme
    Couleurs couleurs() const;
    QString feuilleDeStyle() const;
    void appliquerTheme();

    Ui::MainWindow *ui;
    QList<Projet> m_projetsAffiches;
    QList<Action> m_historique;
    bool m_suppressionEnCours = false;

    // Kanban
    QStackedWidget *m_vues = nullptr;          // tableau ou Kanban
    QWidget *m_kanban = nullptr;
    QList<QVBoxLayout *> m_colonnesKanban;     // cartes de chaque colonne (ordre de STATUTS)

    // Tri du tableau
    int m_triColonne = 0;          // colonne triee (0 = ID Projet)
    bool m_triCroissant = true;

    // Statistiques
    QString m_signatureStats;                       // pour ne reconstruire que si les donnees changent
    QList<QPair<QString, int>> m_partsDonut;        // statut -> nombre de projets
    qreal m_progressionDonut = 1.0;                 // 0 a 1 : animation du donut

    // Notifications
    QList<Notification> m_notifications;   // la plus recente en premier
    QList<QPointer<QFrame>> m_toasts;      // toasts affiches, du haut vers le bas
    QPushButton *m_boutonNotifs = nullptr;
    QLabel *m_badgeNotifs = nullptr;
    QWidget *m_voile = nullptr;            // fond assombri derriere le panneau
    QFrame *m_panneau = nullptr;
    QVBoxLayout *m_listeNotifs = nullptr;
    QComboBox *m_filtreNotifs = nullptr;
    QLabel *m_resumePanneau = nullptr;
    bool m_panneauOuvert = false;
    bool m_sombre = true;
};

#endif // MAINWINDOW_H
