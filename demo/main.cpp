#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// ═══════════════════════════════════════════════════════
//  Helpers système
// ═══════════════════════════════════════════════════════

static void enableAnsi() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m;
    GetConsoleMode(h, &m);
    SetConsoleMode(h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
}

static int langueActuelle = 1;
static std::string txt(const std::string& fr, const std::string& mg) {
    return langueActuelle == 2 ? mg : fr;
}

static std::string now() {
    std::time_t t = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

static std::string fmtNum(double v) {
    bool neg = v < 0;
    std::string s = std::to_string((long long)(neg ? -v : v));
    for (int i = (int)s.size() - 3; i > 0; i -= 3)
        s.insert(i, " ");
    return (neg ? "-" : "") + s;
}

static std::string rep(const std::string& s, int n) {
    std::string r;
    r.reserve(s.size() * n);
    for (int i = 0; i < n; ++i) r += s;
    return r;
}

namespace ansi {
    const std::string rst  = "\033[0m";
    const std::string bold = "\033[1m";
    const std::string red  = "\033[31m";
    const std::string grn  = "\033[32m";
    const std::string yel  = "\033[33m";
    const std::string cyn  = "\033[36m";
    const std::string mag  = "\033[35m";
    const std::string blu  = "\033[34m";
}

static const int W = 60;
static void cls() { std::cout << "\033[2J\033[H" << std::flush; }

static int visLen(const std::string& s) {
    int n = 0; bool esc = false;
    for (unsigned char c : s) {
        if (c == '\033') { esc = true; continue; }
        if (esc)         { if (c == 'm') esc = false; continue; }
        if ((c & 0xC0) != 0x80) ++n;
    }
    return n;
}

static std::string fit(const std::string& s, int w) {
    int vl = visLen(s);
    return vl >= w ? s : s + std::string(w - vl, ' ');
}

static void top() { std::cout << ansi::cyn << "\xe2\x95\x94" << rep("\xe2\x95\x90", W) << "\xe2\x95\x97\n" << ansi::rst; }
static void bot() { std::cout << ansi::cyn << "\xe2\x95\x9a" << rep("\xe2\x95\x90", W) << "\xe2\x95\x9d\n" << ansi::rst; }
static void sep() { std::cout << ansi::cyn << "\xe2\x95\xa0" << rep("\xe2\x95\x90", W) << "\xe2\x95\xa3\n" << ansi::rst; }

static void row(const std::string& s, const std::string& clr = "") {
    std::cout << ansi::cyn << "\xe2\x95\x91 " << ansi::rst
              << clr << fit(s, W - 2) << ansi::rst
              << ansi::cyn << " \xe2\x95\x91\n" << ansi::rst;
}

static void cRow(const std::string& s, const std::string& clr = "") {
    int p = std::max(0, (W - 2 - visLen(s)) / 2);
    row(std::string(p, ' ') + s, clr);
}

static void waitEnter() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

static void attendre(int ms = 1600) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ═══════════════════════════════════════════════════════
//  Saisies
// ═══════════════════════════════════════════════════════

static std::string readHidden() {
    std::cout << std::flush;
    std::string s;
#ifdef _WIN32
    int c;
    while ((c = _getch()) != '\r' && c != '\n') {
        if (c == 8) {
            if (!s.empty()) { s.pop_back(); std::cout << "\b \b"; }
        } else if (c >= 32 && c < 127) {
            s += static_cast<char>(c); std::cout << '*';
        }
        std::cout.flush();
    }
#else
    termios old{};
    tcgetattr(STDIN_FILENO, &old);
    termios t = old;
    t.c_lflag &= ~(ECHO | ICANON);
    t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n' && c != '\r') {
        if (c == 127 || c == 8) {
            if (!s.empty()) { s.pop_back(); std::cout << "\b \b" << std::flush; }
        } else if (c >= 32 && c < 127) {
            s += c; std::cout << '*' << std::flush;
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
    std::cout << "\n";
    return s;
}

static std::string readLine(const std::string& prompt) {
    std::string s;
    std::cout << ansi::yel << "  > " << ansi::rst << prompt;
    std::getline(std::cin, s);
    return s;
}

static double readAmount(const std::string& prompt) {
    double v;
    while (true) {
        std::cout << ansi::yel << "  > " << ansi::rst << prompt;
        if (std::cin >> v && v >= 0) return v;
        std::cout << txt("  Valeur invalide.\n", "  Sanda tsy mety.\n");
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

static std::string stripSpaces(std::string s) {
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
    return s;
}

// ═══════════════════════════════════════════════════════
//  Structures de données
// ═══════════════════════════════════════════════════════

struct Transaction { std::string date, type; double montant, solde; };
struct OpResult    { bool ok; std::string msg; };
struct Status      { std::string msg; bool err = false; };

static void appendCSV(const Transaction& tx, const std::string& path) {
    std::ofstream f(path, std::ios::app);
    if (f) f << tx.date << "," << tx.type << "," << tx.montant << "," << tx.solde << "\n";
}

static std::vector<Transaction> loadCSV(const std::string& path) {
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

// ═══════════════════════════════════════════════════════
//  Classes bancaires
// ═══════════════════════════════════════════════════════

class CompteBancaire {
    std::string titulaire;
    double solde, soldeMin, limiteSession, retraitSession;
    std::string csvPath;
    std::vector<Transaction> historique;

    void log(const std::string& type, double montant) {
        Transaction tx{ now(), type, montant, solde };
        historique.push_back(tx);
        appendCSV(tx, csvPath);
    }

public:
    CompteBancaire(std::string nom, double initial,
                   double sMin = 0.0, double limSess = 0.0,
                   std::string path = "")
        : titulaire(std::move(nom)), solde(initial),
          soldeMin(sMin), limiteSession(limSess), retraitSession(0.0),
          csvPath(path.empty() ? titulaire + ".csv" : path)
    {
        if (initial < soldeMin) throw std::invalid_argument(
            "Solde initial inferieur au solde minimum.");
        if (limSess < 0.0) throw std::invalid_argument("Limite session negative.");
        historique = loadCSV(csvPath);
    }

    virtual ~CompteBancaire() = default;

    double getSolde()          const { return solde; }
    double getSoldeMin()       const { return soldeMin; }
    double getLimiteSession()  const { return limiteSession; }
    double getRetraitSession() const { return retraitSession; }
    const std::string& getTitulaire()               const { return titulaire; }
    const std::vector<Transaction>& getHistorique() const { return historique; }

    virtual std::string getType()     const { return txt("Standard", "Mahazatra"); }
    virtual std::string getTypeCode() const { return "standard"; }
    virtual double      getTaux()     const { return 0.0; }
    virtual double      getDecouvert()const { return 0.0; }
    virtual bool supporteInterets()   const { return false; }
    virtual OpResult appliquerInterets()    { return {false, txt("Non applicable.", "Tsy azo atao.")}; }

    OpResult deposer(double montant, const std::string& type = "DEPOT") {
        if (montant <= 0.0) return {false, txt("Montant invalide.", "Vola tsy mety.")};
        solde += montant;
        log(type, montant);
        return {true, "+ " + fmtNum(montant) + txt(" Ar credite.", " Ar nampidirina.")};
    }

    OpResult retirer(double montant) {
        if (montant <= 0.0)
            return {false, txt("Montant invalide.", "Vola tsy mety.")};
        if (montant > solde)
            return {false, txt("Solde insuffisant (", "Tsy ampy (") + fmtNum(solde) + " Ar)."};
        if ((solde - montant) < soldeMin)
            return {false, txt("Solde min requis : ", "Ambiny kely : ") + fmtNum(soldeMin) + " Ar."};
        if (limiteSession > 0.0 && (retraitSession + montant) > limiteSession)
            return {false, txt("Limite session atteinte : ", "Fetra fotoana : ") + fmtNum(limiteSession) + " Ar."};
        solde -= montant;
        retraitSession += montant;
        log("RETRAIT", montant);
        return {true, "- " + fmtNum(montant) + txt(" Ar debite.", " Ar nesorina.")};
    }
};

class CompteEpargne : public CompteBancaire {
    double taux;
public:
    CompteEpargne(std::string nom, double initial, double t,
                  double sMin = 0.0, double limSess = 0.0)
        : CompteBancaire(std::move(nom), initial, sMin, limSess), taux(t)
    { if (taux < 0.0) throw std::invalid_argument("Taux negatif."); }

    std::string getType() const override {
        std::ostringstream os;
        os << txt("Epargne (", "Fitahirizana (") << taux << "%)";
        return os.str();
    }
    std::string getTypeCode()  const override { return "epargne"; }
    double      getTaux()      const override { return taux; }
    bool supporteInterets()    const override { return true; }
    OpResult appliquerInterets() override {
        return deposer(getSolde() * taux / 100.0, "INTERET");
    }
};

class CompteVue : public CompteBancaire {
    double decouvertMax;
public:
    CompteVue(std::string nom, double initial, double decouvert = 0.0)
        : CompteBancaire(std::move(nom), initial, -decouvert), decouvertMax(decouvert)
    {}

    std::string getType() const override {
        std::ostringstream os;
        if (decouvertMax > 0.0)
            os << txt("Vue (dec:", "Mahazatra (") << fmtNum(decouvertMax) << " Ar)";
        else
            os << txt("Vue", "Mahazatra");
        return os.str();
    }
    std::string getTypeCode()    const override { return "vue"; }
    double      getDecouvert()   const override { return decouvertMax; }
};

// ═══════════════════════════════════════════════════════
//  Structure Utilisateur (client de la banque)
// ═══════════════════════════════════════════════════════

struct Utilisateur {
    std::string carte;        // numéro de carte
    std::string pin;          // code PIN
    std::string nom;          // nom affiché
    std::vector<std::string> comptesTitulaires; // IDs des comptes liés
};

// ═══════════════════════════════════════════════════════
//  Persistance JSON (format maison minimal)
// ═══════════════════════════════════════════════════════

static const std::string COMPTES_FILE = "comptes.json";
static const std::string USERS_FILE   = "clients.json";

static std::string jsonGetStr(const std::string& bloc, const std::string& cle) {
    std::string pat = "\"" + cle + "\"";
    auto pos = bloc.find(pat);
    if (pos == std::string::npos) return "";
    pos = bloc.find(':', pos);
    if (pos == std::string::npos) return "";
    ++pos;
    while (pos < bloc.size() && (bloc[pos] == ' ' || bloc[pos] == '\t')) ++pos;
    if (pos >= bloc.size()) return "";
    if (bloc[pos] == '"') {
        ++pos;
        std::string val;
        while (pos < bloc.size() && bloc[pos] != '"') val += bloc[pos++];
        return val;
    }
    std::string val;
    while (pos < bloc.size() && (std::isdigit(bloc[pos]) || bloc[pos] == '.' || bloc[pos] == '-'))
        val += bloc[pos++];
    return val;
}

// Extrait un tableau JSON de strings  ["a","b","c"]
static std::vector<std::string> jsonGetArray(const std::string& bloc, const std::string& cle) {
    std::vector<std::string> res;
    std::string pat = "\"" + cle + "\"";
    auto pos = bloc.find(pat);
    if (pos == std::string::npos) return res;
    pos = bloc.find('[', pos);
    if (pos == std::string::npos) return res;
    auto end = bloc.find(']', pos);
    if (end == std::string::npos) return res;
    std::string arr = bloc.substr(pos + 1, end - pos - 1);
    size_t p = 0;
    while (p < arr.size()) {
        auto q = arr.find('"', p);
        if (q == std::string::npos) break;
        auto q2 = arr.find('"', q + 1);
        if (q2 == std::string::npos) break;
        res.push_back(arr.substr(q + 1, q2 - q - 1));
        p = q2 + 1;
    }
    return res;
}

// ── Comptes ───────────────────────────────────────────

static void sauvegarderComptes(const std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    std::ofstream f(COMPTES_FILE);
    if (!f) return;
    f << "[\n";
    for (size_t i = 0; i < comptes.size(); ++i) {
        const auto& c = *comptes[i];
        f << "  {\n"
          << "    \"type\"       : \"" << c.getTypeCode()      << "\",\n"
          << "    \"titulaire\"  : \"" << c.getTitulaire()     << "\",\n"
          << "    \"solde\"      : "   << c.getSolde()         << ",\n"
          << "    \"soldeMin\"   : "   << c.getSoldeMin()      << ",\n"
          << "    \"limSession\" : "   << c.getLimiteSession() << ",\n"
          << "    \"taux\"       : "   << c.getTaux()          << ",\n"
          << "    \"decouvert\"  : "   << c.getDecouvert()     << "\n"
          << "  }" << (i + 1 < comptes.size() ? "," : "") << "\n";
    }
    f << "]\n";
}

static void chargerComptes(std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    std::ifstream f(COMPTES_FILE);
    if (!f) return;
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    size_t pos = 0;
    while (pos < content.size()) {
        auto start = content.find('{', pos);
        if (start == std::string::npos) break;
        auto end = content.find('}', start);
        if (end == std::string::npos) break;
        std::string bloc = content.substr(start, end - start + 1);
        pos = end + 1;
        std::string type      = jsonGetStr(bloc, "type");
        std::string titulaire = jsonGetStr(bloc, "titulaire");
        std::string sSolde    = jsonGetStr(bloc, "solde");
        std::string sSoldeMin = jsonGetStr(bloc, "soldeMin");
        std::string sLim      = jsonGetStr(bloc, "limSession");
        std::string sTaux     = jsonGetStr(bloc, "taux");
        std::string sDec      = jsonGetStr(bloc, "decouvert");
        if (titulaire.empty() || sSolde.empty()) continue;
        try {
            double solde   = std::stod(sSolde);
            double sMin    = sSoldeMin.empty() ? 0.0 : std::stod(sSoldeMin);
            double limSess = sLim.empty()      ? 0.0 : std::stod(sLim);
            double taux    = sTaux.empty()     ? 0.0 : std::stod(sTaux);
            double dec     = sDec.empty()      ? 0.0 : std::stod(sDec);
            if (type == "epargne")
                comptes.push_back(std::make_unique<CompteEpargne>(titulaire, solde, taux, sMin, limSess));
            else if (type == "vue")
                comptes.push_back(std::make_unique<CompteVue>(titulaire, solde, dec));
            else
                comptes.push_back(std::make_unique<CompteBancaire>(titulaire, solde, sMin, limSess));
        } catch (...) {}
    }
}

// ── Utilisateurs (clients) ────────────────────────────

static void sauvegarderUtilisateurs(const std::vector<Utilisateur>& users) {
    std::ofstream f(USERS_FILE);
    if (!f) return;
    f << "[\n";
    for (size_t i = 0; i < users.size(); ++i) {
        const auto& u = users[i];
        f << "  {\n"
          << "    \"carte\" : \"" << u.carte << "\",\n"
          << "    \"pin\"   : \"" << u.pin   << "\",\n"
          << "    \"nom\"   : \"" << u.nom   << "\",\n"
          << "    \"comptes\" : [";
        for (size_t j = 0; j < u.comptesTitulaires.size(); ++j)
            f << "\"" << u.comptesTitulaires[j] << "\""
              << (j + 1 < u.comptesTitulaires.size() ? "," : "");
        f << "]\n"
          << "  }" << (i + 1 < users.size() ? "," : "") << "\n";
    }
    f << "]\n";
}

static void chargerUtilisateurs(std::vector<Utilisateur>& users) {
    std::ifstream f(USERS_FILE);
    if (!f) return;
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    size_t pos = 0;
    while (pos < content.size()) {
        auto start = content.find('{', pos);
        if (start == std::string::npos) break;
        // trouver la fermeture de cet objet (peut contenir [])
        int depth = 0; size_t end = start;
        for (; end < content.size(); ++end) {
            if (content[end] == '{') ++depth;
            else if (content[end] == '}') { --depth; if (depth == 0) break; }
        }
        std::string bloc = content.substr(start, end - start + 1);
        pos = end + 1;
        Utilisateur u;
        u.carte = jsonGetStr(bloc, "carte");
        u.pin   = jsonGetStr(bloc, "pin");
        u.nom   = jsonGetStr(bloc, "nom");
        u.comptesTitulaires = jsonGetArray(bloc, "comptes");
        if (!u.carte.empty() && !u.pin.empty()) users.push_back(u);
    }
}

// ═══════════════════════════════════════════════════════
//  Authentification
// ═══════════════════════════════════════════════════════

static const int MAX_ESSAIS = 3;

// Retourne l'index de l'utilisateur authentifié, -1 si échec
static int authentifierClient(const std::vector<Utilisateur>& users) {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string err;

    for (int essai = 1; essai <= MAX_ESSAIS; ++essai) {
        cls(); top();
        cRow(txt("ACCES CLIENT", "FIDIRANA MPANJIFA"), ansi::bold + ansi::cyn);
        sep();
        row("  " + txt("Carte bancaire + code PIN", "Karatra + kaody PIN"));
        if (!err.empty()) { sep(); row("  " + err, ansi::red); }
        bot();

        std::cout << "\n" << ansi::yel << "  > " << ansi::rst
                  << txt("Numero de carte : ", "Laharan-karatra : ");
        std::string carte;
        std::getline(std::cin, carte);
        carte = stripSpaces(carte);

        std::cout << ansi::yel << "  > " << ansi::rst
                  << txt("Code PIN : ", "Kaody PIN : ");
        std::string pin = readHidden();

        for (int i = 0; i < (int)users.size(); ++i) {
            if (users[i].carte == carte && users[i].pin == pin)
                return i;
        }

        err = txt("Carte ou PIN incorrect (", "Diso ny karatra na PIN (")
            + std::to_string(MAX_ESSAIS - essai)
            + txt(" essai(s) restant(s)).", " ezaka sisa).");
    }
    return -1;
}

static const std::string ADMIN_CODE = "GAB-ADMIN";
static const std::string ADMIN_PIN  = "0000";

static bool authentifierAdmin() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string err;

    for (int essai = 1; essai <= MAX_ESSAIS; ++essai) {
        cls(); top();
        cRow(txt("ACCES ADMINISTRATEUR", "FIDIRANA MPITANTANA"), ansi::bold + ansi::mag);
        sep();
        row("  " + txt("Reservé au personnel de la banque.",
                       "Voatokana ho an'ny mpiasan'ny banky."));
        if (!err.empty()) { sep(); row("  " + err, ansi::red); }
        bot();

        std::cout << "\n" << ansi::yel << "  > " << ansi::rst
                  << txt("Code agent : ", "Kaody mpiasa : ");
        std::string code;
        std::getline(std::cin, code);

        std::cout << ansi::yel << "  > " << ansi::rst
                  << txt("Code PIN admin : ", "Kaody PIN admin : ");
        std::string pin = readHidden();

        if (stripSpaces(code) == ADMIN_CODE && pin == ADMIN_PIN) return true;

        err = txt("Identifiants incorrects (", "Tsy mety (")
            + std::to_string(MAX_ESSAIS - essai)
            + txt(" essai(s) restant(s)).", " ezaka sisa).");
    }
    return false;
}

//  Écrans client

static int selectCompte(const std::vector<CompteBancaire*>& comptes) {
    cls(); top();
    cRow(txt("SELECTION DU COMPTE", "FIFIDIANANA KAONTY"), ansi::bold + ansi::cyn);
    sep();
    for (int i = 0; i < (int)comptes.size(); ++i) {
        const auto& c = *comptes[i];
        std::string line = "  [" + std::to_string(i + 1) + "] "
            + fit(c.getTitulaire(), 20)
            + fit(c.getType(), 18)
            + ansi::grn + fmtNum(c.getSolde()) + " Ar" + ansi::rst;
        row(line);
    }
    sep(); bot();

    int choix;
    while (true) {
        std::cout << "\n" << ansi::yel << "  > "
                  << txt("Votre choix (1-", "Safidio (1-")
                  << comptes.size() << ") : " << ansi::rst;
        if (std::cin >> choix && choix >= 1 && choix <= (int)comptes.size())
            return choix - 1;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << txt("  Choix invalide.\n", "  Safidy tsy mety.\n");
    }
}

static void showMenuClient(const CompteBancaire& c, const Status& st = {}) {
    cls(); top();
    cRow(txt("GUICHET AUTOMATIQUE  v2.0", "RAFI-BOLA ATM  v2.0"), ansi::bold + ansi::cyn);
    sep();
    row("  " + txt("Titulaire : ", "Tompon-kaonty : ") + c.getTitulaire());
    row("  " + txt("Type      : ", "Kaonty        : ") + c.getType());
    row("  " + txt("Solde     : ", "Ambiny        : ")
              + ansi::grn + ansi::bold + fmtNum(c.getSolde()) + " Ariary" + ansi::rst);
    if (c.getLimiteSession() > 0.0)
        row("  " + txt("Session   : ", "Fotoana       : ")
                  + fmtNum(c.getRetraitSession()) + " / "
                  + fmtNum(c.getLimiteSession()) + " Ar");
    sep();
    row("  [1] " + txt("Infos compte",    "Fampahalalana"));
    row("  [2] " + txt("Depot",           "Ampiditra vola"));
    row("  [3] " + txt("Retrait",         "Misintona vola"));
    if (c.supporteInterets())
        row("  [4] " + txt("Appliquer interets", "Hanampy zanabola"));
    row("  [5] " + txt("Historique",      "Tantara"));
    row("  [6] " + txt("Changer compte",  "Hanova kaonty"));
    row("  [0] " + txt("Quitter",         "Hiala"), ansi::yel);
    sep();
    if (!st.msg.empty())
        row("  " + st.msg, st.err ? ansi::red : ansi::grn);
    bot();
    std::cout << "\n" << ansi::yel << "  > " << txt("Votre choix : ", "Safidio : ") << ansi::rst;
}

static void showInfo(const CompteBancaire& c) {
    cls(); top();
    cRow(txt("DETAILS DU COMPTE", "ANTSIPIRIHANA KAONTY"), ansi::bold + ansi::cyn);
    sep();
    row("  " + txt("Titulaire  : ", "Tompon-kaonty : ") + c.getTitulaire());
    row("  " + txt("Type       : ", "Kaonty        : ") + c.getType());
    row("  " + txt("Solde      : ", "Ambiny        : ")
               + ansi::grn + ansi::bold + fmtNum(c.getSolde()) + " Ariary" + ansi::rst);
    if (c.getSoldeMin() != 0.0)
        row("  " + txt("Solde min  : ", "Ambiny kely   : ") + fmtNum(c.getSoldeMin()) + " Ar");
    if (c.getLimiteSession() > 0.0)
        row("  " + txt("Lim.session: ", "Fetra fotoana : ")
                   + fmtNum(c.getRetraitSession()) + " / "
                   + fmtNum(c.getLimiteSession()) + " Ar");
    sep();
    row("  " + txt("[ Entree pour revenir ]", "[ Entree hanohy ]"), ansi::yel);
    bot();
    waitEnter();
}

static void showHistorique(const CompteBancaire& c) {
    cls(); top();
    cRow(txt("HISTORIQUE DES TRANSACTIONS", "TANTARA NY ARA-BOLA"), ansi::bold + ansi::cyn);
    sep();
    const auto& hist = c.getHistorique();
    if (hist.empty()) {
        row("  " + txt("Aucune transaction.", "Tsy misy tantara."), ansi::yel);
    } else {
        row("  " + fit("DATE", 21) + fit("TYPE", 9) + fit("MONTANT", 13) + "SOLDE", ansi::bold);
        sep();
        int from = std::max(0, (int)hist.size() - 10);
        for (int i = from; i < (int)hist.size(); ++i) {
            const auto& tx = hist[i];
            const std::string& clr = tx.type == "RETRAIT" ? ansi::red : ansi::grn;
            row("  " + fit(tx.date, 21) + fit(tx.type, 9) +
                       fit(fmtNum(tx.montant) + " Ar", 13) +
                       fmtNum(tx.solde) + " Ar", clr);
        }
    }
    sep();
    row("  " + txt("[ Entree pour revenir ]", "[ Entree hanohy ]"), ansi::yel);
    bot();
    waitEnter();
}

//  Menu Administrateur

static void showMenuAdmin(const Status& st = {}) {
    cls(); top();
    cRow(txt("ADMINISTRATION BANCAIRE", "FITANTANANA NY BANKY"), ansi::bold + ansi::mag);
    sep();
    row("  [1] " + txt("Creer un compte epargne",  "Hamorona kaonty fitahirizana"));
    row("  [2] " + txt("Creer un compte vue",       "Hamorona kaonty mahazatra"));
    row("  [3] " + txt("Supprimer un compte",       "Hamafa kaonty"));
    row("  [4] " + txt("Lister tous les comptes",   "Hijery kaonty rehetra"));
    sep();
    row("  [5] " + txt("Creer un client",           "Hamorona mpanjifa"));
    row("  [6] " + txt("Supprimer un client",       "Hamafa mpanjifa"));
    row("  [7] " + txt("Lister les clients",        "Hijery mpanjifa rehetra"));
    row("  [8] " + txt("Associer compte a client",  "Hampifandray kaonty sy mpanjifa"));
    sep();
    row("  [0] " + txt("Quitter administration",    "Hiala fitantanana"), ansi::yel);
    sep();
    if (!st.msg.empty())
        row("  " + st.msg, st.err ? ansi::red : ansi::grn);
    bot();
    std::cout << "\n" << ansi::yel << "  > " << txt("Choix : ", "Safidio : ") << ansi::rst;
}

static void adminCreerCompteEpargne(std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    cls(); top();
    cRow(txt("NOUVEAU COMPTE EPARGNE", "KAONTY FITAHIRIZANA VAOVAO"), ansi::bold + ansi::cyn);
    sep(); bot();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string nom = readLine(txt("Nom du titulaire     : ", "Anaran'ny tompo-kaonty : "));
    double solde    = readAmount(txt("Solde initial (Ar)   : ", "Vola voalohany (Ar)    : "));
    double taux     = readAmount(txt("Taux d'interet (%)   : ", "Taham-pitomboana (%)   : "));
    double soldeMin = readAmount(txt("Solde minimum (Ar)   : ", "Ambiny kely (Ar)       : "));
    double limSess  = readAmount(txt("Limite session (0=∞) : ", "Fetra fotoana (0=∞)   : "));
    try {
        comptes.push_back(std::make_unique<CompteEpargne>(nom, solde, taux, soldeMin, limSess));
        sauvegarderComptes(comptes);
        cls(); top();
        cRow(txt("Compte epargne cree !", "Kaonty fitahirizana voaforona !"), ansi::grn + ansi::bold);
        bot(); attendre();
    } catch (const std::exception& e) {
        cls(); top(); row("  Erreur : " + std::string(e.what()), ansi::red); bot(); attendre();
    }
}

static void adminCreerCompteVue(std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    cls(); top();
    cRow(txt("NOUVEAU COMPTE VUE", "KAONTY MAHAZATRA VAOVAO"), ansi::bold + ansi::cyn);
    sep(); bot();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string nom = readLine(txt("Nom du titulaire          : ", "Anaran'ny tompo-kaonty    : "));
    double solde    = readAmount(txt("Solde initial (Ar)        : ", "Vola voalohany (Ar)       : "));
    double dec      = readAmount(txt("Decouvert autorise (0=non): ", "Trosa avela (0=tsy misy)  : "));
    try {
        comptes.push_back(std::make_unique<CompteVue>(nom, solde, dec));
        sauvegarderComptes(comptes);
        cls(); top();
        cRow(txt("Compte vue cree !", "Kaonty mahazatra voaforona !"), ansi::grn + ansi::bold);
        bot(); attendre();
    } catch (const std::exception& e) {
        cls(); top(); row("  Erreur : " + std::string(e.what()), ansi::red); bot(); attendre();
    }
}

static void adminSupprimerCompte(std::vector<std::unique_ptr<CompteBancaire>>& comptes,
                                  std::vector<Utilisateur>& users) {
    if (comptes.empty()) {
        cls(); top();
        row("  " + txt("Aucun compte existant.", "Tsy misy kaonty."), ansi::yel);
        bot(); attendre(); return;
    }
    cls(); top();
    cRow(txt("SUPPRIMER UN COMPTE", "HAMAFA KAONTY"), ansi::bold + ansi::red);
    sep();
    for (int i = 0; i < (int)comptes.size(); ++i) {
        row("  [" + std::to_string(i + 1) + "] "
            + fit(comptes[i]->getTitulaire(), 20)
            + fit(comptes[i]->getType(), 18)
            + ansi::grn + fmtNum(comptes[i]->getSolde()) + " Ar" + ansi::rst);
    }
    sep(); bot();

    int idx;
    std::cout << "\n" << ansi::yel << "  > "
              << txt("Numero (0=annuler) : ", "Laharana (0=hiala) : ") << ansi::rst;
    if (!(std::cin >> idx) || idx < 1 || idx > (int)comptes.size()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    int ri = idx - 1;
    std::string nom = comptes[ri]->getTitulaire();

    // Confirmation
    cls(); top();
    row("  " + txt("Supprimer : ", "Hofafana : ") + nom, ansi::yel);
    row("  [1] " + txt("Confirmer", "Manamarina") + "   [0] " + txt("Annuler", "Hiala"));
    bot();
    int conf;
    std::cout << "\n" << ansi::yel << "  > " << ansi::rst;
    if (!(std::cin >> conf) || conf != 1) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    // Retirer ce compte des listes clients aussi
    for (auto& u : users) {
        auto& ct = u.comptesTitulaires;
        ct.erase(std::remove(ct.begin(), ct.end(), nom), ct.end());
    }
    sauvegarderUtilisateurs(users);
    comptes.erase(comptes.begin() + ri);
    sauvegarderComptes(comptes);
    cls(); top();
    cRow(txt("Compte supprime.", "Voafafa ny kaonty."), ansi::grn + ansi::bold);
    bot(); attendre();
}

static void adminListerComptes(const std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    cls(); top();
    cRow(txt("LISTE DE TOUS LES COMPTES", "LISITRY NY KAONTY REHETRA"), ansi::bold + ansi::cyn);
    sep();
    if (comptes.empty()) {
        row("  " + txt("Aucun compte.", "Tsy misy kaonty."), ansi::yel);
    } else {
        row("  " + fit(txt("TITULAIRE","TOMPON-KAONTY"), 20)
                 + fit(txt("TYPE","KAONTY"), 20)
                 + txt("SOLDE","AMBINY"), ansi::bold);
        sep();
        for (const auto& c : comptes) {
            row("  " + fit(c->getTitulaire(), 20)
                     + fit(c->getType(), 20)
                     + ansi::grn + fmtNum(c->getSolde()) + " Ar" + ansi::rst);
        }
    }
    sep();
    row("  " + txt("[ Entree pour revenir ]", "[ Entree hanohy ]"), ansi::yel);
    bot();
    waitEnter();
}

static void adminCreerClient(std::vector<Utilisateur>& users,
                              const std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    cls(); top();
    cRow(txt("NOUVEAU CLIENT", "MPANJIFA VAOVAO"), ansi::bold + ansi::cyn);
    sep(); bot();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string nom   = readLine(txt("Nom du client           : ", "Anaran'ny mpanjifa      : "));
    std::string carte = readLine(txt("Numero de carte (16 ch.): ", "Laharan-karatra (16 sah): "));
    carte = stripSpaces(carte);

    // Vérifier unicité de la carte
    for (const auto& u : users) {
        if (u.carte == carte) {
            cls(); top();
            row("  " + txt("Ce numero de carte existe deja.", "Efa misy io laharan-karatra io."), ansi::red);
            bot(); attendre(); return;
        }
    }

    std::cout << ansi::yel << "  > " << ansi::rst
              << txt("Code PIN (4 chiffres) : ", "Kaody PIN (isa 4) : ");
    std::string pin = readHidden();

    // Associer des comptes dès la création
    std::vector<std::string> comptesChoisis;
    if (!comptes.empty()) {
        cls(); top();
        cRow(txt("ASSOCIER DES COMPTES (optionnel)", "HAMPIFANDRAY KAONTY (safidy)"), ansi::bold + ansi::cyn);
        sep();
        for (int i = 0; i < (int)comptes.size(); ++i)
            row("  [" + std::to_string(i + 1) + "] " + comptes[i]->getTitulaire()
                + "  (" + comptes[i]->getType() + ")");
        row("  [0] " + txt("Terminer la selection", "Vita ny fisafidianana"), ansi::yel);
        sep(); bot();

        while (true) {
            int idx;
            std::cout << "\n" << ansi::yel << "  > "
                      << txt("Ajouter (0=fin) : ", "Hanampy (0=vita) : ") << ansi::rst;
            if (!(std::cin >> idx)) { std::cin.clear(); std::cin.ignore(1000, '\n'); continue; }
            if (idx == 0) break;
            if (idx >= 1 && idx <= (int)comptes.size()) {
                std::string t = comptes[idx - 1]->getTitulaire();
                if (std::find(comptesChoisis.begin(), comptesChoisis.end(), t) == comptesChoisis.end())
                    comptesChoisis.push_back(t);
            }
        }
    }

    users.push_back({carte, pin, nom, comptesChoisis});
    sauvegarderUtilisateurs(users);
    cls(); top();
    cRow(txt("Client cree avec succes !", "Nahomby ny famoronana !"), ansi::grn + ansi::bold);
    bot(); attendre();
}

static void adminSupprimerClient(std::vector<Utilisateur>& users,
                                  std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    if (users.empty()) {
        cls(); top();
        row("  " + txt("Aucun client.", "Tsy misy mpanjifa."), ansi::yel);
        bot(); attendre(); return;
    }
    cls(); top();
    cRow(txt("SUPPRIMER UN CLIENT", "HAMAFA MPANJIFA"), ansi::bold + ansi::red);
    sep();
    for (int i = 0; i < (int)users.size(); ++i) {
        row("  [" + std::to_string(i + 1) + "] " + fit(users[i].nom, 20) + " Carte: " + users[i].carte);
        for (const auto& ct : users[i].comptesTitulaires)
            row("      " + txt("- Compte: ", "- Kaonty: ") + ct, ansi::yel);
    }
    sep(); bot();

    int idx;
    std::cout << "\n" << ansi::yel << "  > "
              << txt("Numero (0=annuler) : ", "Laharana (0=hiala) : ") << ansi::rst;
    if (!(std::cin >> idx) || idx < 1 || idx > (int)users.size()) {
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return;
    }
    int ri = idx - 1;
    const Utilisateur& u = users[ri];

    // Ecran de confirmation avec liste des comptes qui seront supprimes
    cls(); top();
    cRow(txt("CONFIRMATION DE SUPPRESSION", "FANAMAFISANA NY FAFANA"), ansi::bold + ansi::red);
    sep();
    row("  " + txt("Client : ", "Mpanjifa : ") + u.nom, ansi::yel);
    if (u.comptesTitulaires.empty()) {
        row("  " + txt("(aucun compte associe)", "(tsy misy kaonty)"), ansi::yel);
    } else {
        row("  " + txt("Comptes qui seront SUPPRIMES :",
                       "Kaonty ho FAFANA :"), ansi::red + ansi::bold);
        for (const auto& ct : u.comptesTitulaires)
            row("    - " + ct, ansi::red);
    }
    sep();
    row("  [1] " + txt("Confirmer la suppression", "Manamarina ny fafana") +
        "   [0] " + txt("Annuler", "Hiala"), ansi::bold);
    bot();
    int conf;
    std::cout << "\n" << ansi::yel << "  > " << ansi::rst;
    if (!(std::cin >> conf) || conf != 1) {
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return;
    }

    // Supprimer tous les comptes associes a ce client
    for (const auto& titulaire : u.comptesTitulaires) {
        comptes.erase(
            std::remove_if(comptes.begin(), comptes.end(),
                [&titulaire](const std::unique_ptr<CompteBancaire>& c) {
                    return c->getTitulaire() == titulaire;
                }),
            comptes.end()
        );
    }
    sauvegarderComptes(comptes);

    // Supprimer le client
    users.erase(users.begin() + ri);
    sauvegarderUtilisateurs(users);

    cls(); top();
    cRow(txt("Client et comptes supprimes.", "Voafafa ny mpanjifa sy ny kaonty."),
         ansi::grn + ansi::bold);
    bot(); attendre();
}

static void adminListerClients(const std::vector<Utilisateur>& users) {
    cls(); top();
    cRow(txt("LISTE DES CLIENTS", "LISITRY NY MPANJIFA"), ansi::bold + ansi::cyn);
    sep();
    if (users.empty()) {
        row("  " + txt("Aucun client.", "Tsy misy mpanjifa."), ansi::yel);
    } else {
        for (const auto& u : users) {
            row("  " + ansi::bold + u.nom + ansi::rst
                + "  " + txt("Carte: ","Karatra: ") + u.carte);
            if (u.comptesTitulaires.empty()) {
                row("    " + txt("(aucun compte associe)", "(tsy misy kaonty)"), ansi::yel);
            } else {
                for (const auto& ct : u.comptesTitulaires)
                    row("    " + txt("Compte: ","Kaonty: ") + ct, ansi::grn);
            }
            sep();
        }
    }
    row("  " + txt("[ Entree pour revenir ]", "[ Entree hanohy ]"), ansi::yel);
    bot();
    waitEnter();
}

static void adminAssocierCompte(std::vector<Utilisateur>& users,
                                 const std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    if (users.empty() || comptes.empty()) {
        cls(); top();
        row("  " + txt("Clients ou comptes manquants.", "Tsy misy mpanjifa na kaonty."), ansi::yel);
        bot(); attendre(); return;
    }
    // Choisir client
    cls(); top();
    cRow(txt("ASSOCIER COMPTE A CLIENT", "HAMPIFANDRAY KAONTY SY MPANJIFA"), ansi::bold + ansi::cyn);
    sep();
    for (int i = 0; i < (int)users.size(); ++i)
        row("  [" + std::to_string(i + 1) + "] " + users[i].nom);
    sep(); bot();

    int ui;
    std::cout << "\n" << ansi::yel << "  > "
              << txt("Choisir client (0=annuler) : ", "Safidio mpanjifa (0=hiala) : ") << ansi::rst;
    if (!(std::cin >> ui) || ui < 1 || ui > (int)users.size()) {
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return;
    }
    Utilisateur& u = users[ui - 1];

    // Choisir compte
    cls(); top();
    cRow(txt("CHOISIR LE COMPTE", "SAFIDIO NY KAONTY"), ansi::bold + ansi::cyn);
    sep();
    for (int i = 0; i < (int)comptes.size(); ++i) {
        bool deja = std::find(u.comptesTitulaires.begin(), u.comptesTitulaires.end(),
                              comptes[i]->getTitulaire()) != u.comptesTitulaires.end();
        row("  [" + std::to_string(i + 1) + "] "
            + fit(comptes[i]->getTitulaire(), 20)
            + (deja ? ansi::yel + txt(" (deja associe)", " (efa)") + ansi::rst : ""));
    }
    sep(); bot();

    int ci;
    std::cout << "\n" << ansi::yel << "  > "
              << txt("Choisir compte (0=annuler) : ", "Safidio kaonty (0=hiala) : ") << ansi::rst;
    if (!(std::cin >> ci) || ci < 1 || ci > (int)comptes.size()) {
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return;
    }
    std::string nomCompte = comptes[ci - 1]->getTitulaire();
    if (std::find(u.comptesTitulaires.begin(), u.comptesTitulaires.end(), nomCompte)
            == u.comptesTitulaires.end()) {
        u.comptesTitulaires.push_back(nomCompte);
        sauvegarderUtilisateurs(users);
        cls(); top();
        cRow(txt("Association effectuee !", "Vita ny fampifandraisana !"), ansi::grn + ansi::bold);
        bot(); attendre();
    } else {
        cls(); top();
        row("  " + txt("Ce compte est deja associe.", "Efa voampifandray io kaonty io."), ansi::yel);
        bot(); attendre();
    }
}

// ═══════════════════════════════════════════════════════
//  main
// ═══════════════════════════════════════════════════════

int main() {
    enableAnsi();

    // Choix de langue
    std::cout << "Choisir la langue / Safidy fiteny\n1. Francais\n2. Malagasy\n> ";
    int choixLangue;
    if (!(std::cin >> choixLangue)) { std::cerr << "Erreur.\n"; return 1; }
    langueActuelle = (choixLangue == 2) ? 2 : 1;

    try {
        // Chargement des données persistées
        std::vector<std::unique_ptr<CompteBancaire>> comptes;
        std::vector<Utilisateur> users;
        chargerComptes(comptes);
        chargerUtilisateurs(users);

        // ── Choix du mode ──────────────────────────────
        while (true) {
            cls(); top();
            cRow(txt("BIENVENUE AU GAB", "TONGA SOA AU GAB"), ansi::bold + ansi::cyn);
            sep();
            row("  [1] " + txt("Acces Client",          "Fidirana Mpanjifa"),      ansi::grn);
            row("  [2] " + txt("Acces Administrateur",  "Fidirana Mpitantana"),    ansi::mag);
            row("  [0] " + txt("Quitter",                "Hiala"),                  ansi::yel);
            sep(); bot();

            int mode;
            std::cout << "\n" << ansi::yel << "  > " << txt("Mode : ","Safidio : ") << ansi::rst;
            if (!(std::cin >> mode)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            if (mode == 0) break;

            // ════════════════════════════════════════════
            //  MODE CLIENT
            // ════════════════════════════════════════════
            if (mode == 1) {
                int userIdx = authentifierClient(users);
                if (userIdx < 0) {
                    cls(); top();
                    cRow(txt("Carte bloquee : trop d'essais.",
                             "Voasakana : be loatra ny ezaka."), ansi::red + ansi::bold);
                    bot(); attendre(2000); continue;
                }

                const Utilisateur& user = users[userIdx];

                // Récupérer les comptes liés à ce client
                std::vector<CompteBancaire*> mesComptes;
                for (const auto& titulaire : user.comptesTitulaires) {
                    for (const auto& c : comptes) {
                        if (c->getTitulaire() == titulaire)
                            mesComptes.push_back(c.get());
                    }
                }

                if (mesComptes.empty()) {
                    cls(); top();
                    cRow(txt("Bienvenue " + user.nom + " !",
                             "Tonga soa " + user.nom + " !"), ansi::grn + ansi::bold);
                    sep();
                    row("  " + txt("Aucun compte associe a votre carte.",
                                   "Tsy misy kaonty voampifandray."), ansi::yel);
                    row("  " + txt("Contactez votre agence bancaire.",
                                   "Mifandraisa amin'ny banky."));
                    sep(); bot(); attendre(2500); continue;
                }

                // Accueil
                cls(); top();
                cRow(txt("Bienvenue ", "Tonga soa ") + user.nom + " !",
                     ansi::grn + ansi::bold);
                bot(); attendre();

                int idx = (mesComptes.size() == 1) ? 0 : selectCompte(mesComptes);
                CompteBancaire* c = mesComptes[idx];

                Status st;
                bool actif = true;
                while (actif) {
                    showMenuClient(*c, st);
                    st = {};
                    int choix;
                    if (!(std::cin >> choix)) {
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        st = {txt("Entree invalide.", "Tsy mety."), true};
                        continue;
                    }
                    switch (choix) {
                        case 1: showInfo(*c); break;
                        case 2: {
                            double m = readAmount(txt("Montant a deposer (Ar) : ", "Vola ampidirina (Ar) : "));
                            auto res = c->deposer(m);
                            sauvegarderComptes(comptes);
                            st = {res.msg, !res.ok};
                            break;
                        }
                        case 3: {
                            double m = readAmount(txt("Montant a retirer (Ar) : ", "Vola sintonina (Ar) : "));
                            auto res = c->retirer(m);
                            sauvegarderComptes(comptes);
                            st = {res.msg, !res.ok};
                            break;
                        }
                        case 4: {
                            auto res = c->appliquerInterets();
                            sauvegarderComptes(comptes);
                            st = {res.msg, !res.ok};
                            break;
                        }
                        case 5: showHistorique(*c); break;
                        case 6:
                            if (mesComptes.size() > 1) {
                                idx = selectCompte(mesComptes);
                                c = mesComptes[idx];
                            } else {
                                st = {txt("Vous n'avez qu'un seul compte.",
                                          "Kaonty iray ihany anao."), false};
                            }
                            break;
                        case 0: actif = false; break;
                        default:
                            st = {txt("Choix invalide.", "Safidy tsy mety."), true};
                    }
                }

                cls(); top();
                cRow(txt("Merci et au revoir, ", "Misaotra, ") + user.nom + " !",
                     ansi::grn + ansi::bold);
                bot(); attendre();
            }
            //  MODE ADMINISTRATEUR

            else if (mode == 2) {
                if (!authentifierAdmin()) {
                    cls(); top();
                    cRow(txt("Acces refuse : trop d'essais.",
                             "Tsy avela : be loatra ny ezaka."), ansi::red + ansi::bold);
                    bot(); attendre(2000); continue;
                }

                cls(); top();
                cRow(txt("Bienvenue Administrateur", "Tonga soa Mpitantana"), ansi::mag + ansi::bold);
                bot(); attendre(1200);

                Status st;
                bool actif = true;
                while (actif) {
                    showMenuAdmin(st);
                    st = {};
                    int choix;
                    if (!(std::cin >> choix)) {
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        st = {txt("Entree invalide.", "Tsy mety."), true};
                        continue;
                    }
                    switch (choix) {
                        case 1: adminCreerCompteEpargne(comptes); break;
                        case 2: adminCreerCompteVue(comptes); break;
                        case 3: adminSupprimerCompte(comptes, users); break;
                        case 4: adminListerComptes(comptes); break;
                        case 5: adminCreerClient(users, comptes); break;
                        case 6: adminSupprimerClient(users, comptes); break;
                        case 7: adminListerClients(users); break;
                        case 8: adminAssocierCompte(users, comptes); break;
                        case 0: actif = false; break;
                        default:
                            st = {txt("Choix invalide.", "Safidy tsy mety."), true};
                    }
                }
            }
        }

        cls(); top();
        cRow(txt("Au revoir !", "Veloma !"), ansi::grn + ansi::bold);
        bot();
        std::cout << "\n";

    } catch (const std::exception& e) {
        std::cerr << ansi::red << "Erreur : " << e.what() << ansi::rst << "\n";
        return 1;
    }
    return 0;
}