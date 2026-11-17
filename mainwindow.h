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
    // Dessine la courbe des statistiques, gere les clics et le survol des notifications
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
    void exporterPdf();
    void basculerTheme();

private:
    // Une action enregistree pour pouvoir l'annuler avec Ctrl+Z
    struct Action {
        enum Type { Ajout, Modification, Suppression } type;
        Projet avant;   // etat avant l'action (Modification, Suppression)
        Projet apres;   // etat apres l'action (Ajout)
    };

    // Couleurs du theme courant (sombre ou clair)
    struct Couleurs {
        QColor fond, panneau, carte, bordure, texte, discret, accent, accentTexte, badge, danger;
    };

    // Interface
    void construireMenu();
    void configurerTableau();
    void remplirTableau(const QList<Projet> &projets);
    bool ouvrirFormulaire(Projet &p, bool modification);   // fenetre Ajouter / Modifier
    QString prochainId() const;
    void empiler(const Action &a);

    // Statistiques
    void mettreAJourStatistiques();
    void dessinerGraphique();

    // Animations
    void animerPage(QWidget *page);                    // fondu quand on ouvre une page
    void animerCompteur(QLabel *label, int valeur);    // les chiffres defilent jusqu'a la valeur
    void animerGraphique();                            // la courbe se dessine de gauche a droite
    void animerApparition(QWidget *fenetre, bool glisser = true);   // fondu (et glissement) d'une fenetre
    void animerLigne(const QString &id);               // la ligne modifiee s'illumine puis s'eteint
    void animerEntreeLignes();                         // les lignes apparaissent l'une apres l'autre
    void animerSuppression(const QString &id, const std::function<void()> &ensuite);   // balayage rouge
    void animerCartes();                               // les cartes s'allument l'une apres l'autre
    void animerBoutonAjouter();                        // halo qui pulse autour de "+ Ajouter"


    // ===== QR codes et flyers =====
    void afficherQRProjet(const QString &id);              // fenetre QR du projet
    QString contenuQR(const Projet &p) const;              // texte encode dans le QR
    bool exporterFlyers(const QList<Projet> &projets,      // un flyer A4 par projet (PDF)
                        const QString &chemin);

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
    QList<QPair<QString, int>> m_statsThematique;   // points de la courbe
    qreal m_progressionCourbe = 1.0;                  // 0 = rien de dessine, 1 = courbe complete
    QVariantAnimation *m_animationCourbe = nullptr;
    QList<int> m_chiffres;                            // derniers chiffres des cartes
    bool m_suppressionEnCours = false;

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
