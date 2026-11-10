#include "employe.h"
#include "ui_employe.h"
#include "connection.h"

#include <QApplication>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrinter>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

namespace {
QString employeStyleSheet(bool dark)
{
    if (dark) {
        return R"(
            QWidget { background:#050B14; color:#F3F6FA; font-size:13px; font-family:'Segoe UI'; }
            QFrame#headerFrame { background:#081522; border:1px solid #1B344B; border-radius:12px; }
            QLabel#logoLabel { color:#08D5E8; font-size:22px; font-weight:700; }
            QLabel#titleLabel { color:#F3F6FA; font-size:24px; font-weight:700; }
            QLabel#subtitleLabel { color:#8FA9C3; }
            QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox { background:#081522; color:#F3F6FA; border:1px solid #1B344B; border-radius:9px; padding:8px 10px; }
            QLineEdit:focus, QComboBox:focus { border-color:#08D5E8; }
            QPushButton { background:#081522; color:#F3F6FA; border:1px solid #1B344B; border-radius:9px; padding:8px 14px; font-weight:600; }
            QPushButton:hover { border-color:#08D5E8; background:#0D2637; }
            QPushButton#btnAjouter { background:#08D5E8; color:#030A13; border-color:#08D5E8; font-weight:700; }
            QPushButton#btnSupprimer { color:#ff7b8a; border-color:#ff7b8a; }
            QTableWidget { background:#050B14; alternate-background-color:#081522; border:1px solid #1B344B; border-radius:10px; gridline-color:#1B344B; selection-background-color:#092B3A; selection-color:#08D5E8; }
            QHeaderView::section { background:#081522; color:#08D5E8; border:none; border-bottom:1px solid #1B344B; padding:10px 6px; font-weight:700; }
            QStatusBar { color:#8FA9C3; }
        )";
    }
    return R"(
        QWidget { background:#f5f7fb; color:#1f2937; font-size:13px; }
        QFrame#headerFrame { background:#ffffff; border:1px solid #dce3ee; border-radius:12px; }
        QLabel#logoLabel { color:#1677ff; font-size:22px; font-weight:700; }
        QLabel#titleLabel { color:#111827; font-size:24px; font-weight:700; }
        QLabel#subtitleLabel { color:#64748b; }
        QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox { background:#ffffff; color:#1f2937; border:1px solid #cbd5e1; border-radius:8px; padding:8px 10px; }
        QPushButton { background:#ffffff; color:#1f2937; border:1px solid #cbd5e1; border-radius:8px; padding:8px 14px; }
        QPushButton:hover { border-color:#1677ff; background:#f1f7ff; }
        QPushButton#btnAjouter { background:#1677ff; color:#ffffff; border-color:#1677ff; font-weight:700; }
        QPushButton#btnSupprimer { color:#dc2626; }
        QTableWidget { background:#ffffff; alternate-background-color:#f8fafc; border:1px solid #dce3ee; border-radius:10px; gridline-color:#e5e7eb; selection-background-color:#dbeafe; selection-color:#111827; }
        QHeaderView::section { background:#f8fafc; color:#1d4ed8; border:none; border-bottom:1px solid #dce3ee; padding:10px 6px; font-weight:700; }
        QStatusBar { color:#64748b; }
    )";
}

QString esc(const QString &s) { return s.toHtmlEscaped(); }
} // namespace

// ─────────────────────────────────────────────────────────────────
EmployePage::EmployePage(QWidget *parent)
    : QWidget(parent), ui(new Ui::EmployePage)
{
    ui->setupUi(this);
    setWindowTitle("Devena - Employés");
    resize(1400, 760);

    // Table — 5 colonnes Oracle + colonnes UI existantes
    ui->tableEmployes->setColumnCount(10);
    ui->tableEmployes->setHorizontalHeaderLabels({
        "ID", "Nom", "Prénom", "Email", "Téléphone", "Date d'embauche",
        "Salaire", "Poste", "Statut", "Mot de passe"
    });
    ui->tableEmployes->horizontalHeader()->setStretchLastSection(true);
    ui->tableEmployes->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableEmployes->setAlternatingRowColors(true);
    ui->tableEmployes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableEmployes->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableEmployes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableEmployes->setSortingEnabled(true);

    // Postes Oracle réels
    ui->comboPoste->clear();
    ui->comboPoste->addItems({"EV01 – Jury IA", "EV02 – Jury Web",
                              "EV03 – Jury IoT", "EV04 – Jury Sécurité"});
    ui->comboStatut->addItems({"Actif", "Inactif"});
    ui->dateEmbauche->setDate(QDate::currentDate());
    ui->salaire->setRange(0, 1000000);
    ui->salaire->setDecimals(2);
    ui->salaire->setSuffix(" DT");

    connect(ui->btnAjouter,          &QPushButton::clicked,            this, &EmployePage::ajouterEmploye);
    connect(ui->btnModifier,         &QPushButton::clicked,            this, &EmployePage::modifierEmploye);
    connect(ui->btnSupprimer,        &QPushButton::clicked,            this, &EmployePage::supprimerEmploye);
    connect(ui->btnExporterPDF,      &QPushButton::clicked,            this, &EmployePage::exporterPDF);
    connect(ui->btnTheme,            &QPushButton::clicked,            this, &EmployePage::changerTheme);
    connect(ui->btnEffacerRecherche, &QPushButton::clicked,            this, &EmployePage::viderRecherche);
    connect(ui->txtRecherche,        &QLineEdit::textChanged,          this, &EmployePage::rechercherEmploye);
    connect(ui->tableEmployes,       &QTableWidget::itemSelectionChanged, this, &EmployePage::chargerSelection);

    // Charger depuis Oracle si disponible, sinon données démo
    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    m_oracle = db.isOpen();
    if (m_oracle)
        chargerDepuisOracle();
    else
        chargerExemples();

    appliquerStyle(m_sombre);
}

EmployePage::~EmployePage() { delete ui; }

// ─────────────────────────────────────────────────────────────────
// ORACLE — Chargement
// ─────────────────────────────────────────────────────────────────
void EmployePage::chargerDepuisOracle()
{
    ui->tableEmployes->setSortingEnabled(false);
    ui->tableEmployes->setRowCount(0);

    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    QSqlQuery q(db);
    if (!q.exec("SELECT ID_EMPLOYE, NOM, PRENOM, MOT_DE_PASSE, POSTE "
                "FROM EMPLOYES ORDER BY NOM")) {
        QMessageBox::warning(this, "Erreur Oracle",
                             "Impossible de charger les employés :\n" + q.lastError().text());
        chargerExemples();
        return;
    }

    while (q.next()) {
        EmployeData e;
        e.id         = q.value("ID_EMPLOYE").toString();
        e.nom        = q.value("NOM").toString();
        e.prenom     = q.value("PRENOM").toString();
        e.motDePasse = q.value("MOT_DE_PASSE").toString();
        e.poste      = q.value("POSTE").toString();
        // Champs non présents en Oracle → vides
        e.email = e.telephone = e.statut = "";
        e.salaire = 0.0;
        ajouterLigne(e);
    }
    ui->tableEmployes->setSortingEnabled(true);
    ui->statusbar->showMessage(
        QString("Oracle : %1 employé(s) chargé(s)").arg(ui->tableEmployes->rowCount()), 4000);
}

// ─────────────────────────────────────────────────────────────────
// ORACLE — INSERT
// ─────────────────────────────────────────────────────────────────
bool EmployePage::insererOracle(const EmployeData &e)
{
    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    QSqlQuery q(db);
    q.prepare("INSERT INTO EMPLOYES (ID_EMPLOYE, NOM, PRENOM, MOT_DE_PASSE, POSTE) "
              "VALUES (:id, :nom, :prenom, :pwd, :poste)");
    q.bindValue(":id",     e.id);
    q.bindValue(":nom",    e.nom);
    q.bindValue(":prenom", e.prenom);
    q.bindValue(":pwd",    e.motDePasse);
    q.bindValue(":poste",  e.poste);

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur Oracle – Ajout",
                              q.lastError().text());
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────
// ORACLE — UPDATE
// ─────────────────────────────────────────────────────────────────
bool EmployePage::modifierOracle(const EmployeData &e)
{
    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    QSqlQuery q(db);
    q.prepare("UPDATE EMPLOYES SET NOM=:nom, PRENOM=:prenom, "
              "MOT_DE_PASSE=:pwd, POSTE=:poste "
              "WHERE ID_EMPLOYE=:id");
    q.bindValue(":nom",    e.nom);
    q.bindValue(":prenom", e.prenom);
    q.bindValue(":pwd",    e.motDePasse);
    q.bindValue(":poste",  e.poste);
    q.bindValue(":id",     e.id);

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur Oracle – Modification",
                              q.lastError().text());
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────
// ORACLE — DELETE
// ─────────────────────────────────────────────────────────────────
bool EmployePage::supprimerOracle(const QString &id)
{
    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    QSqlQuery q(db);
    q.prepare("DELETE FROM EMPLOYES WHERE ID_EMPLOYE=:id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        const QString err = q.lastError().text();
        // ORA-02292 = FK violation (l'employé gère des évaluations via GERER)
        if (err.contains("02292") || err.contains("FK") || err.contains("GERER")) {
            QMessageBox::warning(this, "Suppression impossible",
                "Cet employé est lié à des évaluations dans la table GERER.\n"
                "Supprimez d'abord les liens dans GERER puis réessayez.");
        } else {
            QMessageBox::critical(this, "Erreur Oracle – Suppression", err);
        }
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────
// Données de démo (si Oracle non disponible)
// ─────────────────────────────────────────────────────────────────
void EmployePage::chargerExemples()
{
    ajouterLigne({"001", "aakari",   "mahdi", "", "", QDate(2026,1,15), 0, "EV01 – Jury IA",       "ai",              "Actif"});
    ajouterLigne({"002", "Ben Salah","Amira", "", "", QDate(2026,2,10), 0, "EV01 – Jury IA",       "Data science",    "Actif"});
    ajouterLigne({"003", "Gharbi",   "Walid", "", "", QDate(2026,3,5),  0, "EV02 – Jury Web",      "Développement web","Actif"});
    ajouterLigne({"004", "Trabelsi", "Ines",  "", "", QDate(2026,4,1),  0, "EV02 – Jury Web",      "UX design",       "Actif"});
    ajouterLigne({"005", "Mansouri", "Karim", "", "", QDate(2026,5,1),  0, "EV03 – Jury IoT",      "Systèmes emb.",   "Actif"});
    ui->statusbar->showMessage("Mode démo — Oracle non disponible", 5000);
}

void EmployePage::viderFormulaire()
{
    ui->txtNom->clear();
    ui->txtPrenom->clear();
    ui->txtEmail->clear();
    ui->txtTelephone->clear();
    ui->txtMotDePasse->clear();
    ui->dateEmbauche->setDate(QDate::currentDate());
    ui->salaire->setValue(0);
    ui->comboPoste->setCurrentIndex(0);
    ui->comboStatut->setCurrentIndex(0);
}

// ─────────────────────────────────────────────────────────────────
void EmployePage::ajouterLigne(const EmployeData &e)
{
    const int row = ui->tableEmployes->rowCount();
    ui->tableEmployes->insertRow(row);
    const QStringList values = {
        e.id, e.nom, e.prenom, e.email, e.telephone,
        e.dateEmbauche.isValid() ? e.dateEmbauche.toString("dd/MM/yyyy") : "",
        e.salaire > 0 ? QString::number(e.salaire, 'f', 2) + " DT" : "",
        e.poste, e.statut, "••••••••"
    };
    for (int c = 0; c < values.size(); ++c) {
        auto *item = new QTableWidgetItem(values[c]);
        item->setData(Qt::UserRole, e.motDePasse);
        ui->tableEmployes->setItem(row, c, item);
    }
    ui->tableEmployes->setRowHeight(row, 42);
}

EmployeData EmployePage::lireLigne(int row) const
{
    EmployeData e;
    e.id         = ui->tableEmployes->item(row,0)->text();
    e.nom        = ui->tableEmployes->item(row,1)->text();
    e.prenom     = ui->tableEmployes->item(row,2)->text();
    e.email      = ui->tableEmployes->item(row,3)->text();
    e.telephone  = ui->tableEmployes->item(row,4)->text();
    e.dateEmbauche = QDate::fromString(ui->tableEmployes->item(row,5)->text(), "dd/MM/yyyy");
    e.salaire    = ui->tableEmployes->item(row,6)->text().remove(" DT").toDouble();
    e.poste      = ui->tableEmployes->item(row,7)->text();
    e.statut     = ui->tableEmployes->item(row,8)->text();
    e.motDePasse = ui->tableEmployes->item(row,9)->data(Qt::UserRole).toString();
    return e;
}

int EmployePage::ligneSelectionnee() const
{
    const auto ranges = ui->tableEmployes->selectedRanges();
    return ranges.isEmpty() ? -1 : ranges.first().topRow();
}

void EmployePage::chargerSelection()
{
    const int row = ligneSelectionnee();
    if (row < 0) return;
    const EmployeData e = lireLigne(row);
    ui->txtNom->setText(e.nom);
    ui->txtPrenom->setText(e.prenom);
    ui->txtEmail->setText(e.email);
    ui->txtTelephone->setText(e.telephone);
    ui->dateEmbauche->setDate(e.dateEmbauche.isValid() ? e.dateEmbauche : QDate::currentDate());
    ui->salaire->setValue(e.salaire);
    ui->comboPoste->setCurrentText(e.poste);
    ui->comboStatut->setCurrentText(e.statut);
    ui->txtMotDePasse->clear();
}

// ─────────────────────────────────────────────────────────────────
// AJOUTER
// ─────────────────────────────────────────────────────────────────
void EmployePage::ajouterEmploye()
{
    const QString nom    = ui->txtNom->text().trimmed();
    const QString prenom = ui->txtPrenom->text().trimmed();
    if (nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires", "Nom et prénom sont obligatoires.");
        return;
    }

    EmployeData e;
    // Générer l'ID : format numérique à 3 chiffres
    e.id         = QString("%1").arg(m_nextId++, 3, 10, QChar('0'));
    e.nom        = nom;
    e.prenom     = prenom;
    e.email      = ui->txtEmail->text().trimmed();
    e.telephone  = ui->txtTelephone->text().trimmed();
    e.dateEmbauche = ui->dateEmbauche->date();
    e.salaire    = ui->salaire->value();
    // Extraire le code POSTE (ex: "EV01 – Jury IA" → "EV01")
    e.poste      = ui->comboPoste->currentText().left(4);
    e.motDePasse = ui->txtMotDePasse->text();
    e.statut     = ui->comboStatut->currentText();

    if (m_oracle) {
        if (!insererOracle(e)) return;
        chargerDepuisOracle();
    } else {
        ajouterLigne(e);
    }
    viderFormulaire();
    ui->statusbar->showMessage("Employé ajouté : " + e.id + " – " + e.prenom + " " + e.nom, 4000);
}

// ─────────────────────────────────────────────────────────────────
// MODIFIER
// ─────────────────────────────────────────────────────────────────
void EmployePage::modifierEmploye()
{
    const int row = ligneSelectionnee();
    if (row < 0) {
        QMessageBox::information(this, "Modification", "Sélectionnez d'abord un employé.");
        return;
    }
    EmployeData e = lireLigne(row);
    e.nom        = ui->txtNom->text().trimmed();
    e.prenom     = ui->txtPrenom->text().trimmed();
    e.email      = ui->txtEmail->text().trimmed();
    e.telephone  = ui->txtTelephone->text().trimmed();
    e.dateEmbauche = ui->dateEmbauche->date();
    e.salaire    = ui->salaire->value();
    e.poste      = ui->comboPoste->currentText().left(4);
    if (!ui->txtMotDePasse->text().isEmpty())
        e.motDePasse = ui->txtMotDePasse->text();
    e.statut     = ui->comboStatut->currentText();

    if (m_oracle) {
        if (!modifierOracle(e)) return;
        chargerDepuisOracle();
    } else {
        // Mise à jour locale
        ui->tableEmployes->removeRow(row);
        ui->tableEmployes->insertRow(row);
        const QStringList vals = {e.id,e.nom,e.prenom,e.email,e.telephone,
            e.dateEmbauche.toString("dd/MM/yyyy"),
            QString::number(e.salaire,'f',2)+" DT",
            e.poste,e.statut,"••••••••"};
        for (int c=0;c<vals.size();++c) {
            auto *item=new QTableWidgetItem(vals[c]);
            item->setData(Qt::UserRole,e.motDePasse);
            ui->tableEmployes->setItem(row,c,item);
        }
        ui->tableEmployes->selectRow(row);
    }
    ui->statusbar->showMessage("Employé modifié : " + e.id, 3000);
}

// ─────────────────────────────────────────────────────────────────
// SUPPRIMER
// ─────────────────────────────────────────────────────────────────
void EmployePage::supprimerEmploye()
{
    const int row = ligneSelectionnee();
    if (row < 0) {
        QMessageBox::information(this, "Suppression", "Sélectionnez d'abord un employé.");
        return;
    }
    const QString id  = ui->tableEmployes->item(row,0)->text();
    const QString nom = ui->tableEmployes->item(row,2)->text() + " "
                      + ui->tableEmployes->item(row,1)->text();

    if (QMessageBox::question(this, "Confirmer", "Supprimer " + nom + " ?")
            == QMessageBox::Yes) {
        if (m_oracle) {
            if (!supprimerOracle(id)) return;
            chargerDepuisOracle();
        } else {
            ui->tableEmployes->removeRow(row);
        }
        ui->statusbar->showMessage("Employé supprimé : " + id, 3000);
    }
}

// ─────────────────────────────────────────────────────────────────
void EmployePage::rechercherEmploye(const QString &texte)
{
    const QString q = texte.trimmed();
    for (int r=0; r<ui->tableEmployes->rowCount(); ++r) {
        bool visible = q.isEmpty();
        if (!visible) {
            for (int c=0; c<ui->tableEmployes->columnCount(); ++c) {
                if (ui->tableEmployes->item(r,c) &&
                    ui->tableEmployes->item(r,c)->text().contains(q, Qt::CaseInsensitive)) {
                    visible = true; break;
                }
            }
        }
        ui->tableEmployes->setRowHidden(r, !visible);
    }
}

void EmployePage::viderRecherche()   { ui->txtRecherche->clear(); }
void EmployePage::changerTheme()     { m_sombre = !m_sombre; appliquerStyle(m_sombre); }

void EmployePage::appliquerStyle(bool sombre)
{
    qApp->setStyleSheet(employeStyleSheet(sombre));
    ui->btnTheme->setText(sombre ? "☀ Mode clair" : "☾ Mode sombre");
}

void EmployePage::ouvrirFormulaire(int /*row*/) {}

// ─────────────────────────────────────────────────────────────────
// EXPORT PDF
// ─────────────────────────────────────────────────────────────────
void EmployePage::exporterPDF()
{
    const QString file = QFileDialog::getSaveFileName(
        this, "Exporter les employés", "Employes.pdf", "PDF (*.pdf)");
    if (file.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file);

    QString html = R"(<html><head><style>
        body{font-family:Arial;color:#172033;}
        h1{color:#08D5E8;}
        table{border-collapse:collapse;width:100%;font-size:8pt;}
        th{background:#081522;color:white;padding:6px;}
        td{border:1px solid #d8dee9;padding:5px;}
        .logo{font-size:18pt;font-weight:bold;color:#08D5E8;}
        </style></head><body>)";
    html += "<div class='logo'>◆ Devena</div><h1>Gestion des employés</h1>";
    html += "<p>Liste des employés — " + QDate::currentDate().toString("dd/MM/yyyy") + "</p>";
    html += "<table><tr>";
    for (const auto &h : {"ID","Nom","Prénom","Poste"})
        html += "<th>" + esc(h) + "</th>";
    html += "</tr>";
    for (int r=0; r<ui->tableEmployes->rowCount(); ++r) {
        if (ui->tableEmployes->isRowHidden(r)) continue;
        html += "<tr>";
        // Colonnes Oracle : ID(0), Nom(1), Prénom(2), Poste(7)
        for (int c : {0,1,2,7}) {
            auto *item = ui->tableEmployes->item(r,c);
            html += "<td>" + esc(item ? item->text() : "") + "</td>";
        }
        html += "</tr>";
    }
    html += "</table></body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);
    ui->statusbar->showMessage("PDF exporté : " + file, 5000);
}
