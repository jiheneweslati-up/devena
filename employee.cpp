#include "employee.h"

#include <QApplication>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QTableView>
#include <QTextDocument>
#include <QTextStream>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QPixmap>
#include <QPolygonF>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// ══════════════════════════════════════════════════════════════════
//  Theme helper
// ══════════════════════════════════════════════════════════════════
QString makeStyle(bool dark)
{
    const QString bg   = dark ? "#050B14" : "#f0f4f8";
    const QString card = dark ? "#081522" : "#ffffff";
    const QString tx   = dark ? "#F3F6FA" : "#1b2430";
    const QString tx2  = dark ? "#8FA9C3" : "#5b6675";
    const QString bd   = dark ? "#1B344B" : "#dde3ea";
    const QString ac   = dark ? "#08D5E8" : "#1f6feb";
    const QString onAc = dark ? "#030A13" : "#ffffff";
    const QString acBg = dark ? "#092B3A" : "#e3eefe";
    const QString mu   = dark ? "#4785B8" : "#5b6675";   // texte discret du menu

    return QString(R"(
QMainWindow, QDialog, QWidget#root { background:%1; }
QWidget { color:%3; font-size:13px; font-family:'Segoe UI'; }
QFrame#kpi { background:%2; border:1px solid %5; border-radius:10px; }
QLabel#title { font-size:30px; font-weight:700; }
QLabel#sub, QLabel#kpiTitle { color:%4; }
QLabel#kpiVal { font-size:22px; font-weight:700; color:%6; }
QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox { background:%2; border:1px solid %5; border-radius:9px; padding:7px 10px; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus { border:1px solid %6; }
QPushButton { background:%2; border:1px solid %5; border-radius:9px; padding:7px 14px; color:%3; }
QPushButton:hover { background:#0D2637; border-color:%6; }
QPushButton#primary { background:%6; border-color:%6; color:%7; font-weight:700; }
QPushButton#primary:hover { background:#38e8f5; }
QTableView { background:%1; border:1px solid %5; border-radius:9px; gridline-color:transparent;
             selection-background-color:%8; selection-color:%3; }
QHeaderView::section { background:%1; color:%6; border:none; border-bottom:1px solid %5;
                       padding:10px 7px; font-weight:700; font-size:13px; }
QLabel#avatar { background:%8; color:%6; border-radius:40px; font-size:26px; font-weight:600; }
QLabel#status { border-radius:10px; padding:3px 10px; font-weight:600; }
QLabel#logo { font-size:22px; font-weight:700; color:%6; }
QFrame#sidebar { background:#030A13; border-right:1px solid %5; }
QPushButton#nav { background:transparent; border:1px solid transparent; text-align:left; padding:11px 14px; color:%4; border-radius:9px; }
QFrame#sidebar QLabel#sub { color:%9; }
QPushButton#nav:hover { background:#0D2637; color:%3; border-color:%5; }
QPushButton#nav[active="true"] { background:#092B3A; color:%6; border:1px solid %6; font-weight:700; min-height:48px; }
QLabel#section { font-size:20px; font-weight:700; }
QMessageBox { background:%1; }
QScrollBar:horizontal { height:8px; background:%2; }
QScrollBar::handle:horizontal { background:%5; border-radius:4px; }
QScrollBar:vertical { width:8px; background:%2; }
QScrollBar::handle:vertical { background:%5; border-radius:4px; }
)").arg(bg, card, tx, tx2, bd, ac, onAc, acBg, mu);
}

// ══════════════════════════════════════════════════════════════════
//  EmployeeModel
// ══════════════════════════════════════════════════════════════════
EmployeeModel::EmployeeModel(QObject* parent) : QAbstractTableModel(parent) {}

int EmployeeModel::rowCount   (const QModelIndex& parent) const { return parent.isValid() ? 0 : m_data.size(); }
int EmployeeModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : 10; }

QVariant EmployeeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const Employee& e = m_data.at(index.row());
    if (role == IdRole) return e.id_emp;
    if (role != Qt::DisplayRole) return {};

    switch (index.column()) {
    case 0: return e.id_emp;
    case 1: return e.nom;
    case 2: return e.prenom;
    case 3: return e.email;
    case 4: return e.telephone;
    case 5: return e.date_embauche;
    case 6: return QString::number(e.salaire, 'f', 2) + " DT";
    case 7: return e.poste;
    case 8: return e.statut;
    case 9: return QString();
    default: return {};
    }
}

QVariant EmployeeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static const QStringList headers = {
        "ID", "Nom", "Prénom", "Email", "Téléphone", "Date embauche",
        "Salaire", "Poste", "Statut", "Actions"
    };
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return headers.value(section);
    return {};
}

int EmployeeModel::rowOf(const QString& id) const
{
    for (int i = 0; i < m_data.size(); ++i)
        if (m_data[i].id_emp == id) return i;
    return -1;
}

QString EmployeeModel::nextId()
{
    return QString("EMP%1").arg(m_next++, 3, 10, QChar('0'));
}

void EmployeeModel::insertAt(int row, const Employee& e)
{
    row = qBound(0, row, m_data.size());
    beginInsertRows({}, row, row);
    m_data.insert(row, e);
    endInsertRows();
    emit changed();
}

void EmployeeModel::removeAt(int row)
{
    if (row < 0 || row >= m_data.size()) return;
    beginRemoveRows({}, row, row);
    m_data.removeAt(row);
    endRemoveRows();
    emit changed();
}

void EmployeeModel::replace(int row, const Employee& e)
{
    if (row < 0 || row >= m_data.size()) return;
    m_data[row] = e;
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
    emit changed();
}

// ──────────────────────────────────────────────────────────────────
void EmployeeModel::loadFromDatabase()
{
    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    if (!db.isOpen()) {
        QMessageBox::warning(nullptr, "Base de données",
            "Connexion Oracle non disponible.\nChargement des données de démonstration.");
        loadSampleData();
        return;
    }

    QSqlQuery q(db);
    if (!q.exec("SELECT ID_EMPLOYE, NOM, PRENOM, MOT_DE_PASSE, POSTE "
                "FROM EMPLOYES ORDER BY NOM")) {
        QMessageBox::warning(nullptr, "Erreur SQL",
            "Impossible de charger les employés :\n" + q.lastError().text());
        loadSampleData();
        return;
    }

    beginResetModel();
    m_data.clear();
    while (q.next()) {
        Employee e;
        e.id_emp       = q.value("ID_EMPLOYE").toString();
        e.nom          = q.value("NOM").toString();
        e.prenom       = q.value("PRENOM").toString();
        e.mot_de_passe = q.value("MOT_DE_PASSE").toString();
        e.poste        = q.value("POSTE").toString();
        // Fields not stored in Oracle — keep defaults
        e.email        = "";
        e.telephone    = "";
        e.date_embauche= "";
        e.salaire      = 0.0;
        e.photo_profil = "";
        e.statut       = "Actif";
        m_data.append(e);
    }
    endResetModel();
    emit changed();
}

// ──────────────────────────────────────────────────────────────────
void EmployeeModel::loadSampleData()
{
    beginResetModel();
    m_data.clear();
    const QList<Employee> samples = {
        {"EMP001", "Ben Ali",   "Yassine", "yassine.benali@esprit.tn",  "+216 20 111 111", "12/09/2025", 1800, "EV01", "123456", "", "Actif"},
        {"EMP002", "Trabelsi",  "Aya",     "aya.trabelsi@esprit.tn",    "+216 21 222 222", "18/09/2025", 1450, "EV02", "123456", "", "Actif"},
        {"EMP003", "Sassi",     "Omar",    "omar.sassi@esprit.tn",      "+216 22 333 333", "01/10/2025", 1500, "EV02", "123456", "", "Actif"},
        {"EMP004", "Kacem",     "Salma",   "salma.kacem@esprit.tn",     "+216 23 444 444", "14/10/2025", 1750, "EV01", "123456", "", "Inactif"},
        {"EMP005", "Jaziri",    "Mohamed", "mohamed.jaziri@esprit.tn",  "+216 24 555 555", "20/10/2025", 1450, "EV03", "123456", "", "Actif"}
    };
    m_data = samples;
    endResetModel();
    emit changed();
}

// ══════════════════════════════════════════════════════════════════
//  Undo commands (unchanged logic, still work after DB reloads)
// ══════════════════════════════════════════════════════════════════
AddEmployeeCmd::AddEmployeeCmd(EmployeeModel* model, const Employee& employee)
    : m_model(model), m_employee(employee), m_row(model->rowCount())
{
    setText(QObject::tr("Ajouter %1 %2").arg(employee.prenom, employee.nom));
}
void AddEmployeeCmd::redo() { m_model->insertAt(m_row, m_employee); }
void AddEmployeeCmd::undo() { m_model->removeAt(m_model->rowOf(m_employee.id_emp)); }

DeleteEmployeeCmd::DeleteEmployeeCmd(EmployeeModel* model, const Employee& employee)
    : m_model(model), m_employee(employee), m_row(model->rowOf(employee.id_emp))
{
    setText(QObject::tr("Supprimer %1 %2").arg(employee.prenom, employee.nom));
}
void DeleteEmployeeCmd::redo() { m_model->removeAt(m_model->rowOf(m_employee.id_emp)); }
void DeleteEmployeeCmd::undo() { m_model->insertAt(m_row, m_employee); }

EditEmployeeCmd::EditEmployeeCmd(EmployeeModel* model, const Employee& oldEmployee,
                                 const Employee& newEmployee)
    : m_model(model), m_oldEmployee(oldEmployee), m_newEmployee(newEmployee)
{
    setText(QObject::tr("Modifier %1 %2").arg(oldEmployee.prenom, oldEmployee.nom));
}
void EditEmployeeCmd::redo() { m_model->replace(m_model->rowOf(m_newEmployee.id_emp), m_newEmployee); }
void EditEmployeeCmd::undo() { m_model->replace(m_model->rowOf(m_oldEmployee.id_emp), m_oldEmployee); }

// ══════════════════════════════════════════════════════════════════
//  ActionsDelegate
// ══════════════════════════════════════════════════════════════════
QRect ActionsDelegate::buttonRect(const QRect& cell, int index)
{
    return QRect(cell.left() + 6 + index * 34, cell.center().y() - 13, 28, 26);
}

void ActionsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    opt.text.clear();
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    const char* glyphs[] = {"👁", "✏", "🗑"};
    for (int i = 0; i < 3; ++i) {
        QRect r = buttonRect(option.rect, i);
        painter->drawText(r, Qt::AlignCenter, QString::fromUtf8(glyphs[i]));
    }
    painter->restore();
}

bool ActionsDelegate::editorEvent(QEvent* event, QAbstractItemModel*,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index)
{
    if (event->type() != QEvent::MouseButtonRelease) return false;
    const auto* mouse = static_cast<QMouseEvent*>(event);
    const QString id = index.data(EmployeeModel::IdRole).toString();
    if      (buttonRect(option.rect, 0).contains(mouse->pos())) emit viewClicked(id);
    else if (buttonRect(option.rect, 1).contains(mouse->pos())) emit editClicked(id);
    else if (buttonRect(option.rect, 2).contains(mouse->pos())) emit deleteClicked(id);
    return true;
}

// ══════════════════════════════════════════════════════════════════
//  StatusDelegate
// ══════════════════════════════════════════════════════════════════
void StatusDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                           const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    opt.text.clear();
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    const QString status = index.data().toString();
    QRect r = option.rect.adjusted(8, 9, -8, -9);
    const bool active = status.compare("Actif", Qt::CaseInsensitive) == 0;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(active ? "#123d30" : "#3a2530"));
    painter->drawRoundedRect(r, 10, 10);
    painter->setPen(QColor(active ? "#4ade80" : "#fb7185"));
    painter->drawText(r, Qt::AlignCenter, status);
    painter->restore();
}

QSize StatusDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index) + QSize(0, 10);
}

// ══════════════════════════════════════════════════════════════════
//  EmployeeProfileDialog
// ══════════════════════════════════════════════════════════════════
EmployeeProfileDialog::EmployeeProfileDialog(const Employee& e, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Profil de l'employé");
    setMinimumWidth(430);
    auto* v = new QVBoxLayout(this);
    v->setSpacing(8);

    auto* avatar = new QLabel;
    avatar->setObjectName("avatar");
    avatar->setFixedSize(80, 80);
    avatar->setAlignment(Qt::AlignCenter);
    if (!e.photo_profil.isEmpty() && QFileInfo::exists(e.photo_profil)) {
        QPixmap pix(e.photo_profil);
        avatar->setPixmap(pix.scaled(80, 80, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        avatar->setText((e.prenom.left(1) + e.nom.left(1)).toUpper());
    }
    v->addWidget(avatar, 0, Qt::AlignHCenter);

    auto* name = new QLabel(e.prenom + " " + e.nom);
    name->setObjectName("title");
    v->addWidget(name, 0, Qt::AlignHCenter);
    auto* id = new QLabel(e.id_emp);
    id->setObjectName("sub");
    v->addWidget(id, 0, Qt::AlignHCenter);

    auto* form = new QFormLayout;
    form->addRow("Email",         new QLabel(e.email.isEmpty()        ? "(non renseigné)" : e.email));
    form->addRow("Téléphone",     new QLabel(e.telephone.isEmpty()    ? "(non renseigné)" : e.telephone));
    form->addRow("Date embauche", new QLabel(e.date_embauche.isEmpty()? "(non renseigné)" : e.date_embauche));
    form->addRow("Salaire",       new QLabel(e.salaire > 0 ? QString::number(e.salaire, 'f', 2) + " DT" : "(non renseigné)"));
    form->addRow("Poste",         new QLabel(e.poste));
    form->addRow("Statut",        new QLabel(e.statut));
    v->addSpacing(8);
    v->addLayout(form);

    auto* close = new QPushButton("Fermer");
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    v->addSpacing(8);
    v->addWidget(close);
}

// ══════════════════════════════════════════════════════════════════
//  EmployeeFormDialog
// ══════════════════════════════════════════════════════════════════
EmployeeFormDialog::EmployeeFormDialog(QWidget* parent, const Employee* e,
                                       const QString& suggestedId)
    : QDialog(parent)
{
    setWindowTitle(e ? "Modifier un employé" : "Ajouter un employé");
    setMinimumWidth(520);

    // ── ID field (editable when adding, read-only when editing) ──
    m_id        = new QLineEdit;
    m_id->setPlaceholderText("ex: 009");
    if (e) {
        m_id->setText(e->id_emp);
        m_id->setReadOnly(true);
        m_id->setStyleSheet("color:#8FA9C3;");   // greyed out
    } else {
        m_id->setText(suggestedId);
    }

    m_nom       = new QLineEdit;
    m_prenom    = new QLineEdit;
    m_email     = new QLineEdit;
    m_telephone = new QLineEdit;
    m_date      = new QDateEdit(QDate::currentDate());
    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat("dd/MM/yyyy");
    m_salaire   = new QDoubleSpinBox;
    m_salaire->setRange(0, 99999999);
    m_salaire->setDecimals(2);
    m_salaire->setSuffix(" DT");
    m_poste     = new QComboBox;
    m_poste->addItems({"EV01", "EV02", "EV03", "EV04"});
    m_password  = new QLineEdit;
    m_password->setEchoMode(QLineEdit::Password);
    m_photo     = new QLineEdit;
    m_photo->setReadOnly(true);
    auto* photoBtn = new QPushButton("Choisir...");
    m_statut    = new QComboBox;
    m_statut->addItems({"Actif", "Inactif"});
    m_error     = new QLabel;
    m_error->setStyleSheet("color:#ef4444;");
    m_error->setWordWrap(true);

    // Le formulaire n'affiche que les attributs de la table EMPLOYES :
    //   ID_EMPLOYE, NOM, PRENOM, MOT_DE_PASSE, POSTE
    auto* form = new QFormLayout;
    form->setSpacing(10);
    form->addRow("ID Employé *",      m_id);
    form->addRow("Nom *",             m_nom);
    form->addRow("Prénom *",          m_prenom);
    form->addRow("Mot de passe *",    m_password);
    form->addRow("Poste *",           m_poste);

    // Les champs qui n'existent pas dans la base (email, téléphone, date d'embauche,
    // salaire, photo, statut) ne sont plus affichés. Ils restent en mémoire, cachés,
    // pour que le reste du module (tableau, fiche, PDF) continue de fonctionner.
    for (QWidget* cache : {static_cast<QWidget*>(m_email), static_cast<QWidget*>(m_telephone),
                           static_cast<QWidget*>(m_date),  static_cast<QWidget*>(m_salaire),
                           static_cast<QWidget*>(m_photo), static_cast<QWidget*>(photoBtn),
                           static_cast<QWidget*>(m_statut)}) {
        cache->setParent(this);
        cache->hide();
    }

    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb,       &QDialogButtonBox::accepted, this, [this] { validate(); });
    connect(bb,       &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(photoBtn, &QPushButton::clicked,       this, &EmployeeFormDialog::choosePhoto);

    auto* v = new QVBoxLayout(this);
    v->addLayout(form);
    v->addWidget(m_error);
    v->addWidget(bb);

    if (e) {
        m_nom->setText(e->nom);
        m_prenom->setText(e->prenom);
        m_email->setText(e->email);
        m_telephone->setText(e->telephone);
        const QDate d = QDate::fromString(e->date_embauche, "dd/MM/yyyy");
        if (d.isValid()) m_date->setDate(d);
        m_salaire->setValue(e->salaire);
        m_poste->setCurrentText(e->poste);
        m_password->setText(e->mot_de_passe);
        m_photo->setText(e->photo_profil);
        m_statut->setCurrentText(e->statut);
    }
}

Employee EmployeeFormDialog::result() const
{
    Employee e;
    e.id_emp        = m_id->text().trimmed();
    e.nom           = m_nom->text().trimmed();
    e.prenom        = m_prenom->text().trimmed();
    e.email         = m_email->text().trimmed();
    e.telephone     = m_telephone->text().trimmed();
    e.date_embauche = m_date->date().toString("dd/MM/yyyy");
    e.salaire       = m_salaire->value();
    e.poste         = m_poste->currentText();
    e.mot_de_passe  = m_password->text();
    e.photo_profil  = m_photo->text().trimmed();
    e.statut        = m_statut->currentText();
    return e;
}

void EmployeeFormDialog::choosePhoto()
{
    const QString file = QFileDialog::getOpenFileName(
        this, "Choisir une photo", QString(),
        "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (!file.isEmpty()) m_photo->setText(file);
}

void EmployeeFormDialog::validate()
{
    if (m_id->text().trimmed().isEmpty()) {
        m_error->setText("L'ID employé est obligatoire (ex: 009).");
        return;
    }
    if (m_nom->text().trimmed().isEmpty() || m_prenom->text().trimmed().isEmpty()) {
        m_error->setText("Le nom et le prénom sont obligatoires.");
        return;
    }
    if (m_password->text().isEmpty()) {
        m_error->setText("Le mot de passe est obligatoire.");
        return;
    }
    accept();
}

// ══════════════════════════════════════════════════════════════════
//  Trend chart (unchanged, internal widget)
// ══════════════════════════════════════════════════════════════════
namespace {
class EmployeeTrendChart : public QWidget {
public:
    explicit EmployeeTrendChart(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumHeight(135);
        setMaximumHeight(150);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        const QColor grid("#1B344B");
        const QColor line("#08D5E8");
        const QRectF area(42, 18, qMax(100, width()-58), height()-42);
        p.setPen(QPen(grid, 1));
        for (int i=0; i<=6; ++i) {
            const qreal y = area.top() + area.height()*i/6.0;
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
        }
        const QStringList months = {"Janv", "Févr", "Mars", "Avr", "Mai", "Juin", "Juil", "Août", "Sept"};
        for (int i=0; i<months.size(); ++i) {
            const qreal x = area.left() + area.width()*i/(months.size()-1.0);
            p.drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
            p.setPen(QColor("#4785B8"));
            p.drawText(QRectF(x-24, area.bottom()+5, 48, 18), Qt::AlignHCenter, months[i]);
            p.setPen(QPen(grid, 1));
        }
        const QVector<qreal> values = {0.12,0.32,0.32,0.52,0.62,0.72,0.82,0.94,0.94};
        QPolygonF points;
        for (int i=0; i<values.size(); ++i)
            points << QPointF(area.left()+area.width()*i/(values.size()-1.0), area.bottom()-area.height()*values[i]);
        p.setPen(QPen(line, 2));
        p.drawPolyline(points);
        p.setBrush(line);
        for (const QPointF &pt : points) p.drawEllipse(pt, 3, 3);
    }
};
} // namespace

// ══════════════════════════════════════════════════════════════════
//  EmployeWindow
// ══════════════════════════════════════════════════════════════════
EmployeWindow::EmployeWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("Devena - Employés");
    resize(1450, 820);
    setMinimumSize(1120, 650);

    m_model = new EmployeeModel(this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);
    m_stack = new QUndoStack(this);

    // Load from Oracle; falls back to demo data automatically on failure
    m_model->loadFromDatabase();

    auto* shell = new QWidget;
    shell->setObjectName("root");
    auto* h = new QHBoxLayout(shell);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    h->addWidget(buildSidebar());

    auto* root = new QWidget;
    h->addWidget(root, 1);
    setCentralWidget(shell);

    auto* v = new QVBoxLayout(root);
    v->setContentsMargins(20, 16, 20, 16);
    v->setSpacing(12);

    auto* head = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    auto* title = new QLabel("Employés");
    title->setObjectName("title");
    auto* sub = new QLabel("Gérez les employés du hackathon");
    sub->setObjectName("sub");
    titles->addWidget(title);
    titles->addWidget(sub);
    head->addLayout(titles);
    head->addStretch();

    m_themeBtn = new QPushButton("🌙 Mode sombre");
    auto* pdfBtn = new QPushButton("Exporter en PDF");
    auto* addBtn = new QPushButton("+ Ajouter un employé");
    addBtn->setObjectName("primary");
    head->addWidget(m_themeBtn);
    head->addWidget(pdfBtn);
    head->addWidget(addBtn);
    v->addLayout(head);

    auto* bar = new QHBoxLayout;
    m_search = new QLineEdit;
    m_search->setPlaceholderText("Rechercher : nom, prénom, ID, poste...");
    m_search->setClearButtonEnabled(true);
    auto* undoBtn = new QPushButton("↶ Annuler (Ctrl+Z)");
    bar->addWidget(m_search, 1);
    bar->addWidget(undoBtn);
    v->addLayout(bar);

    m_view = new QTableView;
    m_view->setModel(m_proxy);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setShowGrid(false);
    m_view->setSortingEnabled(true);
    m_view->setWordWrap(false);
    m_view->verticalHeader()->hide();
    m_view->verticalHeader()->setDefaultSectionSize(48);
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setStretchLastSection(false);
    m_view->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_view->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Fixed);
    m_view->setColumnWidth(0, 78);
    m_view->setColumnWidth(1, 105);
    m_view->setColumnWidth(2, 105);
    m_view->setColumnWidth(4, 115);
    m_view->setColumnWidth(5, 105);
    m_view->setColumnWidth(6, 105);
    m_view->setColumnWidth(7, 125);
    m_view->setColumnWidth(8, 95);
    m_view->setColumnWidth(9, 120);
    m_view->sortByColumn(1, Qt::AscendingOrder);

    auto* statusDelegate  = new StatusDelegate(m_view);
    auto* actionsDelegate = new ActionsDelegate(m_view);
    m_view->setItemDelegateForColumn(8, statusDelegate);
    m_view->setItemDelegateForColumn(9, actionsDelegate);
    v->addWidget(m_view, 1);

    auto *trendTitle = new QLabel("Statistiques");
    trendTitle->setObjectName("section");
    v->addWidget(trendTitle);
    auto *trendCaption = new QLabel("Évolution des employés");
    trendCaption->setAlignment(Qt::AlignCenter);
    trendCaption->setStyleSheet("color:#526071; font-size:11px;");
    v->addWidget(trendCaption);
    v->addWidget(new EmployeeTrendChart, 0);

    auto* kpis = new QHBoxLayout;
    m_kTotal    = addKpi(kpis, "Total employés");
    m_kActive   = addKpi(kpis, "Jury IA (EV01)");
    m_kInactive = addKpi(kpis, "Jury Web (EV02)");
    m_kAdmins   = addKpi(kpis, "IoT + Sécu (EV03/04)");
    v->addLayout(kpis);

    QAction* undoAct = m_stack->createUndoAction(this, "Annuler");
    undoAct->setShortcut(QKeySequence::Undo);
    QAction* redoAct = m_stack->createRedoAction(this, "Rétablir");
    redoAct->setShortcut(QKeySequence::Redo);
    addAction(undoAct);
    addAction(redoAct);

    connect(undoBtn,   &QPushButton::clicked,    m_stack, &QUndoStack::undo);
    connect(m_search,  &QLineEdit::textChanged,  m_proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(addBtn,    &QPushButton::clicked,    this,    &EmployeWindow::onAdd);
    connect(pdfBtn,    &QPushButton::clicked,    this,    &EmployeWindow::exportPdf);
    connect(m_themeBtn,&QPushButton::clicked,    this,    &EmployeWindow::toggleTheme);
    connect(m_model,   &EmployeeModel::changed,  this,    &EmployeWindow::refreshKpis);

    connect(m_view, &QTableView::clicked, this, [this](const QModelIndex& index) {
        if (index.column() < 9)
            onView(index.data(EmployeeModel::IdRole).toString());
    });
    connect(actionsDelegate, &ActionsDelegate::viewClicked,   this, &EmployeWindow::onView,   Qt::QueuedConnection);
    connect(actionsDelegate, &ActionsDelegate::editClicked,   this, &EmployeWindow::onEdit,   Qt::QueuedConnection);
    connect(actionsDelegate, &ActionsDelegate::deleteClicked, this, &EmployeWindow::onDelete, Qt::QueuedConnection);

    applyTheme(true);
    refreshKpis();
}

// ──────────────────────────────────────────────────────────────────
QLabel* EmployeWindow::addKpi(QHBoxLayout* layout, const QString& title)
{
    auto* card  = new QFrame;
    card->setObjectName("kpi");
    auto* l     = new QVBoxLayout(card);
    auto* t     = new QLabel(title);
    t->setObjectName("kpiTitle");
    auto* value = new QLabel("0");
    value->setObjectName("kpiVal");
    l->addWidget(t);
    l->addWidget(value);
    layout->addWidget(card);
    return value;
}

// ──────────────────────────────────────────────────────────────────
//  CRUD — Oracle-backed operations
// ──────────────────────────────────────────────────────────────────
void EmployeWindow::onAdd()
{
    // Suggest next Oracle-style ID ("009", "010", ...)
    QString suggestedId = "009";
    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    if (db.isOpen()) {
        QSqlQuery q(db);
        if (q.exec("SELECT MAX(TO_NUMBER(TRIM(ID_EMPLOYE))) FROM EMPLOYES "
                   "WHERE REGEXP_LIKE(TRIM(ID_EMPLOYE), '^[0-9]+$')") && q.next()) {
            int maxId = q.value(0).toInt();
            suggestedId = QString("%1").arg(maxId + 1, 3, 10, QChar('0'));
        }
    }

    EmployeeFormDialog dialog(this, nullptr, suggestedId);
    if (dialog.exec() != QDialog::Accepted) return;

    Employee e = dialog.result();   // ID comes from the form's m_id field

    if (db.isOpen()) {
        QSqlQuery ins(db);
        ins.prepare("INSERT INTO EMPLOYES (ID_EMPLOYE, NOM, PRENOM, MOT_DE_PASSE, POSTE) "
                    "VALUES (:id, :nom, :prenom, :pwd, :poste)");
        ins.bindValue(":id",     e.id_emp);
        ins.bindValue(":nom",    e.nom);
        ins.bindValue(":prenom", e.prenom);
        ins.bindValue(":pwd",    e.mot_de_passe);
        ins.bindValue(":poste",  e.poste);

        if (!ins.exec()) {
            QMessageBox::critical(this, "Erreur SQL — INSERT échoué",
                "L'employé n'a PAS été enregistré en base Oracle.\n\n"
                "Raison : " + ins.lastError().text() +
                "\n\nVérifiez que l'ID «" + e.id_emp + "» n'existe pas déjà.");
            return;
        }
        m_model->loadFromDatabase();
        statusBar()->showMessage("✓ Employé «" + e.prenom + " " + e.nom +
                                 "» ajouté en base Oracle (ID: " + e.id_emp + ").", 5000);
    } else {
        // Offline mode — in-memory only, warn the user
        m_stack->push(new AddEmployeeCmd(m_model, e));
        QMessageBox::warning(this, "Mode hors-ligne",
            "Oracle non connecté. L'employé est ajouté localement seulement\n"
            "et sera perdu à la fermeture de l'application.");
    }
}

void EmployeWindow::onEdit(const QString& id)
{
    const int row = m_model->rowOf(id);
    if (row < 0) return;
    const Employee oldEmployee = m_model->at(row);

    EmployeeFormDialog dialog(this, &oldEmployee);
    if (dialog.exec() != QDialog::Accepted) return;

    Employee updated = dialog.result();   // ID stays locked to oldEmployee.id_emp

    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("UPDATE EMPLOYES SET NOM=:nom, PRENOM=:prenom, "
                  "MOT_DE_PASSE=:pwd, POSTE=:poste "
                  "WHERE ID_EMPLOYE=:id");
        q.bindValue(":nom",   updated.nom);
        q.bindValue(":prenom",updated.prenom);
        q.bindValue(":pwd",   updated.mot_de_passe);
        q.bindValue(":poste", updated.poste);
        q.bindValue(":id",    updated.id_emp);

        if (!q.exec()) {
            QMessageBox::warning(this, "Erreur SQL",
                "Impossible de modifier l'employé :\n" + q.lastError().text());
            return;
        }
        m_model->loadFromDatabase();
        statusBar()->showMessage("Employé modifié avec succès.", 4000);
    } else {
        m_stack->push(new EditEmployeeCmd(m_model, oldEmployee, updated));
    }
}

void EmployeWindow::onDelete(const QString& id)
{
    const int row = m_model->rowOf(id);
    if (row < 0) return;
    const Employee employee = m_model->at(row);

    if (QMessageBox::question(this, "Supprimer l'employé",
                              "Supprimer " + employee.prenom + " " + employee.nom + " ?",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM EMPLOYES WHERE ID_EMPLOYE=:id");
        q.bindValue(":id", id);

        if (!q.exec()) {
            // Handle FK constraint from GERER table gracefully
            const QString errText = q.lastError().text();
            if (errText.contains("ORA-02292") || errText.contains("integrity constraint")) {
                QMessageBox::warning(this, "Contrainte de clé étrangère",
                    "Cet employé est référencé dans la table GERER.\n"
                    "Supprimez d'abord les évaluations associées.");
            } else {
                QMessageBox::warning(this, "Erreur SQL",
                    "Impossible de supprimer l'employé :\n" + errText);
            }
            return;
        }
        m_model->loadFromDatabase();
        statusBar()->showMessage("Employé supprimé.", 4000);
    } else {
        m_stack->push(new DeleteEmployeeCmd(m_model, employee));
        statusBar()->showMessage("Employé supprimé. Ctrl+Z pour annuler.", 5000);
    }
}

void EmployeWindow::onView(const QString& id)
{
    const int row = m_model->rowOf(id);
    if (row < 0) return;
    EmployeeProfileDialog(m_model->at(row), this).exec();
}

// ──────────────────────────────────────────────────────────────────
//  KPI — Oracle GROUP BY query
// ──────────────────────────────────────────────────────────────────
void EmployeWindow::refreshKpis()
{
    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    if (db.isOpen()) {
        // Total
        {
            QSqlQuery q(db);
            if (q.exec("SELECT COUNT(*) FROM EMPLOYES") && q.next())
                m_kTotal->setText(q.value(0).toString());
        }
        // Per-poste counts
        int ev01 = 0, ev02 = 0, ev0304 = 0;
        {
            QSqlQuery q(db);
            if (q.exec("SELECT POSTE, COUNT(*) FROM EMPLOYES GROUP BY POSTE")) {
                while (q.next()) {
                    const QString poste = q.value(0).toString();
                    const int     cnt   = q.value(1).toInt();
                    if      (poste == "EV01") ev01   += cnt;
                    else if (poste == "EV02") ev02   += cnt;
                    else if (poste == "EV03" || poste == "EV04") ev0304 += cnt;
                }
            }
        }
        m_kActive  ->setText(QString::number(ev01));
        m_kInactive->setText(QString::number(ev02));
        m_kAdmins  ->setText(QString::number(ev0304));
    } else {
        // Offline: derive from in-memory model
        int total = m_model->rowCount();
        int ev01 = 0, ev02 = 0, ev0304 = 0;
        for (const Employee& e : m_model->all()) {
            if      (e.poste == "EV01") ++ev01;
            else if (e.poste == "EV02") ++ev02;
            else if (e.poste == "EV03" || e.poste == "EV04") ++ev0304;
        }
        m_kTotal   ->setText(QString::number(total));
        m_kActive  ->setText(QString::number(ev01));
        m_kInactive->setText(QString::number(ev02));
        m_kAdmins  ->setText(QString::number(ev0304));
    }
}

// ──────────────────────────────────────────────────────────────────
//  PDF export (unchanged)
// ──────────────────────────────────────────────────────────────────
void EmployeWindow::exportPdf()
{
    const QString file = QFileDialog::getSaveFileName(
        this, "Exporter les employés en PDF", "employes.pdf", "PDF (*.pdf)");
    if (file.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);

    QTextDocument doc;
    QString html;
    html += "<html><head><meta charset='utf-8'><style>"
            "body{font-family:Arial,sans-serif;color:#172033;}"
            ".head{border-bottom:2px solid #1f6feb;padding-bottom:10px;margin-bottom:12px;}"
            ".logo{font-size:20pt;font-weight:bold;color:#1f6feb;}"
            ".title{font-size:18pt;font-weight:bold;margin-top:3px;}"
            ".sub{color:#64748b;font-size:9pt;}"
            "table{width:100%;border-collapse:collapse;font-size:7.3pt;}"
            "th{background:#eef4ff;color:#1f6feb;padding:5px;border:1px solid #dbe3ef;}"
            "td{padding:5px;border:1px solid #dbe3ef;white-space:nowrap;}"
            ".active{color:#15803d;font-weight:bold;} .inactive{color:#be123c;font-weight:bold;}"
            ".footer{color:#64748b;font-size:8pt;margin-top:8px;}"
            "</style></head><body>";
    html += "<div class='head'><div class='logo'>◆ Devena</div>"
            "<div class='title'>Liste des employés</div>"
            "<div class='sub'>Gestion des employés du hackathon</div></div>";
    html += "<table><tr>"
            "<th>ID</th><th>Nom</th><th>Prénom</th><th>Email</th><th>Téléphone</th>"
            "<th>Date embauche</th><th>Salaire</th><th>Poste</th><th>Statut</th></tr>";

    for (const Employee& e : m_model->all()) {
        const QString statusClass = e.statut == "Actif" ? "active" : "inactive";
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>"
                        "<td>%6</td><td>%7 DT</td><td>%8</td><td class='%9'>%10</td></tr>")
                    .arg(e.id_emp.toHtmlEscaped(), e.nom.toHtmlEscaped(), e.prenom.toHtmlEscaped(),
                         e.email.toHtmlEscaped(), e.telephone.toHtmlEscaped(), e.date_embauche.toHtmlEscaped(),
                         QString::number(e.salaire, 'f', 2), e.poste.toHtmlEscaped(), statusClass,
                         e.statut.toHtmlEscaped());
    }
    html += QString("<</table><div class='footer'>Total : %1 employé(s) — Export Devena</div>")
                .arg(m_model->rowCount());
    html += "</body></html>";

    doc.setHtml(html);
    doc.print(&printer);
    statusBar()->showMessage("PDF exporté : " + file, 5000);
}

// ──────────────────────────────────────────────────────────────────
void EmployeWindow::toggleTheme()         { applyTheme(!m_dark); }

void EmployeWindow::applyTheme(bool dark)
{
    m_dark = dark;
    Theme::accent   = QColor(dark ? "#08D5E8" : "#1f6feb");
    Theme::accentBg = QColor(dark ? "#092B3A" : "#e3eefe");
    // Style applique a ce module : le menu de gauche a ainsi le meme aspect
    // que celui du tableau de bord et des equipes.
    setStyleSheet(makeStyle(dark));
    m_themeBtn->setText(dark ? "☀ Mode clair" : "🌙 Mode sombre");
    m_view->viewport()->update();
}

// ──────────────────────────────────────────────────────────────────
QWidget* EmployeWindow::buildSidebar()
{
    auto* bar = new QFrame;
    bar->setObjectName("sidebar");
    bar->setFixedWidth(236);
    auto* l = new QVBoxLayout(bar);
    l->setContentsMargins(13, 22, 13, 18);
    l->setSpacing(8);

    auto* logo   = new QLabel("◆ Devena");
    logo->setObjectName("logo");
    auto* slogan = new QLabel("Connecter les talents,\nconstruire l'avenir");
    slogan->setObjectName("sub");
    l->addWidget(logo);
    l->addWidget(slogan);
    l->addSpacing(18);

    const QStringList items = {"Tableau de bord", "Employés", "Participants",
                               "Équipes", "Projets", "Évaluations", "Paramètres"};
    for (const QString& item : items) {
        auto* button = new QPushButton(item);
        button->setObjectName("nav");
        button->setMinimumHeight(44);
        button->setCursor(Qt::PointingHandCursor);
        button->setProperty("active", item == "Employés");
        connect(button, &QPushButton::clicked, this, [this, item] {
            if (item == "Tableau de bord") emit navigateDashboard();
            else if (item != "Employés" && item != "Paramètres") emit navigateModule(item);
            else if (item != "Employés")
                statusBar()->showMessage("Module « " + item + " » : bientôt disponible", 3000);
        });
        l->addWidget(button);
    }
    l->addStretch();

    auto* foot = new QLabel("Hack today\nBuild tomorrow");
    foot->setObjectName("sub");
    foot->setAlignment(Qt::AlignCenter);
    l->addWidget(foot);
    return bar;
}
