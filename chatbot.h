
#ifndef CHATBOT_H
#define CHATBOT_H

#include <QDialog>
#include <QString>

class QTextBrowser;
class QLineEdit;
class QPushButton;

class ChatbotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatbotDialog(QWidget *parent = nullptr);

private slots:
    void envoyerMessage();

private:
    QString traiterQuestion(const QString &question);
    QString compterParticipants();
    QString compterParSexe();
    QString ageMoyen();
    QString compterParCompetence(const QString &competence);
    QString competencePlusFrequente();
    QString rechercherParticipant(const QString &question);
    QString listeParticipants();

    QTextBrowser *m_chat;
    QLineEdit *m_question;
    QPushButton *m_envoyer;
};

#endif
