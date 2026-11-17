/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *layoutCentral;
    QFrame *sidebar;
    QVBoxLayout *layoutSidebar;
    QLabel *labelLogo;
    QLabel *labelSlogan;
    QSpacerItem *espaceHautMenu;
    QPushButton *boutonTableauBord;
    QPushButton *boutonEmployee;
    QPushButton *boutonParticipants;
    QPushButton *boutonEquipes;
    QPushButton *boutonProjets;
    QPushButton *boutonEvaluations;
    QPushButton *boutonParametres;
    QSpacerItem *espaceBasMenu;
    QLabel *labelPied;
    QStackedWidget *stackedWidget;
    QWidget *pageProjets;
    QVBoxLayout *layoutPageProjets;
    QScrollArea *scrollArea;
    QWidget *contenuProjets;
    QVBoxLayout *layoutContenu;
    QHBoxLayout *layoutEntete;
    QVBoxLayout *layoutTitres;
    QLabel *labelTitrePage;
    QLabel *labelSousTitrePage;
    QSpacerItem *espaceEntete;
    QPushButton *boutonTheme;
    QPushButton *boutonExporterPdf;
    QPushButton *boutonAjouter;
    QHBoxLayout *layoutRecherche;
    QLineEdit *lineEditRecherche;
    QPushButton *boutonAnnuler;
    QLabel *labelTableau;
    QTableWidget *tableProjets;
    QLabel *labelStatistiques;
    QFrame *cadreGraphique;
    QVBoxLayout *layoutGraphique;
    QWidget *graphique;
    QHBoxLayout *layoutCartesStats;
    QFrame *carteProjets;
    QVBoxLayout *layoutCarteProjets;
    QLabel *labelTitreProjets;
    QLabel *labelNbProjets;
    QFrame *carteEnCours;
    QVBoxLayout *layoutCarteEnCours;
    QLabel *labelTitreEnCours;
    QLabel *labelNbEnCours;
    QFrame *carteTermines;
    QVBoxLayout *layoutCarteTermines;
    QLabel *labelTitreTermines;
    QLabel *labelNbTermines;
    QFrame *carteEnRetard;
    QVBoxLayout *layoutCarteEnRetard;
    QLabel *labelTitreEnRetard;
    QLabel *labelNbEnRetard;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1600, 900);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        layoutCentral = new QHBoxLayout(centralwidget);
        layoutCentral->setSpacing(0);
        layoutCentral->setObjectName("layoutCentral");
        layoutCentral->setContentsMargins(0, 0, 0, 0);
        sidebar = new QFrame(centralwidget);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(230, 0));
        sidebar->setMaximumSize(QSize(230, 16777215));
        layoutSidebar = new QVBoxLayout(sidebar);
        layoutSidebar->setSpacing(8);
        layoutSidebar->setObjectName("layoutSidebar");
        layoutSidebar->setContentsMargins(16, 22, 16, 18);
        labelLogo = new QLabel(sidebar);
        labelLogo->setObjectName("labelLogo");

        layoutSidebar->addWidget(labelLogo);

        labelSlogan = new QLabel(sidebar);
        labelSlogan->setObjectName("labelSlogan");

        layoutSidebar->addWidget(labelSlogan);

        espaceHautMenu = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);

        layoutSidebar->addItem(espaceHautMenu);

        boutonTableauBord = new QPushButton(sidebar);
        boutonTableauBord->setObjectName("boutonTableauBord");
        boutonTableauBord->setCheckable(true);

        layoutSidebar->addWidget(boutonTableauBord);

        boutonEmployee = new QPushButton(sidebar);
        boutonEmployee->setObjectName("boutonEmployee");
        boutonEmployee->setCheckable(true);

        layoutSidebar->addWidget(boutonEmployee);

        boutonParticipants = new QPushButton(sidebar);
        boutonParticipants->setObjectName("boutonParticipants");
        boutonParticipants->setCheckable(true);

        layoutSidebar->addWidget(boutonParticipants);

        boutonEquipes = new QPushButton(sidebar);
        boutonEquipes->setObjectName("boutonEquipes");
        boutonEquipes->setCheckable(true);

        layoutSidebar->addWidget(boutonEquipes);

        boutonProjets = new QPushButton(sidebar);
        boutonProjets->setObjectName("boutonProjets");
        boutonProjets->setCheckable(true);
        boutonProjets->setChecked(true);

        layoutSidebar->addWidget(boutonProjets);

        boutonEvaluations = new QPushButton(sidebar);
        boutonEvaluations->setObjectName("boutonEvaluations");
        boutonEvaluations->setCheckable(true);

        layoutSidebar->addWidget(boutonEvaluations);

        boutonParametres = new QPushButton(sidebar);
        boutonParametres->setObjectName("boutonParametres");
        boutonParametres->setCheckable(true);

        layoutSidebar->addWidget(boutonParametres);

        espaceBasMenu = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        layoutSidebar->addItem(espaceBasMenu);

        labelPied = new QLabel(sidebar);
        labelPied->setObjectName("labelPied");
        labelPied->setAlignment(Qt::AlignCenter);

        layoutSidebar->addWidget(labelPied);


        layoutCentral->addWidget(sidebar);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        pageProjets = new QWidget();
        pageProjets->setObjectName("pageProjets");
        layoutPageProjets = new QVBoxLayout(pageProjets);
        layoutPageProjets->setObjectName("layoutPageProjets");
        layoutPageProjets->setContentsMargins(0, 0, 0, 0);
        scrollArea = new QScrollArea(pageProjets);
        scrollArea->setObjectName("scrollArea");
        scrollArea->setWidgetResizable(true);
        contenuProjets = new QWidget();
        contenuProjets->setObjectName("contenuProjets");
        layoutContenu = new QVBoxLayout(contenuProjets);
        layoutContenu->setSpacing(14);
        layoutContenu->setObjectName("layoutContenu");
        layoutContenu->setContentsMargins(34, 22, 26, 22);
        layoutEntete = new QHBoxLayout();
        layoutEntete->setSpacing(10);
        layoutEntete->setObjectName("layoutEntete");
        layoutTitres = new QVBoxLayout();
        layoutTitres->setSpacing(2);
        layoutTitres->setObjectName("layoutTitres");
        labelTitrePage = new QLabel(contenuProjets);
        labelTitrePage->setObjectName("labelTitrePage");

        layoutTitres->addWidget(labelTitrePage);

        labelSousTitrePage = new QLabel(contenuProjets);
        labelSousTitrePage->setObjectName("labelSousTitrePage");

        layoutTitres->addWidget(labelSousTitrePage);


        layoutEntete->addLayout(layoutTitres);

        espaceEntete = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutEntete->addItem(espaceEntete);

        boutonTheme = new QPushButton(contenuProjets);
        boutonTheme->setObjectName("boutonTheme");

        layoutEntete->addWidget(boutonTheme);

        boutonExporterPdf = new QPushButton(contenuProjets);
        boutonExporterPdf->setObjectName("boutonExporterPdf");

        layoutEntete->addWidget(boutonExporterPdf);

        boutonAjouter = new QPushButton(contenuProjets);
        boutonAjouter->setObjectName("boutonAjouter");

        layoutEntete->addWidget(boutonAjouter);


        layoutContenu->addLayout(layoutEntete);

        layoutRecherche = new QHBoxLayout();
        layoutRecherche->setSpacing(20);
        layoutRecherche->setObjectName("layoutRecherche");
        lineEditRecherche = new QLineEdit(contenuProjets);
        lineEditRecherche->setObjectName("lineEditRecherche");
        lineEditRecherche->setClearButtonEnabled(true);

        layoutRecherche->addWidget(lineEditRecherche);

        boutonAnnuler = new QPushButton(contenuProjets);
        boutonAnnuler->setObjectName("boutonAnnuler");

        layoutRecherche->addWidget(boutonAnnuler);


        layoutContenu->addLayout(layoutRecherche);

        labelTableau = new QLabel(contenuProjets);
        labelTableau->setObjectName("labelTableau");

        layoutContenu->addWidget(labelTableau);

        tableProjets = new QTableWidget(contenuProjets);
        tableProjets->setObjectName("tableProjets");
        tableProjets->setMinimumSize(QSize(0, 330));

        layoutContenu->addWidget(tableProjets);

        labelStatistiques = new QLabel(contenuProjets);
        labelStatistiques->setObjectName("labelStatistiques");

        layoutContenu->addWidget(labelStatistiques);

        cadreGraphique = new QFrame(contenuProjets);
        cadreGraphique->setObjectName("cadreGraphique");
        layoutGraphique = new QVBoxLayout(cadreGraphique);
        layoutGraphique->setObjectName("layoutGraphique");
        graphique = new QWidget(cadreGraphique);
        graphique->setObjectName("graphique");
        graphique->setMinimumSize(QSize(0, 180));

        layoutGraphique->addWidget(graphique);


        layoutContenu->addWidget(cadreGraphique);

        layoutCartesStats = new QHBoxLayout();
        layoutCartesStats->setSpacing(14);
        layoutCartesStats->setObjectName("layoutCartesStats");
        carteProjets = new QFrame(contenuProjets);
        carteProjets->setObjectName("carteProjets");
        layoutCarteProjets = new QVBoxLayout(carteProjets);
        layoutCarteProjets->setObjectName("layoutCarteProjets");
        labelTitreProjets = new QLabel(carteProjets);
        labelTitreProjets->setObjectName("labelTitreProjets");

        layoutCarteProjets->addWidget(labelTitreProjets);

        labelNbProjets = new QLabel(carteProjets);
        labelNbProjets->setObjectName("labelNbProjets");

        layoutCarteProjets->addWidget(labelNbProjets);


        layoutCartesStats->addWidget(carteProjets);

        carteEnCours = new QFrame(contenuProjets);
        carteEnCours->setObjectName("carteEnCours");
        layoutCarteEnCours = new QVBoxLayout(carteEnCours);
        layoutCarteEnCours->setObjectName("layoutCarteEnCours");
        labelTitreEnCours = new QLabel(carteEnCours);
        labelTitreEnCours->setObjectName("labelTitreEnCours");

        layoutCarteEnCours->addWidget(labelTitreEnCours);

        labelNbEnCours = new QLabel(carteEnCours);
        labelNbEnCours->setObjectName("labelNbEnCours");

        layoutCarteEnCours->addWidget(labelNbEnCours);


        layoutCartesStats->addWidget(carteEnCours);

        carteTermines = new QFrame(contenuProjets);
        carteTermines->setObjectName("carteTermines");
        layoutCarteTermines = new QVBoxLayout(carteTermines);
        layoutCarteTermines->setObjectName("layoutCarteTermines");
        labelTitreTermines = new QLabel(carteTermines);
        labelTitreTermines->setObjectName("labelTitreTermines");

        layoutCarteTermines->addWidget(labelTitreTermines);

        labelNbTermines = new QLabel(carteTermines);
        labelNbTermines->setObjectName("labelNbTermines");

        layoutCarteTermines->addWidget(labelNbTermines);


        layoutCartesStats->addWidget(carteTermines);

        carteEnRetard = new QFrame(contenuProjets);
        carteEnRetard->setObjectName("carteEnRetard");
        layoutCarteEnRetard = new QVBoxLayout(carteEnRetard);
        layoutCarteEnRetard->setObjectName("layoutCarteEnRetard");
        labelTitreEnRetard = new QLabel(carteEnRetard);
        labelTitreEnRetard->setObjectName("labelTitreEnRetard");

        layoutCarteEnRetard->addWidget(labelTitreEnRetard);

        labelNbEnRetard = new QLabel(carteEnRetard);
        labelNbEnRetard->setObjectName("labelNbEnRetard");

        layoutCarteEnRetard->addWidget(labelNbEnRetard);


        layoutCartesStats->addWidget(carteEnRetard);


        layoutContenu->addLayout(layoutCartesStats);

        scrollArea->setWidget(contenuProjets);

        layoutPageProjets->addWidget(scrollArea);

        stackedWidget->addWidget(pageProjets);

        layoutCentral->addWidget(stackedWidget);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Devena", nullptr));
        labelLogo->setText(QCoreApplication::translate("MainWindow", "\342\227\206 Devena", nullptr));
        labelSlogan->setText(QCoreApplication::translate("MainWindow", "Connecter les talents,\n"
"construire l'avenir", nullptr));
        boutonTableauBord->setText(QCoreApplication::translate("MainWindow", "\342\211\241   Tableau de bord", nullptr));
        boutonTableauBord->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "menu", nullptr)));
        boutonEmployee->setText(QCoreApplication::translate("MainWindow", "\342\231\231   Employee", nullptr));
        boutonEmployee->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "menu", nullptr)));
        boutonParticipants->setText(QCoreApplication::translate("MainWindow", "\342\231\246   Participants", nullptr));
        boutonParticipants->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "menu", nullptr)));
        boutonEquipes->setText(QCoreApplication::translate("MainWindow", "\342\231\231   Equipes", nullptr));
        boutonEquipes->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "menu", nullptr)));
        boutonProjets->setText(QCoreApplication::translate("MainWindow", "\342\214\230   Projets", nullptr));
        boutonProjets->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "menu", nullptr)));
        boutonEvaluations->setText(QCoreApplication::translate("MainWindow", "\342\230\205   Evaluations", nullptr));
        boutonEvaluations->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "menu", nullptr)));
        boutonParametres->setText(QCoreApplication::translate("MainWindow", "\342\232\231   Param\303\250tres", nullptr));
        boutonParametres->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "menu", nullptr)));
        labelPied->setText(QCoreApplication::translate("MainWindow", "Hack today\n"
"Build tomorrow", nullptr));
        labelTitrePage->setText(QCoreApplication::translate("MainWindow", "Projets", nullptr));
        labelSousTitrePage->setText(QCoreApplication::translate("MainWindow", "G\303\251rez les projets du hackathon", nullptr));
        labelSousTitrePage->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "sousTitre", nullptr)));
        boutonTheme->setText(QCoreApplication::translate("MainWindow", "\342\230\274  Mode clair", nullptr));
        boutonExporterPdf->setText(QCoreApplication::translate("MainWindow", "Exporter en PDF", nullptr));
        boutonAjouter->setText(QCoreApplication::translate("MainWindow", "+ Ajouter un projet", nullptr));
        boutonAjouter->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primaire", nullptr)));
        lineEditRecherche->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher un projet", nullptr));
        boutonAnnuler->setText(QCoreApplication::translate("MainWindow", "\342\206\266  Annuler (Ctrl+Z)", nullptr));
        boutonAnnuler->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "lien", nullptr)));
        labelTableau->setText(QCoreApplication::translate("MainWindow", "Tableau des projets", nullptr));
        labelTableau->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "section", nullptr)));
        labelStatistiques->setText(QCoreApplication::translate("MainWindow", "Statistiques", nullptr));
        labelStatistiques->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "section", nullptr)));
        cadreGraphique->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "carte", nullptr)));
        carteProjets->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "carte", nullptr)));
        labelTitreProjets->setText(QCoreApplication::translate("MainWindow", "Projets", nullptr));
        labelTitreProjets->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "sousTitre", nullptr)));
        labelNbProjets->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        labelNbProjets->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "valeurStat", nullptr)));
        carteEnCours->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "carte", nullptr)));
        labelTitreEnCours->setText(QCoreApplication::translate("MainWindow", "En cours", nullptr));
        labelTitreEnCours->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "sousTitre", nullptr)));
        labelNbEnCours->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        labelNbEnCours->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "valeurStat", nullptr)));
        carteTermines->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "carte", nullptr)));
        labelTitreTermines->setText(QCoreApplication::translate("MainWindow", "Termin\303\251s", nullptr));
        labelTitreTermines->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "sousTitre", nullptr)));
        labelNbTermines->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        labelNbTermines->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "valeurStat", nullptr)));
        carteEnRetard->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "carte", nullptr)));
        labelTitreEnRetard->setText(QCoreApplication::translate("MainWindow", "En retard", nullptr));
        labelTitreEnRetard->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "sousTitre", nullptr)));
        labelNbEnRetard->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        labelNbEnRetard->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "valeurStat", nullptr)));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
