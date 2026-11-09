#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColor>
#include <QList>
#include <QMainWindow>
#include <QPair>

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
    // Sert a dessiner la courbe des statistiques dans ui->graphique
    bool eventFilter(QObject *objet, QEvent *evenement) override;

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

    // Theme
    Couleurs couleurs() const;
    QString feuilleDeStyle() const;
    void appliquerTheme();

    Ui::MainWindow *ui;
    QList<Projet> m_projetsAffiches;
    QList<Action> m_historique;
    QList<QPair<QString, int>> m_statsThematique;   // points de la courbe
    bool m_sombre = true;
};

#endif // MAINWINDOW_H
