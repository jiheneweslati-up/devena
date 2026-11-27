#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "equipe.h"

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <QDate>
#include <QPushButton>
#include <QComboBox>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void creerInterface();

    int prochainIdEquipe() const;
    int prochainIdParticipant() const;

    int meilleureEquipe(const Participant &p) const;

    void affecterAutomatiquement();

    void ajouterEquipe();
    void modifierEquipe(int index);
    void supprimerEquipe(int index);
    void ajouterMembre(bool automatique, int indexEquipe = -1);

    void voirEquipe(int index);

    QImage creerQR(const QString &code) const;
    void afficherQR(int index);

    QImage creerBadge(const Equipe &e,
                      const QString &membres,
                      const QString &hommes,
                      const QString &femmes) const;
    void afficherBadge(int index);

    void afficherEquipes();
    void afficherOrganigramme();

    QPixmap creerCourbe() const;
    QWidget *m_organigramme;
    void filtrer();
    void afficherNotification(
        const QString &texte,
        bool succes = true
        );

    void rafraichir();
    void exporterPDF();
    void exporterEquipePDF(int index); // Exporter une seule équipe avec son QR Code
    void changerModeClair();

private:
    class QTableWidget *m_table;
    class QLineEdit *m_recherche;
    class QComboBox *m_filtre;
    QComboBox *m_tri = nullptr;
    QPushButton *m_ordreTri = nullptr;

    bool m_triCroissant = true;

    void trierEquipes();
    class QLabel *m_notification;
    class QLabel *m_courbe;

    class QLabel *m_totalEquipes;
    class QLabel *m_totalMembres;
    class QLabel *m_totalHommes;
    class QLabel *m_totalFemmes;

    class QWidget *m_side;
    Equipe Etmp;

    QVector<Equipe> m_equipes;
    QVector<Participant> m_participants;

    bool m_modeClair = false;
};

#endif // MAINWINDOW_H