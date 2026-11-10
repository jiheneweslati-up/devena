#ifndef EVALUATIONWINDOW_H
#define EVALUATIONWINDOW_H


// ===========================================================================
// Charte graphique (thème commun de l'équipe)
// ===========================================================================
#include <QString>

// ---------------------------------------------------------------------------
//  Charte graphique Devena — thème commun de l'équipe (sombre, accent cyan)
//
//  Les widgets des fichiers .ui portent une propriété dynamique « role »
//  (Qt Designer : bouton « + » de l'éditeur de propriétés > Chaîne > role).
//  Exemple : un QFrame avec role = card prend automatiquement le style carte.
// ---------------------------------------------------------------------------
namespace EvalTheme {

inline QString feuilleDeStyle()
{
    // Palette commune de l'équipe (même thème que le module Équipes)
    //   fond #050B14 · barre latérale #030914 · cartes #091522
    //   accent #18D9EF · texte #EAF2FF · texte secondaire #7D8BA3
    return QString::fromUtf8(R"(
        * {
            font-family: "Segoe UI", "Segoe UI Emoji", Arial;
            font-size: 13px;
            color: #EAF2FF;
        }

        QMainWindow, QWidget#central { background: #050B14; }
        QDialog, QMessageBox         { background: #091522; }
        QToolTip { background: #091522; color: #EAF2FF; border: 1px solid #243A52; padding: 4px; }

        /* ---------- Barre latérale (220 px) ---------- */
        QFrame[role="sidebar"] { background: #030914; border-right: 1px solid #17283A; }
        QLabel[role="logo"]    { font-size: 27px; font-weight: bold; color: #18D9EF; }
        QLabel[role="slogan"]  { color: #718198; font-size: 13px; }
        QLabel[role="footer"]  { color: #3A4D65; font-size: 13px; }

        QPushButton[role="nav"] {
            text-align: left; padding: 12px; border: 1px solid transparent; border-radius: 8px;
            font-size: 13px; font-weight: bold; color: #718198; background: transparent;
        }
        QPushButton[role="nav"]:hover   { background: #0B1726; color: #DCE7F4; }
        QPushButton[role="nav"]:checked { background: #092A3D; color: #18D9EF; border: 1px solid #18D9EF; }

        /* ---------- Barre du haut ---------- */
        QFrame[role="topbar"] { background: #050B14; border-bottom: 1px solid #17283A; }
        QPushButton[role="iconBtn"] { background: transparent; border: none; font-size: 18px; padding: 2px; }
        QPushButton[role="iconBtn"]:hover { background: #10283D; border-radius: 8px; }
        QLabel[role="avatar"] {
            background: #18D9EF; color: #03212A; border-radius: 20px;
            font-weight: bold; font-size: 16px;
        }

        /* ---------- Cartes et textes ---------- */
        QFrame[role="card"] { background: #091522; border: 1px solid #1C3045; border-radius: 10px; }
        QLabel[role="pageTitle"]    { font-size: 30px; font-weight: bold; color: #FFFFFF; }
        QLabel[role="sectionTitle"] { font-size: 19px; font-weight: bold; color: #FFFFFF; }
        QLabel[role="muted"]        { color: #7D8BA3; font-size: 12px; }
        QLabel[role="statValue"]    { font-size: 21px; font-weight: bold; color: #18D9EF; }
        QLabel[role="statLabel"]    { color: #7D8BA3; font-size: 13px; }
        QLabel[role="statTrend"]    { color: #67F0B5; font-size: 12px; }
        QLabel[role="stars"]        { color: #f5c242; font-size: 17px; }

        /* ---------- Champs ---------- */
        QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit, QPlainTextEdit {
            background: #091522; border: 1px solid #243A52; border-radius: 8px;
            padding: 9px; color: #FFFFFF; font-size: 14px;
        }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QPlainTextEdit:focus {
            border: 1px solid #18D9EF;
        }
        QLineEdit[erreur="true"], QComboBox[erreur="true"], QDateEdit[erreur="true"], QPlainTextEdit[erreur="true"] { border: 1px solid #C44858; }
        QLabel[role="erreur"] { color: #FF8C99; font-size: 12px; }
        QLabel[role="champ"]  { color: #DCE7F4; font-weight: bold; }

        /* ---------- Boutons ---------- */
        QPushButton {
            background: #0B1726; color: #DCE7F4; border: 1px solid #243A52;
            border-radius: 8px; padding: 8px 12px; font-weight: bold;
        }
        QPushButton:hover { background: #10283D; border-color: #18D9EF; }

        QPushButton[role="primary"] {
            background: #18D9EF; color: #03212A; border: 1px solid #18D9EF; border-radius: 8px;
            padding: 8px 12px; font-weight: bold; min-height: 20px;
        }
        QPushButton[role="primary"]:hover    { background: #4FE5F5; }
        QPushButton[role="primary"]:disabled { background: #10283D; color: #718198; border-color: #243A52; }
        QPushButton[role="secondary"] {
            background: #0B1726; color: #DCE7F4; border: 1px solid #243A52; border-radius: 8px;
            padding: 8px 12px; font-weight: bold;
        }
        QPushButton[role="secondary"]:hover { background: #10283D; border-color: #18D9EF; }
        QPushButton[role="warning"] {
            background: #2E2A12; color: #f5c242; border: 1px solid #6B5A1E; border-radius: 8px;
            padding: 8px 12px; font-weight: bold;
        }
        QPushButton[role="warning"]:hover { background: #3D3716; }
        QPushButton[role="danger"] {
            background: #421F28; color: #FF8C99; border: 1px solid #C44858; border-radius: 8px;
            padding: 8px 12px; font-weight: bold;
        }
        QPushButton[role="danger"]:hover { background: #5A2733; }
        QPushButton[role="secondary"]:disabled, QPushButton[role="danger"]:disabled {
            background: #091522; color: #3A4D65; border: 1px solid #1A3046;
        }
        QComboBox::drop-down { border: none; width: 22px; }
        QComboBox QAbstractItemView {
            background: #091522; color: #FFFFFF; border: 1px solid #243A52;
            selection-background-color: #10394B; selection-color: #FFFFFF; outline: none;
        }
        QComboBox[role="accent"] { background: #092A3D; border: 1px solid #18D9EF; }

        /* ---------- Onglets ---------- */
        QFrame[role="tabBar"] { background: #091522; border: 1px solid #1C3045; border-radius: 10px; }
        QPushButton[role="tab"] {
            background: #0B1726; border: 1px solid #243A52; border-radius: 8px;
            padding: 9px 22px; color: #DCE7F4; font-weight: bold;
        }
        QPushButton[role="tab"]:hover   { background: #10283D; border-color: #18D9EF; }
        QPushButton[role="tab"]:checked { background: #092A3D; color: #18D9EF; border: 1px solid #18D9EF; }

        /* ---------- Tableaux ---------- */
        QTableView {
            background: #081421; alternate-background-color: #0B1828;
            border: 1px solid #1A3046; border-radius: 10px;
            gridline-color: #15283B; selection-background-color: #0C3042;
        }
        QTableView::item { padding: 8px; border-bottom: 1px solid #15283B; }
        QTableView::item:selected { background: #0C3042; color: #FFFFFF; }
        QHeaderView::section {
            background: #091522; color: #18D9EF; border: 0;
            border-bottom: 1px solid #1A3046;
            padding: 11px; font-weight: bold; font-size: 13px;
        }

        QPushButton[role="action"], QPushButton[role="actionDelete"], QPushButton[role="actionStar"] {
            border: 1px solid #243A52; border-radius: 8px; font-size: 17px; padding: 2px;
        }
        QPushButton[role="action"]       { background: #0B1726; }
        QPushButton[role="action"]:hover { background: #10283D; border-color: #18D9EF; }
        QPushButton[role="actionDelete"]       { background: #421F28; color: #FF8C99; }
        QPushButton[role="actionDelete"]:hover { background: #5A2733; }
        QPushButton[role="actionStar"]         { background: #2E2A12; color: #f5c242; }
        QPushButton[role="actionStar"]:hover   { background: #3D3716; }

        QMessageBox QPushButton { min-width: 70px; }

        /* ---------- Divers ---------- */
        QProgressBar { background: #15283B; border: none; border-radius: 4px; }
        QProgressBar::chunk { background: #18D9EF; border-radius: 4px; }

        QScrollArea, QScrollArea > QWidget > QWidget { background: #050B14; border: none; }
        QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
        QScrollBar::handle:vertical { background: #243A52; border-radius: 5px; min-height: 30px; }
        QScrollBar::add-line, QScrollBar::sub-line { height: 0; }
    )");
}

} // namespace EvalTheme


// ===========================================================================
// Widget promu ChartView (graphiques dans le .ui)
// ===========================================================================
#include <QtCharts/QChartView>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

// ---------------------------------------------------------------------------
//  Widget « promu » dans Qt Designer : QWidget  ->  ChartView (header.h)
//  Permet de placer un graphique QtCharts directement dans un fichier .ui
//  (compatible Qt 5 et Qt 6).
// ---------------------------------------------------------------------------
class ChartView : public QChartView
{
    Q_OBJECT

public:
    explicit ChartView(QWidget *parent = nullptr) : QChartView(parent)
    {
        setRenderHint(QPainter::Antialiasing);
        setFrameShape(QFrame::NoFrame);
        setStyleSheet("background: transparent;");
    }
};


// ===========================================================================
// Fenêtre principale (vue) — classe EvaluationWindow
// ===========================================================================
#include <QMainWindow>
#include <QList>
#include <QDate>

// === Classes métier des modules (une par étudiant) ===
#include "evaluation.h"

class QButtonGroup;
class QPushButton;
class QTableView;
class QAbstractItemModel;
class QLayout;

namespace Ui { class EvaluationWindow; }

class EvaluationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EvaluationWindow(QWidget *parent = nullptr);
    ~EvaluationWindow() override;

signals:
    // Integration : clic sur un autre module dans le menu de gauche
    void naviguer(const QString &module);

private slots:
    // =========================================================================
    // === SLOTS GÉNÉRAUX (NAVIGATION ENTRE PAGES) =============================
    // =========================================================================
    void allerVersPage(int index);

    // =========================================================================
    // === MODULE TABLEAU DE BORD ==============================================
    // =========================================================================
    // 💡 Étudiant responsable du module Tableau de bord travaille ici uniquement

    // =========================================================================
    // === MODULE EMPLOYÉS ======================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Employés travaille ici uniquement

    // =========================================================================
    // === MODULE PARTICIPANTS ==================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Participants travaille ici uniquement

    // =========================================================================
    // === MODULE ÉQUIPES ======================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Équipes travaille ici uniquement

    // =========================================================================
    // === MODULE PROJETS ======================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Projets travaille ici uniquement

    // =========================================================================
    // === MODULE EVALUATION ===================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Évaluation travaille ici uniquement
    void changerOngletEvaluation(int index);
    void actualiserEvaluations();
    void majSelectionEvaluation();
    void nouvelleEvaluation();
    void modifierEvaluationSelectionnee();
    void supprimerEvaluationSelectionnee();
    void enregistrerEvaluation();
    void annulerFormulaireEvaluation();
    void exporterEvaluationsPdf();
    void exporterFicheEvaluationPdf();
    void exporterAlertesPdf();
    void majEtoilesEvaluation(double note);
    void majCompteurEvaluation();

private:
    Ui::EvaluationWindow *ui;

    // =========================================================================
    // === SLOTS GÉNÉRAUX ======================================================
    // =========================================================================
    QButtonGroup *m_navigation = nullptr;

    // =========================================================================
    // === MODULE EVALUATION ===================================================
    // =========================================================================
    void initModuleEvaluation();
    void preparerTableEvaluation(QTableView *table);
    void remplacerModeleEvaluation(QTableView *table, QAbstractItemModel *modele);
    void remplirTypesEvaluation();
    void viderFormulaireEvaluation();
    bool controlerSaisieEvaluation();
    void marquerErreurEvaluation(QWidget *champ, bool enErreur);
    QString idEvaluationSelectionnee() const;
    QDate debutPeriodeEvaluation() const;
    void rafraichirStatistiquesEvaluation();
    void majGraphiqueBarresEvaluation(const StatistiquesEvaluation &s);
    void majGraphiqueDonutEvaluation(const StatistiquesEvaluation &s);
    void majDernieresEvaluations();
    bool exporterHtmlEnPdfEvaluation(const QString &html, const QString &nomParDefaut);
    QString htmlListeEvaluations(const QString &titre, const QList<ResumeEvaluation> &lignes) const;
    static void viderLayoutEvaluation(QLayout *layout);
    static QString ilYAEvaluation(const QDate &date);

    Evaluation          Evtmp;                         // objet de la classe métier (cf. atelier CRUD)
    QString             m_idEvaluationEnEdition;       // vide = ajout, sinon id modifié (ex. EV03)
    QList<QPushButton*> m_ongletsEvaluation;
    QWidget            *m_zoneLegendeEvaluation  = nullptr;
    QWidget            *m_zoneRecentesEvaluation = nullptr;
};


#endif // EVALUATIONWINDOW_H
