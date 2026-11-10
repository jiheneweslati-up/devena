#ifndef EQUIPEPAGE_H
#define EQUIPEPAGE_H

#include "equipe.h"

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <QDate>

class EquipePage : public QMainWindow
{
    Q_OBJECT

public:
    explicit EquipePage(QWidget *parent = nullptr);
    ~EquipePage() override;

signals:
    // Integration : clic sur un autre module dans le menu de gauche
    void naviguer(const QString &module);

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

    QImage creerBadge(const Equipe &e) const;
    void afficherBadge(int index);

    void afficherEquipes();

    QPixmap creerCourbe() const;
    void afficherStatistiques();

    void filtrer();

    void afficherNotification(
        const QString &texte,
        bool succes = true
        );

    void rafraichir();

    void exporterPDF();
    void changerModeClair();

private:
    class QTableWidget *m_table;
    class QLineEdit *m_recherche;
    class QComboBox *m_filtre;
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

#endif // EQUIPEPAGE_H