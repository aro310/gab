#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <memory>
#include "compte.hpp"

class AtmBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString      titulaire  READ titulaire  NOTIFY changed)
    Q_PROPERTY(double       solde      READ solde      NOTIFY changed)
    Q_PROPERTY(double       soldeMin   READ soldeMin   NOTIFY changed)
    Q_PROPERTY(double       limSession READ limSession NOTIFY changed)
    Q_PROPERTY(double       retSession READ retSession NOTIFY changed)
    Q_PROPERTY(QString      type       READ type       NOTIFY changed)
    Q_PROPERTY(QVariantList historique READ historique NOTIFY changed)

    std::unique_ptr<CompteEpargne> m;

public:
    explicit AtmBridge(QObject* parent = nullptr)
        : QObject(parent),
          m(std::make_unique<CompteEpargne>("Ahmed Benali", 50000.0, 3.5, 10000.0, 20000.0))
    {}

    QString      titulaire()  const { return QString::fromStdString(m->getTitulaire()); }
    double       solde()      const { return m->getSolde(); }
    double       soldeMin()   const { return m->getSoldeMin(); }
    double       limSession() const { return m->getLimiteSession(); }
    double       retSession() const { return m->getRetraitSession(); }

    QString type() const {
        if (m->getType() == "epargne")
            return QString("Compte epargne  (%1%)").arg(m->getTaux());
        return "Compte standard";
    }

    QVariantList historique() const {
        QVariantList out;
        for (const auto& tx : m->getHistorique()) {
            QVariantMap row;
            row["date"]    = QString::fromStdString(tx.date);
            row["type"]    = QString::fromStdString(tx.type);
            row["montant"] = tx.montant;
            row["solde"]   = tx.solde;
            out.append(row);
        }
        return out;
    }

    Q_INVOKABLE QString deposer(double v) {
        auto r = m->deposer(v);
        if (r.ok) emit changed();
        return QString::fromStdString(r.msg);
    }

    Q_INVOKABLE QString retirer(double v) {
        auto r = m->retirer(v);
        if (r.ok) emit changed();
        return QString::fromStdString(r.msg);
    }

    Q_INVOKABLE QString appliquerInterets() {
        auto r = m->appliquerInterets();
        if (r.ok) emit changed();
        return QString::fromStdString(r.msg);
    }

signals:
    void changed();
};