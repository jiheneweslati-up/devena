#include "header.h"
#include "ui_mainwindow.h"
#include "connexion.h"
#include "evaluation.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <QDateTime>
#include <QRegularExpression>
#include <QStyle>
#include <algorithm>
#include <QtCharts/QChart>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>


// ===========================================================================
// Programme principal
// ===========================================================================
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    a.setStyleSheet(Theme::feuilleDeStyle());

    // On récupère l'instance unique du singleton
    Connection& c = Connection::createInstance();

    // On tente d'établir la connexion AVANT de créer la fenêtre
    // (le constructeur de la vue appelle afficher())
    bool test = c.createConnection();
    QString erreurOracle;
    if (!test) {
        erreurOracle = QSqlDatabase::database(QSqlDatabase::defaultConnection, false).lastError().text();
        c.createDemoConnection();   // secours : base de démonstration en mémoire
    }

    MainWindow W;
    W.show();

    //************************Test Connection BD**************************//
    if (test) {
        QMessageBox::information(&W, QObject::tr("Base de données"),
                                 QObject::tr("Connexion réussie."));
    } else {
        QMessageBox::warning(&W, QObject::tr("Base de données"),
                             QObject::tr("Échec de la connexion à Oracle.\n"
                                         "L'application fonctionne avec une base de démonstration "
                                         "(les modifications ne sont pas enregistrées dans Oracle).\n\n")
                                 + erreurOracle);
    }

    return a.exec();
}

// ===========================================================================
// Implémentation de la fenêtre principale (MainWindow)
// ===========================================================================
// =============================================================================
// === CONSTRUCTEUR & DESTRUCTEUR ==============================================
// =============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1400, 900);

    // ----- Navigation : un bouton = une page du stackedWidget (même ordre) -----
    const QList<QPushButton*> boutons = {
        ui->btnNavTableau, ui->btnNavEmployes, ui->btnNavParticipants, ui->btnNavEquipes,
        ui->btnNavProjets, ui->btnNavEvaluation, ui->btnNavParametres
    };
    m_navigation = new QButtonGroup(this);
    m_navigation->setExclusive(true);
    for (int i = 0; i < boutons.size(); ++i) {
        m_navigation->addButton(boutons[i], i);
        connect(boutons[i], &QPushButton::clicked, this, [this, i] { allerVersPage(i); });
    }

    // Pastille rouge sur la cloche de notifications
    auto *pastille = new QLabel(ui->btnNotifications);
    pastille->setFixedSize(8, 8);
    pastille->setStyleSheet("background:#e2574c; border-radius:4px;");
    pastille->move(25, 6);

    // ----- Initialisation des modules (chaque étudiant appelle la sienne) -----
    initModuleEvaluation();

    allerVersPage(ui->stackedPages->indexOf(ui->pageEvaluation));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// =============================================================================
// === SLOTS GÉNÉRAUX (NAVIGATION ENTRE PAGES) =================================
// =============================================================================
void MainWindow::allerVersPage(int index)
{
    ui->stackedPages->setCurrentIndex(index);
    if (QAbstractButton *b = m_navigation->button(index))
        b->setChecked(true);
}

// =============================================================================
// === MODULE TABLEAU DE BORD ==================================================
// =============================================================================
// 💡 Étudiant responsable du module Tableau de bord écrit ici

// =============================================================================
// === MODULE EMPLOYÉS ==========================================================
// =============================================================================
// 💡 Étudiant responsable du module Employés écrit ici

// =============================================================================
// === MODULE PARTICIPANTS ======================================================
// =============================================================================
// 💡 Étudiant responsable du module Participants écrit ici

// =============================================================================
// === MODULE ÉQUIPES ==========================================================
// =============================================================================
// 💡 Étudiant responsable du module Équipes écrit ici

// =============================================================================
// === MODULE PROJETS ==========================================================
// =============================================================================
// 💡 Étudiant responsable du module Projets écrit ici

// =============================================================================
// === MODULE EVALUATION =======================================================
// =============================================================================
// 💡 Étudiant responsable du module Évaluation travaille ici uniquement
//    Classe métier : evaluation.h / evaluation.cpp (aucune requête SQL ici)


// ===========================================================================
//  Outils internes (affichage uniquement, aucune requête)
// ===========================================================================
namespace {

const QColor EVAL_C_TEXTE("#EAF2FF");
const QColor EVAL_C_MUET("#7D8BA3");
const QColor EVAL_C_GRILLE("#15283B");

QLabel *evalIconeCarree(const QString &icone, const QString &couleur, int taille)
{
    auto *l = new QLabel(icone);
    l->setFixedSize(taille, taille);
    l->setAlignment(Qt::AlignCenter);
    l->setStyleSheet(QString("background:%1; border-radius:10px; font-size:%2px;")
                     .arg(couleur).arg(taille / 2));
    return l;
}

void evalRemplacerGraphique(QChartView *vue, QChart *nouveau)
{
    QChart *ancien = vue->chart();
    vue->setChart(nouveau);
    delete ancien;
}

QChart *evalNouveauGraphique()
{
    auto *chart = new QChart;
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    return chart;
}

QString evalEchapper(const QString &t) { return t.toHtmlEscaped(); }
QString evalOuTiret(const QString &t)  { return t.trimmed().isEmpty() ? QString::fromUtf8("—") : t; }
QString evalDate(const QDate &d)       { return d.isValid() ? d.toString("dd/MM/yyyy") : QString::fromUtf8("—"); }

} // namespace

// ===========================================================================
//  Initialisation du module (appelée par le constructeur)
// ===========================================================================
void MainWindow::initModuleEvaluation()
{
    // ----- Onglets (QStackedWidget, pas de fenêtres de dialogue) -----
    m_ongletsEvaluation = {ui->btnOngletListe, ui->btnOngletFormulaire, ui->btnOngletClassement};
    for (int i = 0; i < m_ongletsEvaluation.size(); ++i)
        connect(m_ongletsEvaluation[i], &QPushButton::clicked, this, [this, i] { changerOngletEvaluation(i); });

    preparerTableEvaluation(ui->tableEvaluations);
    preparerTableEvaluation(ui->tableClassement);
    preparerTableEvaluation(ui->tableAlertes);

    // ----- Zones remplies dynamiquement -----
    m_zoneLegendeEvaluation = new QWidget;
    auto *vl = new QVBoxLayout(m_zoneLegendeEvaluation);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(9);
    ui->layoutLegende->addWidget(m_zoneLegendeEvaluation);

    m_zoneRecentesEvaluation = new QWidget;
    auto *vr = new QVBoxLayout(m_zoneRecentesEvaluation);
    vr->setContentsMargins(0, 0, 0, 0);
    vr->setSpacing(12);
    ui->layoutRecentes->addWidget(m_zoneRecentesEvaluation);
    ui->layoutRecentes->addStretch();

    ui->lblCentreDonut->setAttribute(Qt::WA_TransparentForMouseEvents);

    // ----- Indicateur Oracle / démonstration -----
    if (Connection::estModeDemo()) {
        ui->lblMode->setText(QString::fromUtf8("● Base de démonstration"));
        ui->lblMode->setStyleSheet("color:#f5c242; font-size:12px; padding-top:8px;");
        ui->lblMode->setToolTip("Oracle indisponible : les données ne sont pas enregistrées dans Oracle.");
    } else {
        ui->lblMode->setText(QString::fromUtf8("● Oracle connecté"));
        ui->lblMode->setStyleSheet("color:#67F0B5; font-size:12px; padding-top:8px;");
    }

    // ----- Formulaire : mêmes limites que les colonnes de la base -----
    ui->leNom->setMaxLength(Evaluation::LONGUEUR_NOM);
    ui->cbType->setEditable(true);
    ui->cbType->lineEdit()->setMaxLength(Evaluation::LONGUEUR_TYPE);
    ui->dsbNote->setRange(0.0, Evaluation::NOTE_MAX);
    ui->dsbNote->setSingleStep(0.5);
    ui->deDate->setMaximumDate(QDate::currentDate());
    ui->deDate->setMinimumDate(QDate(2000, 1, 1));
    ui->deDate->setDate(QDate::currentDate());

    // ----- Signaux / slots -----
    connect(ui->leRecherche, &QLineEdit::textChanged, this, &MainWindow::actualiserEvaluations);
    connect(ui->cbTri,     QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::actualiserEvaluations);
    connect(ui->cbPeriode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::actualiserEvaluations);

    connect(ui->btnNouvelle,   &QPushButton::clicked, this, &MainWindow::nouvelleEvaluation);
    connect(ui->btnModifier,   &QPushButton::clicked, this, &MainWindow::modifierEvaluationSelectionnee);
    connect(ui->btnSupprimer,  &QPushButton::clicked, this, &MainWindow::supprimerEvaluationSelectionnee);
    connect(ui->btnFiche,      &QPushButton::clicked, this, &MainWindow::exporterFicheEvaluationPdf);
    connect(ui->btnPdf,        &QPushButton::clicked, this, &MainWindow::exporterEvaluationsPdf);
    connect(ui->btnPdfAlertes, &QPushButton::clicked, this, &MainWindow::exporterAlertesPdf);
    connect(ui->btnEnregistrer,&QPushButton::clicked, this, &MainWindow::enregistrerEvaluation);
    connect(ui->btnAnnuler,    &QPushButton::clicked, this, &MainWindow::annulerFormulaireEvaluation);
    connect(ui->tableEvaluations, &QTableView::doubleClicked, this, &MainWindow::modifierEvaluationSelectionnee);

    connect(ui->dsbNote, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::majEtoilesEvaluation);
    connect(ui->teRemarque, &QPlainTextEdit::textChanged, this, &MainWindow::majCompteurEvaluation);
    connect(ui->leNom, &QLineEdit::textChanged, this, [this] { marquerErreurEvaluation(ui->leNom, false); });
    connect(ui->cbType, &QComboBox::currentTextChanged, this, [this] { marquerErreurEvaluation(ui->cbType, false); });

    // ----- Affichage initial (appel de afficher(), comme dans l'atelier) -----
    viderFormulaireEvaluation();
    actualiserEvaluations();
    changerOngletEvaluation(0);
}

// ===========================================================================
//  Outils de la vue
// ===========================================================================
void MainWindow::preparerTableEvaluation(QTableView *t)
{
    t->verticalHeader()->hide();
    t->verticalHeader()->setDefaultSectionSize(46);
    t->setAlternatingRowColors(true);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setFocusPolicy(Qt::NoFocus);
    t->setShowGrid(false);
    t->setWordWrap(false);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    t->horizontalHeader()->setHighlightSections(false);
}

// Remplace le modèle d'une QTableView en libérant l'ancien (évite les fuites mémoire)
void MainWindow::remplacerModeleEvaluation(QTableView *table, QAbstractItemModel *modele)
{
    QAbstractItemModel  *ancienModele      = table->model();
    QItemSelectionModel *ancienneSelection = table->selectionModel();

    table->setModel(modele);
    delete ancienneSelection;
    delete ancienModele;

    QHeaderView *hh = table->horizontalHeader();
    hh->setSectionResizeMode(QHeaderView::ResizeToContents);
    hh->setStretchLastSection(false);
    if (table == ui->tableClassement && modele->columnCount() > 1)
        hh->setSectionResizeMode(1, QHeaderView::Stretch);                    // nom de l'équipe
    else if (modele->columnCount() > Evaluation::COL_REMARQUE)
        hh->setSectionResizeMode(Evaluation::COL_REMARQUE, QHeaderView::Stretch);

    if (table == ui->tableEvaluations) {
        connect(table->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MainWindow::majSelectionEvaluation);
    }
}

void MainWindow::viderLayoutEvaluation(QLayout *layout)
{
    if (!layout)
        return;
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *w = item->widget())
            delete w;
        if (QLayout *sous = item->layout()) {
            viderLayoutEvaluation(sous);
            delete sous;
        }
        delete item;
    }
}

QString MainWindow::ilYAEvaluation(const QDate &date)
{
    if (!date.isValid())
        return QString::fromUtf8("Sans date");
    const qint64 jours = date.daysTo(QDate::currentDate());
    if (jours <= 0) return "Aujourd'hui";
    if (jours == 1) return "Il y a 1 jour";
    if (jours < 30) return QString("Il y a %1 jours").arg(jours);
    return date.toString("dd/MM/yyyy");
}

QString MainWindow::idEvaluationSelectionnee() const
{
    QItemSelectionModel *sel = ui->tableEvaluations->selectionModel();
    if (!sel || sel->selectedRows().isEmpty())
        return QString();
    const int ligne = sel->selectedRows().first().row();
    return ui->tableEvaluations->model()->index(ligne, Evaluation::COL_ID).data(Qt::UserRole).toString();
}

QDate MainWindow::debutPeriodeEvaluation() const
{
    const QDate auj = QDate::currentDate();
    switch (ui->cbPeriode->currentIndex()) {
    case 1:  return QDate(auj.year(), auj.month(), 1);                          // ce mois-ci
    case 2:  return QDate(auj.year(), ((auj.month() - 1) / 3) * 3 + 1, 1);     // ce trimestre
    case 3:  return QDate(auj.year(), 1, 1);                                    // cette année
    default: return QDate();                                                    // toutes
    }
}

void MainWindow::changerOngletEvaluation(int index)
{
    if (index < 0 || index >= m_ongletsEvaluation.size())
        index = 0;
    for (int i = 0; i < m_ongletsEvaluation.size(); ++i)
        m_ongletsEvaluation[i]->setChecked(i == index);
    ui->stackPages->setCurrentIndex(index);
}

// ===========================================================================
//  READ : actualisation de l'affichage (après chaque requête qui modifie la table)
// ===========================================================================
void MainWindow::actualiserEvaluations()
{
    remplacerModeleEvaluation(ui->tableEvaluations,
                    Evtmp.afficher(ui->leRecherche->text(), ui->cbTri->currentIndex(), debutPeriodeEvaluation()));
    remplacerModeleEvaluation(ui->tableClassement, Evtmp.classementEquipes());

    QSqlQueryModel *modeleAlertes = Evtmp.alertes();
    remplacerModeleEvaluation(ui->tableAlertes, modeleAlertes);

    const int nbLignes  = ui->tableEvaluations->model()->rowCount();
    const int nbAlertes = modeleAlertes->rowCount();
    ui->lblTitreListe->setText(QString::fromUtf8("Tableau des évaluations (%1)").arg(nbLignes));
    ui->btnOngletClassement->setText(QString::fromUtf8("🏆   Classement && alertes (%1)").arg(nbAlertes));
    ui->lblSousAlertes->setText(nbAlertes == 0
        ? QString::fromUtf8("Aucune alerte : toutes les notes sont supérieures ou égales à 10/20. 🎉")
        : QString::fromUtf8("%1 évaluation(s) ont une note inférieure à 10/20 : un accompagnement est recommandé.")
              .arg(nbAlertes));
    ui->btnPdfAlertes->setEnabled(nbAlertes > 0);

    rafraichirStatistiquesEvaluation();
    majSelectionEvaluation();
}

void MainWindow::majSelectionEvaluation()
{
    const QString id = idEvaluationSelectionnee();
    const bool selection = !id.isEmpty();
    ui->btnModifier->setEnabled(selection);
    ui->btnSupprimer->setEnabled(selection);
    ui->btnFiche->setEnabled(selection);
    ui->lblSelection->setText(selection
        ? QString::fromUtf8("Évaluation %1 sélectionnée.").arg(id)
        : QString::fromUtf8("Sélectionnez une évaluation dans le tableau (double-clic pour la modifier)."));
}

// ===========================================================================
//  CREATE / UPDATE : formulaire intégré
// ===========================================================================
void MainWindow::remplirTypesEvaluation()
{
    const QString actuel = ui->cbType->currentText();
    ui->cbType->blockSignals(true);
    ui->cbType->clear();
    ui->cbType->addItems(Evaluation::listeTypes());
    ui->cbType->setCurrentText(actuel);
    ui->cbType->blockSignals(false);
}

void MainWindow::viderFormulaireEvaluation()
{
    m_idEvaluationEnEdition.clear();
    remplirTypesEvaluation();
    ui->leNom->clear();
    ui->cbType->setCurrentIndex(-1);
    ui->cbType->setCurrentText(QString());
    ui->dsbNote->setValue(14.0);
    ui->deDate->setDate(QDate::currentDate());
    ui->teRemarque->clear();
    ui->lblErreur->clear();
    ui->lblTitreFormulaire->setText(QString::fromUtf8("Nouvelle évaluation (%1)").arg(Evaluation::prochainId()));
    ui->btnEnregistrer->setText("Ajouter");
    for (QWidget *w : {static_cast<QWidget*>(ui->leNom), static_cast<QWidget*>(ui->cbType),
                       static_cast<QWidget*>(ui->deDate), static_cast<QWidget*>(ui->teRemarque)})
        marquerErreurEvaluation(w, false);
    majEtoilesEvaluation(ui->dsbNote->value());
    majCompteurEvaluation();
}

void MainWindow::nouvelleEvaluation()
{
    viderFormulaireEvaluation();
    changerOngletEvaluation(1);
    ui->leNom->setFocus();
}

void MainWindow::modifierEvaluationSelectionnee()
{
    const QString id = idEvaluationSelectionnee();
    if (id.isEmpty()) {
        QMessageBox::information(this, "Modifier", QString::fromUtf8("Sélectionnez d'abord une évaluation dans le tableau."));
        return;
    }

    Evaluation e;
    if (!e.chargerParId(id)) {
        QMessageBox::critical(this, "Erreur", QString::fromUtf8("Évaluation introuvable.\n") + Evaluation::derniereErreur());
        actualiserEvaluations();
        return;
    }

    viderFormulaireEvaluation();
    m_idEvaluationEnEdition = id;
    ui->leNom->setText(e.getNom());
    ui->cbType->setCurrentText(e.getType());
    ui->dsbNote->setValue(e.getNote());
    ui->deDate->setDate(e.getDateEvaluation().isValid() ? e.getDateEvaluation() : QDate::currentDate());
    ui->teRemarque->setPlainText(e.getRemarque());
    ui->lblTitreFormulaire->setText(QString::fromUtf8("Modifier l'évaluation %1").arg(id));
    ui->btnEnregistrer->setText("Enregistrer les modifications");
    changerOngletEvaluation(1);
}

void MainWindow::annulerFormulaireEvaluation()
{
    viderFormulaireEvaluation();
    changerOngletEvaluation(0);
}

void MainWindow::majEtoilesEvaluation(double note)
{
    ui->lblEtoilesForm->setText(EvaluationModel::etoiles(note) + QString("   %1 / 20").arg(QString::number(note, 'f', 1)));
    ui->lblEtoilesForm->setStyleSheet(note < EvaluationModel::SEUIL_ALERTE ? "color:#ff8a8a;" : QString());
}

void MainWindow::majCompteurEvaluation()
{
    const int n = ui->teRemarque->toPlainText().trimmed().length();
    ui->lblCompteur->setText(QString("%1 / %2").arg(n).arg(Evaluation::REMARQUE_MAX));
    marquerErreurEvaluation(ui->teRemarque, n > Evaluation::REMARQUE_MAX);
}

void MainWindow::marquerErreurEvaluation(QWidget *champ, bool enErreur)
{
    champ->setProperty("erreur", enErreur);
    champ->style()->unpolish(champ);
    champ->style()->polish(champ);
}

// Contrôle de saisie (obligatoire dans le code C++, cf. consignes de l'atelier)
bool MainWindow::controlerSaisieEvaluation()
{
    QStringList erreurs;

    const QString nom      = ui->leNom->text().trimmed();
    const QString type     = ui->cbType->currentText().trimmed();
    const double  note     = ui->dsbNote->value();
    const QDate   date     = ui->deDate->date();
    const QString remarque = ui->teRemarque->toPlainText().trimmed();

    static const QRegularExpression contientLettre("[A-Za-zÀ-ÿ]");
    static const QRegularExpression texteSimple("^[A-Za-zÀ-ÿ0-9 '\\-/&.]+$");

    const bool nomOk = !nom.isEmpty() && nom.toUtf8().size() <= Evaluation::LONGUEUR_NOM
                    && contientLettre.match(nom).hasMatch() && texteSimple.match(nom).hasMatch();
    marquerErreurEvaluation(ui->leNom, !nomOk);
    if (!nomOk)
        erreurs << QString::fromUtf8("• Le nom est obligatoire (20 octets max : une lettre accentuée compte double, au moins une lettre).");

    const bool typeOk = !type.isEmpty() && type.toUtf8().size() <= Evaluation::LONGUEUR_TYPE
                     && contientLettre.match(type).hasMatch() && texteSimple.match(type).hasMatch();
    marquerErreurEvaluation(ui->cbType, !typeOk);
    if (!typeOk)
        erreurs << QString::fromUtf8("• Le type d'évaluation est obligatoire (ex. : IA, Web, IoT), 20 octets max.");

    if (note < 0.0 || note > Evaluation::NOTE_MAX)
        erreurs << QString::fromUtf8("• La note doit être comprise entre 0 et 20.");

    const bool dateOk = date.isValid() && date <= QDate::currentDate() && date >= QDate(2000, 1, 1);
    marquerErreurEvaluation(ui->deDate, !dateOk);
    if (!dateOk)
        erreurs << QString::fromUtf8("• La date doit être valide et ne peut pas être dans le futur.");

    const bool remarqueOk = remarque.length() >= Evaluation::REMARQUE_MIN
                         && remarque.length() <= Evaluation::REMARQUE_MAX
                         && contientLettre.match(remarque).hasMatch();
    marquerErreurEvaluation(ui->teRemarque, !remarqueOk);
    if (!remarqueOk)
        erreurs << QString::fromUtf8("• La remarque doit contenir entre 3 et 200 caractères, avec au moins une lettre.");

    // Règle métier : pas deux évaluations identiques (même nom, même type, même date)
    if (erreurs.isEmpty()) {
        Evaluation test(m_idEvaluationEnEdition, note, remarque, date, nom, type);
        if (test.existeDeja())
            erreurs << QString::fromUtf8("• Une évaluation avec ce nom et ce type existe déjà à cette date.");
    }

    ui->lblErreur->setText(erreurs.join("\n"));
    return erreurs.isEmpty();
}

void MainWindow::enregistrerEvaluation()
{
    if (!controlerSaisieEvaluation())
        return;

    // Récupération des informations saisies + instanciation de l'objet Evaluation
    Evaluation E(m_idEvaluationEnEdition,
                 ui->dsbNote->value(),
                 ui->teRemarque->toPlainText().trimmed(),
                 ui->deDate->date(),
                 ui->leNom->text().trimmed(),
                 ui->cbType->currentText().trimmed());

    const bool ajout = m_idEvaluationEnEdition.isEmpty();
    const bool test  = ajout ? E.ajouter() : E.modifier();   // INSERT ou UPDATE dans la base

    if (test) {
        actualiserEvaluations();   // Refresh : ré-exécuter le SELECT
        QMessageBox::information(this, QObject::tr("OK"),
            ajout ? QObject::tr("Ajout effectué (%1).").arg(E.getId())
                  : QObject::tr("Modification effectuée."));
        viderFormulaireEvaluation();
        changerOngletEvaluation(0);
    } else {
        QMessageBox::critical(this, QObject::tr("Not OK"),
            (ajout ? QObject::tr("Ajout non effectué.\n") : QObject::tr("Modification non effectuée.\n"))
                + Evaluation::derniereErreur());
    }
}

// ===========================================================================
//  DELETE
// ===========================================================================
void MainWindow::supprimerEvaluationSelectionnee()
{
    const QString id = idEvaluationSelectionnee();
    if (id.isEmpty())
        return;

    if (QMessageBox::question(this, "Supprimer",
            QString::fromUtf8("Supprimer définitivement l'évaluation %1 ?").arg(id)) != QMessageBox::Yes)
        return;

    const bool test = Evtmp.supprimer(id);   // DELETE dans la base
    if (test) {
        actualiserEvaluations();
        QMessageBox::information(this, QObject::tr("OK"), QObject::tr("Suppression effectuée."));
    } else {
        QMessageBox::critical(this, QObject::tr("Not OK"),
                              QObject::tr("Suppression non effectuée.\n") + Evaluation::derniereErreur());
    }
}

// ===========================================================================
//  Statistiques
// ===========================================================================
void MainWindow::rafraichirStatistiquesEvaluation()
{
    const StatistiquesEvaluation s = Evtmp.statistiques();

    ui->lblValTotal->setText(QString::number(s.total));
    ui->lblTrendTotal->setText(QString::fromUtf8("↗ +%1 ce mois").arg(s.totalCeMois));
    ui->lblValEquipes->setText(QString::number(s.equipesEvaluees));
    ui->lblTrendEquipes->setText(QString::fromUtf8("sur %1 équipes").arg(s.totalEquipes));
    ui->lblValEmployes->setText(QString::number(s.evaluateurs));
    ui->lblTrendEmployes->setText(QString::fromUtf8("sur %1 employés").arg(s.totalEmployes));
    ui->lblValNote->setText(QString::number(s.moyenne, 'f', 1) + " / 20");
    ui->lblTrendNote->setText(EvaluationModel::etoiles(s.moyenne));
    ui->lblCentreDonut->setText(
        QString("<div align='center'><span style='font-size:22px; font-weight:700;'>%1</span><br>"
                "<span style='font-size:11px; color:#7D8BA3;'>Moyenne / 20</span></div>")
            .arg(QString::number(s.moyenne, 'f', 1)));

    majGraphiqueBarresEvaluation(s);
    majGraphiqueDonutEvaluation(s);
    majDernieresEvaluations();
}

void MainWindow::majGraphiqueBarresEvaluation(const StatistiquesEvaluation &s)
{
    auto *serie = new QBarSet("Note");
    QStringList categories;
    const int nb = std::min(5, static_cast<int>(s.noteParEquipe.size()));
    for (int i = 0; i < nb; ++i) {
        QString nom = s.noteParEquipe[i].first;
        if (nom.length() > 12)
            nom = nom.left(11) + QString::fromUtf8("…");
        categories << nom;
        *serie << qRound(s.noteParEquipe[i].second * 10.0) / 10.0;
    }
    serie->setColor(QColor("#18D9EF"));
    serie->setBorderColor(Qt::transparent);
    serie->setLabelColor(EVAL_C_TEXTE);

    auto *barres = new QBarSeries;
    barres->append(serie);
    barres->setBarWidth(0.55);
    barres->setLabelsVisible(true);
    barres->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    QChart *chart = evalNouveauGraphique();
    chart->addSeries(barres);

    auto *axeX = new QBarCategoryAxis;
    axeX->append(categories);
    axeX->setLabelsColor(EVAL_C_MUET);
    axeX->setGridLineVisible(false);
    axeX->setLinePenColor(EVAL_C_GRILLE);

    auto *axeY = new QValueAxis;
    axeY->setRange(0, Evaluation::NOTE_MAX);
    axeY->setTickCount(5);
    axeY->setLabelFormat("%d");
    axeY->setLabelsColor(EVAL_C_MUET);
    axeY->setGridLineColor(EVAL_C_GRILLE);
    axeY->setLinePenColor(EVAL_C_GRILLE);

    chart->addAxis(axeX, Qt::AlignBottom);
    chart->addAxis(axeY, Qt::AlignLeft);
    barres->attachAxis(axeX);
    barres->attachAxis(axeY);

    evalRemplacerGraphique(ui->chartBarres, chart);
}

void MainWindow::majGraphiqueDonutEvaluation(const StatistiquesEvaluation &s)
{
    struct Tranche { QString nom; QString couleur; int nb; };
    const QList<Tranche> tranches = {
        {QString::fromUtf8("Excellent (≥ 16)"),  "#67F0B5", s.tranches[0]},
        {QString::fromUtf8("Très bien (14 – 16)"), "#18D9EF", s.tranches[1]},
        {QString::fromUtf8("Passable (10 – 14)"), "#f5c242", s.tranches[2]},
        {QString::fromUtf8("Faible (< 10)"),      "#e2574c", s.tranches[3]},
    };

    auto *donut = new QPieSeries;
    donut->setHoleSize(0.60);
    donut->setPieSize(0.92);

    viderLayoutEvaluation(m_zoneLegendeEvaluation->layout());
    for (const Tranche &tr : tranches) {
        if (tr.nb > 0) {
            QPieSlice *slice = donut->append(tr.nom, tr.nb);
            slice->setColor(QColor(tr.couleur));
            slice->setBorderColor(QColor("#081421"));
            slice->setBorderWidth(2);
        }

        auto *ligne = new QWidget;
        auto *h = new QHBoxLayout(ligne);
        h->setContentsMargins(0, 0, 0, 0);
        auto *point = new QLabel(QString::fromUtf8("●"));
        point->setStyleSheet(QString("color:%1; font-size:14px;").arg(tr.couleur));
        auto *nom = new QLabel(tr.nom);
        nom->setStyleSheet("color:#DCE7F4; font-size:12px;");
        const int pct = s.total ? qRound(100.0 * tr.nb / s.total) : 0;
        auto *valeur = new QLabel(QString("%1  (%2 %)").arg(tr.nb).arg(pct));
        valeur->setProperty("role", "muted");
        h->addWidget(point);
        h->addWidget(nom);
        h->addStretch();
        h->addWidget(valeur);
        m_zoneLegendeEvaluation->layout()->addWidget(ligne);
    }

    QChart *chart = evalNouveauGraphique();
    chart->addSeries(donut);
    evalRemplacerGraphique(ui->chartDonut, chart);
}

void MainWindow::majDernieresEvaluations()
{
    viderLayoutEvaluation(m_zoneRecentesEvaluation->layout());

    const QList<ResumeEvaluation> recentes = Evtmp.dernieres(3);
    if (recentes.isEmpty()) {
        auto *vide = new QLabel(QString::fromUtf8("Aucune évaluation pour le moment."));
        vide->setProperty("role", "muted");
        m_zoneRecentesEvaluation->layout()->addWidget(vide);
        return;
    }

    for (const ResumeEvaluation &r : recentes) {
        auto *ligne = new QWidget;
        auto *h = new QHBoxLayout(ligne);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(12);
        h->addWidget(evalIconeCarree(QString::fromUtf8("📋"), "rgba(24,217,239,0.18)", 38));

        auto *textes = new QVBoxLayout;
        textes->setSpacing(1);
        auto *nom = new QLabel(r.id + "  " + (r.nom.isEmpty() ? QString() : QString::fromUtf8("· ") + r.nom));
        nom->setStyleSheet("font-weight:600;");
        auto *detail = new QLabel(QString::fromUtf8("%1 · %2 équipe(s)").arg(evalOuTiret(r.type)).arg(r.nbEquipes));
        detail->setProperty("role", "muted");
        textes->addWidget(nom);
        textes->addWidget(detail);

        auto *note = new QLabel(QString("<span style='color:%1;'>★</span> <b>%2</b>/20")
                                    .arg(r.note < EvaluationModel::SEUIL_ALERTE ? "#ff8a8a" : "#f5c242",
                                         QString::number(r.note, 'f', 1)));
        auto *quand = new QLabel(ilYAEvaluation(r.date));
        quand->setProperty("role", "muted");
        quand->setFixedWidth(90);
        quand->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        h->addLayout(textes);
        h->addStretch();
        h->addWidget(note);
        h->addWidget(quand);
        m_zoneRecentesEvaluation->layout()->addWidget(ligne);
    }
}

// ===========================================================================
//  Export PDF
// ===========================================================================
QString MainWindow::htmlListeEvaluations(const QString &titre, const QList<ResumeEvaluation> &lignes) const
{
    double somme = 0;
    for (const ResumeEvaluation &r : lignes)
        somme += r.note;
    const double moyenne = lignes.isEmpty() ? 0.0 : somme / lignes.size();

    QString html;
    html += "<html><body style='font-family:Arial; font-size:10pt; color:#182433;'>";
    html += "<table width='100%'><tr>"
            "<td><span style='font-size:20pt; font-weight:bold; color:#087F91;'>&#9670; Devena</span><br>"
            "<span style='color:#526579;'>Connecter les talents, construire l'avenir.</span></td>"
            "<td align='right' style='color:#526579;'>Généré le " +
            QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm") + "</td></tr></table><hr>";
    html += "<h2 style='color:#087F91;'>" + evalEchapper(titre) + "</h2>";
    html += QString("<p><b>%1</b> évaluation(s) &nbsp;|&nbsp; Note moyenne : <b>%2 / 20</b></p>")
                .arg(lignes.size()).arg(QString::number(moyenne, 'f', 1));
    html += "<table width='100%' cellspacing='0' cellpadding='6' border='1' style='border-collapse:collapse; border-color:#D5DEE8;'>"
            "<tr style='background-color:#087F91; color:white;'>"
            "<th>ID</th><th>Nom</th><th>Type d'évaluation</th><th>Note / 20</th><th>Remarque</th><th>Date d'évaluation</th><th>Équipes</th></tr>";
    for (int i = 0; i < lignes.size(); ++i) {
        const ResumeEvaluation &r = lignes[i];
        const bool alerte = r.note < EvaluationModel::SEUIL_ALERTE;
        html += QString("<tr style='background-color:%1;'>").arg(alerte ? "#fde8e8" : (i % 2 ? "#F1F6FA" : "#ffffff"));
        html += "<td>" + evalEchapper(r.id) + "</td>";
        html += "<td>" + evalEchapper(evalOuTiret(r.nom)) + "</td>";
        html += "<td>" + evalEchapper(evalOuTiret(r.type)) + "</td>";
        html += QString("<td><b style='color:%1;'>%2</b></td>")
                    .arg(alerte ? "#c0392b" : "#087F91", QString::number(r.note, 'f', 1));
        html += "<td>" + evalEchapper(r.remarque) + "</td>";
        html += "<td>" + evalDate(r.date) + "</td>";
        html += "<td align='center'>" + QString::number(r.nbEquipes) + "</td></tr>";
    }
    html += "</table><p style='color:#526579; font-size:8pt;'>Lignes en rouge : note inférieure à 10/20.</p>";
    html += "</body></html>";
    return html;
}

bool MainWindow::exporterHtmlEnPdfEvaluation(const QString &html, const QString &nomParDefaut)
{
    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString chemin = QFileDialog::getSaveFileName(this, "Enregistrer le PDF",
                                                  QDir(dossier).filePath(nomParDefaut),
                                                  "Fichiers PDF (*.pdf)");
    if (chemin.isEmpty())
        return false;
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive))
        chemin += ".pdf";

    QPdfWriter writer(chemin);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setTitle("Devena - Evaluations");
    writer.setCreator("Devena");

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&writer);

    if (QMessageBox::question(this, "Export PDF",
            QString::fromUtf8("PDF enregistré :\n%1\n\nVoulez-vous l'ouvrir ?").arg(chemin)) == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl::fromLocalFile(chemin));
    return true;
}

void MainWindow::exporterEvaluationsPdf()
{
    const QList<ResumeEvaluation> lignes =
        Evtmp.lister(ui->leRecherche->text(), ui->cbTri->currentIndex(), debutPeriodeEvaluation());
    if (lignes.isEmpty()) {
        QMessageBox::information(this, "Export PDF", QString::fromUtf8("Aucune évaluation à exporter."));
        return;
    }
    const QString titre = QString::fromUtf8("Liste des évaluations — %1").arg(ui->cbPeriode->currentText());
    exporterHtmlEnPdfEvaluation(htmlListeEvaluations(titre, lignes),
                      QString("evaluations_%1.pdf").arg(QDate::currentDate().toString("yyyyMMdd")));
}

void MainWindow::exporterAlertesPdf()
{
    QList<ResumeEvaluation> faibles;
    for (const ResumeEvaluation &r : Evtmp.lister(QString(), 2, QDate()))   // tri par note croissante
        if (r.note < EvaluationModel::SEUIL_ALERTE)
            faibles.append(r);
    if (faibles.isEmpty())
        return;
    exporterHtmlEnPdfEvaluation(htmlListeEvaluations(QString::fromUtf8("Évaluations à surveiller (note < 10/20)"), faibles),
                      QString("alertes_%1.pdf").arg(QDate::currentDate().toString("yyyyMMdd")));
}

void MainWindow::exporterFicheEvaluationPdf()
{
    const QString id = idEvaluationSelectionnee();
    if (id.isEmpty())
        return;

    ResumeEvaluation r;
    bool trouve = false;
    for (const ResumeEvaluation &x : Evtmp.lister(QString(), 0, QDate()))
        if (x.id == id) { r = x; trouve = true; break; }
    if (!trouve)
        return;

    const bool alerte = r.note < EvaluationModel::SEUIL_ALERTE;
    QString html;
    html += "<html><body style='font-family:Arial; font-size:11pt; color:#182433;'>";
    html += "<p><span style='font-size:20pt; font-weight:bold; color:#087F91;'>&#9670; Devena</span><br>"
            "<span style='color:#526579;'>Fiche d'évaluation</span></p><hr>";
    html += QString("<h1>%1 — %2</h1>").arg(evalEchapper(r.id), evalEchapper(evalOuTiret(r.nom)));
    html += QString("<p style='color:#526579;'>Type d'évaluation : %1 &nbsp;|&nbsp; Date d'évaluation : %2</p>")
                .arg(evalEchapper(evalOuTiret(r.type)), evalDate(r.date));
    html += "<table cellpadding='8' cellspacing='0' border='1' width='100%' style='border-collapse:collapse; border-color:#D5DEE8;'>";
    html += QString("<tr><td width='35%' style='background-color:#EAF1F7;'><b>Note</b></td>"
                    "<td><b style='font-size:16pt; color:%1;'>%2 / 20</b></td></tr>")
                .arg(alerte ? "#c0392b" : "#087F91", QString::number(r.note, 'f', 1));
    html += "<tr><td style='background-color:#EAF1F7;'><b>Remarque</b></td><td>" + evalEchapper(r.remarque) + "</td></tr>";
    html += QString("<tr><td style='background-color:#EAF1F7;'><b>Équipes évaluées</b></td><td>%1</td></tr>").arg(r.nbEquipes);
    html += "</table>";
    if (alerte)
        html += "<p style='color:#c0392b;'><b>&#9888; Alerte :</b> note inférieure à 10/20, un accompagnement est recommandé.</p>";
    html += "<p style='color:#526579; font-size:8pt;'>Généré le " +
            QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm") + "</p></body></html>";

    exporterHtmlEnPdfEvaluation(html, QString("fiche_evaluation_%1.pdf").arg(r.id));
}

// =============================================================================
// === FIN MODULE EVALUATION ===================================================
// =============================================================================
