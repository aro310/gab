#pragma once

#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Transaction { std::string date, type; double montant, solde; };
struct OpResult    { bool ok; std::string msg; };

namespace {
    std::string nowStr() {
        std::time_t t = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
        return buf;
    }

    void appendCSV(const Transaction& tx, const std::string& path) {
        std::ofstream f(path, std::ios::app);
        if (f) f << tx.date << "," << tx.type << "," << tx.montant << "," << tx.solde << "\n";
    }

    std::vector<Transaction> loadCSV(const std::string& path) {
        std::vector<Transaction> h;
        std::ifstream f(path);
        if (!f) return h;
        std::string line;
        while (std::getline(f, line)) {
            std::istringstream ss(line);
            Transaction tx; std::string m, s;
            if (!std::getline(ss, tx.date, ',') || !std::getline(ss, tx.type, ',') ||
                !std::getline(ss, m, ',')        || !std::getline(ss, s, ',')) continue;
            try { tx.montant = std::stod(m); tx.solde = std::stod(s); }
            catch (...) { continue; }
            h.push_back(tx);
        }
        return h;
    }
}

class CompteBancaire {
    std::string titulaire;
    double solde, soldeMin, limiteSession, retraitSession;
    std::string csvPath;
    std::vector<Transaction> historique;

    void log(const std::string& type, double montant) {
        Transaction tx{ nowStr(), type, montant, solde };
        historique.push_back(tx);
        appendCSV(tx, csvPath);
    }

public:
    CompteBancaire(std::string nom, double initial,
                   double sMin = 0.0, double limSess = 0.0,
                   std::string path = "historique.csv")
        : titulaire(std::move(nom)), solde(initial),
          soldeMin(sMin), limiteSession(limSess),
          retraitSession(0.0), csvPath(std::move(path))
    {
        if (initial < 0.0) throw std::invalid_argument("Solde initial negatif.");
        if (sMin    < 0.0) throw std::invalid_argument("Solde minimum negatif.");
        if (limSess < 0.0) throw std::invalid_argument("Limite session negative.");
        historique = loadCSV(csvPath);
    }

    virtual ~CompteBancaire() = default;

    double getSolde()          const { return solde; }
    double getSoldeMin()       const { return soldeMin; }
    double getLimiteSession()  const { return limiteSession; }
    double getRetraitSession() const { return retraitSession; }
    const std::string& getTitulaire()           const { return titulaire; }
    const std::vector<Transaction>& getHistorique() const { return historique; }

    virtual std::string getType() const { return "standard"; }

    OpResult deposer(double montant, const std::string& type = "DEPOT") {
        if (montant <= 0.0) return {false, "montant_invalide"};
        solde += montant;
        log(type, montant);
        return {true, "ok"};
    }

    OpResult retirer(double montant) {
        if (montant <= 0.0)                                                      return {false, "montant_invalide"};
        if (montant > solde)                                                     return {false, "solde_insuffisant"};
        if ((solde - montant) < soldeMin)                                        return {false, "solde_min"};
        if (limiteSession > 0.0 && (retraitSession + montant) > limiteSession)   return {false, "limite_session"};
        solde -= montant;
        retraitSession += montant;
        log("RETRAIT", montant);
        return {true, "ok"};
    }
};

class CompteEpargne : public CompteBancaire {
    double taux;

public:
    CompteEpargne(std::string nom, double initial, double t,
                  double sMin = 0.0, double limSess = 0.0,
                  std::string path = "historique.csv")
        : CompteBancaire(std::move(nom), initial, sMin, limSess, std::move(path)), taux(t)
    {
        if (taux < 0.0) throw std::invalid_argument("Taux negatif.");
    }

    double getTaux()              const { return taux; }
    std::string getType()         const override { return "epargne"; }
    OpResult appliquerInterets()  { return deposer(getSolde() * taux / 100.0, "INTERET"); }
};