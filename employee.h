#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include <QDialog>
#include <QList>
#include <QMainWindow>
#include <QStyledItemDelegate>
#include <QString>
#include <QUndoCommand>

class QDateEdit;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QTableView;
class QUndoStack;
class QDoubleSpinBox;
class QComboBox;

QString makeStyle(bool dark);

struct Theme {
    static inline QColor accent{"#08D5E8"};
    static inline QColor accentBg{"#092B3A"};
};

struct Employee {
    QString id_emp;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString date_embauche;
    double  salaire = 0.0;
    QString poste;
    QString mot_de_passe;
    QString photo_profil;
    QString statut;
};

class EmployeeModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Role { IdRole = Qt::UserRole + 1 };
    explicit EmployeeModel(QObject* parent = nullptr);

    int     rowCount   (const QModelIndex& parent = {}) const override;
    int     columnCount(const QModelIndex& parent = {}) const override;
    QVariant data      (const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const QList<Employee>& all()  const { return m_data; }
    const Employee&        at(int row) const { return m_data.at(row); }
    int     rowOf (const QString& id) const;
    QString nextId();
    void    insertAt(int row, const Employee& e);
    void    removeAt(int row);
    void    replace (int row, const Employee& e);

    /** Load / reload data from Oracle EMPLOYES table. */
    void loadFromDatabase();

    /** Fallback: populate with hardcoded demo rows (offline mode). */
    void loadSampleData();

signals:
    void changed();

private:
    QList<Employee> m_data;
    int             m_next = 1;
};

class AddEmployeeCmd : public QUndoCommand {
public:
    AddEmployeeCmd(EmployeeModel* model, const Employee& employee);
    void redo() override;
    void undo() override;
private:
    EmployeeModel* m_model;
    Employee       m_employee;
    int            m_row;
};

class DeleteEmployeeCmd : public QUndoCommand {
public:
    DeleteEmployeeCmd(EmployeeModel* model, const Employee& employee);
    void redo() override;
    void undo() override;
private:
    EmployeeModel* m_model;
    Employee       m_employee;
    int            m_row;
};

class EditEmployeeCmd : public QUndoCommand {
public:
    EditEmployeeCmd(EmployeeModel* model, const Employee& oldEmployee, const Employee& newEmployee);
    void redo() override;
    void undo() override;
private:
    EmployeeModel* m_model;
    Employee       m_oldEmployee;
    Employee       m_newEmployee;
};

class ActionsDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;
signals:
    void viewClicked  (const QString& id);
    void editClicked  (const QString& id);
    void deleteClicked(const QString& id);
private:
    static QRect buttonRect(const QRect& cell, int index);
};

class StatusDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void  paint   (QPainter* painter, const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;
};

class EmployeeProfileDialog : public QDialog {
public:
    EmployeeProfileDialog(const Employee& employee, QWidget* parent = nullptr);
};

class EmployeeFormDialog : public QDialog {
public:
    explicit EmployeeFormDialog(QWidget* parent, const Employee* employee = nullptr,
                               const QString& suggestedId = QString());
    Employee result() const;   // ID taken from m_id field

private:
    void validate();
    void choosePhoto();

    QLineEdit*     m_id;         // ID_EMPLOYE — editable on Add, read-only on Edit
    QLineEdit*     m_nom;
    QLineEdit*     m_prenom;
    QLineEdit*     m_email;
    QLineEdit*     m_telephone;
    QDateEdit*     m_date;
    QDoubleSpinBox*m_salaire;
    QComboBox*     m_poste;
    QLineEdit*     m_password;
    QLineEdit*     m_photo;
    QComboBox*     m_statut;
    QLabel*        m_error;
};

class EmployeWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit EmployeWindow(QWidget* parent = nullptr);
signals:
    void navigateDashboard();
    void navigateModule(const QString& module);   // "Équipes" ou "Projets"

private slots:
    void onAdd();
    void onEdit  (const QString& id);
    void onDelete(const QString& id);
    void onView  (const QString& id);
    void exportPdf();
    void toggleTheme();
    void refreshKpis();

private:
    void   applyTheme(bool dark);
    QLabel* addKpi(QHBoxLayout* layout, const QString& title);
    QWidget* buildSidebar();

    EmployeeModel*          m_model;
    QSortFilterProxyModel*  m_proxy;
    QTableView*             m_view;
    QUndoStack*             m_stack;
    QLineEdit*              m_search;
    QPushButton*            m_themeBtn;

    // KPI labels (now mapped to Oracle postes)
    QLabel* m_kTotal;    // Total employés
    QLabel* m_kActive;   // EV01 — Jury IA
    QLabel* m_kInactive; // EV02 — Jury Web
    QLabel* m_kAdmins;   // EV03 + EV04 — IoT + Sécu

    bool m_dark = true;
};
