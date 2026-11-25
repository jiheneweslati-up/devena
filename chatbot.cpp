
#include "chatbot.h"

#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QMap>
#include <QMessageBox>
#include <QDateTime>
#include <QStringList>
#include <QTextCursor>

ChatbotDialog::ChatbotDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Devena AI - Assistant Participants");
    resize(650, 550);

    auto *layout = new QVBoxLayout(this);

    auto *titre = new QLabel("◆ Devena AI");
    titre->setStyleSheet(
        "font-size:22px;font-weight:bold;color:#06b6d4;"
        "padding:10px;"
        );

    auto *description = new QLabel(
        "Assistant intelligent connecté à la base Oracle des participants."
        );
    description->setStyleSheet("color:#8fa0b6;padding-bottom:8px;");

    m_chat = new QTextBrowser(this);
    m_chat->setStyleSheet(
        "QTextBrowser {"
        "background:#0b1329;"
        "color:white;"
        "border:1px solid #1e294b;"
        "border-radius:8px;"
        "padding:10px;"
        "}"
        );

    m_question = new QLineEdit(this);
    m_question->setPlaceholderText(
        "Exemple : Combien de participants maîtrisent C++ ?"
        );

    m_question->setStyleSheet(
        "QLineEdit {"
        "background:#0f1c3f;"
        "color:white;"
        "border:1px solid #1e294b;"
        "border-radius:7px;"
        "padding:12px;"
        "}"
        );

    m_envoyer = new QPushButton("Envoyer", this);
    m_envoyer->setStyleSheet(
        "QPushButton {"
        "background:#06b6d4;"
        "color:#0b1329;"
        "font-weight:bold;"
        "border:none;"
        "border-radius:7px;"
        "padding:10px 18px;"
        "}"
        "QPushButton:hover {background:#38bdf8;}"
        );

    auto *bottom = new QHBoxLayout;
    bottom->addWidget(m_question, 1);
    bottom->addWidget(m_envoyer);

    layout->addWidget(titre);
    layout->addWidget(description);
    layout->addWidget(m_chat, 1);
    layout->addLayout(bottom);

    connect(m_envoyer, &QPushButton::clicked,
            this, &ChatbotDialog::envoyerMessage);

    connect(m_question, &QLineEdit::returnPressed,
            this, &ChatbotDialog::envoyerMessage);

    m_chat->append(
        "<b style='color:#06b6d4'>Devena AI :</b><br>"
        "Bonjour ! Je peux analyser les données des participants "
        "enregistrées dans Oracle.<br><br>"
        "<b>Exemples :</b><br>"
        "• Combien de participants ?<br>"
        "• Combien maîtrisent C++ ?<br>"
        "• Quelle est la compétence la plus fréquente ?<br>"
        "• Donne-moi la répartition par sexe.<br>"
        "• Quel est l'âge moyen ?<br>"
        "• Recherche PA01."
        );
}

void ChatbotDialog::envoyerMessage()
{
    const QString question = m_question->text().trimmed();

    if (question.isEmpty())
        return;

    m_chat->append(
        "<br><b style='color:#38bdf8'>Vous :</b><br>"
        + question.toHtmlEscaped()
        );

    m_question->clear();

    const QString reponse = traiterQuestion(question);

    m_chat->append(
        "<br><b style='color:#06b6d4'>Devena AI :</b><br>"
        + reponse.toHtmlEscaped().replace("\n", "<br>")
        );

    m_chat->moveCursor(QTextCursor::End);
}


QString ChatbotDialog::traiterQuestion(const QString &question)
{
    const QString q = question.toLower().simplified();

    // 1. Questions sur la compétence la plus fréquente
    if (q.contains("plus fréquente") ||
        q.contains("plus frequente") ||
        q.contains("compétence dominante") ||
        q.contains("competence dominante") ||
        q.contains("compétence la plus") ||
        q.contains("competence la plus"))
        return competencePlusFrequente();

    // 2. Questions sur une compétence précise
    const QStringList motsCompetences = {
        "c++", "python", "java", "qt", "sql", "oracle",
        "figma", "tensorflow", "pandas", "scrum",
        "réseaux", "reseaux", "deep learning",
        "machine learning", "javascript", "html", "css",
        "php", "c#", "arduino", "ia", "data"
    };

    for (const QString &competence : motsCompetences) {
        if (q.contains(competence))
            return compterParCompetence(competence);
    }

    // 3. Recherche d'un participant
    if (q.contains("cherche") || q.contains("recherche") ||
        q.contains("trouve") || q.contains("informations sur"))
        return rechercherParticipant(question);

    // 4. Répartition par sexe
    if (q.contains("répartition") || q.contains("repartition") ||
        q.contains("homme") || q.contains("femme") ||
        q.contains("sexe"))
        return compterParSexe();

    // 5. Âge moyen
    if (q.contains("âge moyen") || q.contains("age moyen") ||
        q.contains("moyenne d'âge") || q.contains("moyenne d age"))
        return ageMoyen();

    // 6. Liste des participants
    if (q.contains("liste") && q.contains("participant"))
        return listeParticipants();

    // 7. Nombre total de participants
    if (q.contains("combien") &&
        (q.contains("participant") || q.contains("inscrit")))
        return compterParticipants();

    return "Je n'ai pas compris cette question.\n"
           "Essaie une question sur le nombre de participants, "
           "les compétences, le sexe, l'âge ou la recherche.";
}


QString ChatbotDialog::compterParticipants()
{
    QSqlQuery query;

    if (!query.exec("SELECT COUNT(*) FROM PARTICIPANTS"))
        return "Erreur Oracle : " + query.lastError().text();

    if (query.next())
        return "La base Oracle contient "
               + query.value(0).toString()
               + " participant(s).";

    return "Impossible de récupérer le nombre de participants.";
}

QString ChatbotDialog::compterParSexe()
{
    QSqlQuery query;

    if (!query.exec(
            "SELECT SEXE, COUNT(*) "
            "FROM PARTICIPANTS "
            "GROUP BY SEXE"))
        return "Erreur Oracle : " + query.lastError().text();

    QString resultat = "Répartition des participants :\n";

    while (query.next()) {
        QString sexe = query.value(0).toString();

        if (sexe.isEmpty())
            sexe = "Non renseigné";

        resultat += sexe + " : "
                    + query.value(1).toString()
                    + " participant(s)\n";
    }

    return resultat;
}

QString ChatbotDialog::ageMoyen()
{
    QSqlQuery query;

    if (!query.exec(
            "SELECT ROUND(AVG(AGE), 2) "
            "FROM PARTICIPANTS WHERE AGE > 0"))
        return "Erreur Oracle : " + query.lastError().text();

    if (query.next() && !query.value(0).isNull())
        return "L'âge moyen des participants est de "
               + query.value(0).toString()
               + " ans.";

    return "Aucun âge valide n'est enregistré.";
}

QString ChatbotDialog::compterParCompetence(
    const QString &competence)
{
    QSqlQuery query;

    if (!query.exec(
            "SELECT COMPETENCES FROM PARTICIPANTS"))
        return "Erreur Oracle : " + query.lastError().text();

    int nombre = 0;
    QStringList personnes;

    while (query.next()) {
        const QStringList competences =
            query.value(0).toString().split(
                ',', Qt::SkipEmptyParts);

        bool trouve = false;

        for (const QString &c : competences) {
            if (c.trimmed().compare(
                    competence, Qt::CaseInsensitive) == 0) {
                trouve = true;
                break;
            }
        }

        if (trouve)
            ++nombre;
    }

    return QString::number(nombre)
           + " participant(s) possèdent la compétence "
           + competence + ".";
}

QString ChatbotDialog::competencePlusFrequente()
{
    QSqlQuery query;

    if (!query.exec(
            "SELECT COMPETENCES FROM PARTICIPANTS"))
        return "Erreur Oracle : " + query.lastError().text();

    QMap<QString, int> frequences;

    while (query.next()) {
        const QStringList competences =
            query.value(0).toString().split(
                ',', Qt::SkipEmptyParts);

        for (QString c : competences) {
            c = c.trimmed();

            if (!c.isEmpty())
                frequences[c]++;
        }
    }

    if (frequences.isEmpty())
        return "Aucune compétence n'est enregistrée.";

    QString meilleure;
    int maximum = 0;

    for (auto it = frequences.cbegin();
         it != frequences.cend(); ++it) {
        if (it.value() > maximum) {
            maximum = it.value();
            meilleure = it.key();
        }
    }

    return "La compétence la plus fréquente est "
           + meilleure + " avec "
           + QString::number(maximum)
           + " occurrence(s).";
}

QString ChatbotDialog::rechercherParticipant(
    const QString &question)
{
    QRegularExpression idRegex(
        "\\b(?:PA|P)\\d+\\b",
        QRegularExpression::CaseInsensitiveOption
        );

    const auto match = idRegex.match(question);

    QSqlQuery query;

    if (match.hasMatch()) {
        query.prepare(
            "SELECT ID_PARTICIPANT, NOM, PRENOM, EMAIL, "
            "COMPETENCES, AGE, SEXE "
            "FROM PARTICIPANTS "
            "WHERE UPPER(ID_PARTICIPANT) = UPPER(:id)"
            );

        query.bindValue(":id", match.captured(0));
    }
    else {
        QString nom = question;

        const QStringList prefixes = {
            "cherche", "recherche", "trouve",
            "informations sur"
        };

        for (const QString &prefix : prefixes) {
            const int pos = nom.toLower().indexOf(prefix);

            if (pos >= 0) {
                nom = nom.mid(pos + prefix.length()).trimmed();
                break;
            }
        }

        nom.remove('!');
        nom.remove('?');
        nom.remove('.');

        if (nom.trimmed().isEmpty())
            return "Indique un identifiant ou un nom.";

        query.prepare(
            "SELECT ID_PARTICIPANT, NOM, PRENOM, EMAIL, "
            "COMPETENCES, AGE, SEXE "
            "FROM PARTICIPANTS "
            "WHERE UPPER(NOM) LIKE UPPER(:nom) "
            "OR UPPER(PRENOM) LIKE UPPER(:nom)"
            );

        query.bindValue(":nom", "%" + nom + "%");
    }

    if (!query.exec())
        return "Erreur Oracle : " + query.lastError().text();

    if (!query.next())
        return "Aucun participant trouvé.";

    QString resultat = "Participant trouvé :\n";

    do {
        resultat +=
            "ID : " + query.value(0).toString() + "\n"
                                                  "Nom : " + query.value(1).toString() + "\n"
                                          "Prénom : " + query.value(2).toString() + "\n"
                                          "Email : " + query.value(3).toString() + "\n"
                                          "Compétences : " + query.value(4).toString() + "\n"
                                          "Âge : " + query.value(5).toString() + "\n"
                                          "Sexe : " + query.value(6).toString() + "\n\n";

    } while (query.next());

    return resultat;
}

QString ChatbotDialog::listeParticipants()
{
    QSqlQuery query;

    if (!query.exec(
            "SELECT ID_PARTICIPANT, NOM, PRENOM "
            "FROM PARTICIPANTS WHERE ROWNUM <= 20"))
        return "Erreur Oracle : " + query.lastError().text();

    QString resultat = "Liste des participants (20 maximum) :\n";

    while (query.next()) {
        resultat += query.value(0).toString()
        + " - "
            + query.value(2).toString()
            + " "
            + query.value(1).toString()
            + "\n";
    }

    return resultat;
}
