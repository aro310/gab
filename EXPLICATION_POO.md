# 🏦 Système ATM — Explication de la Programmation Orientée Objet (POO)

> Ce document explique **tous les concepts POO** utilisés dans le projet `DEMO_ATM_SYSTEM.CPP`,  
> rédigé pour un **débutant en C++**.

---

## 📋 Table des matières

1. [C'est quoi la POO ?](#cest-quoi-la-poo-)
2. [Concept 1 — La Classe et l'Objet](#concept-1--la-classe-et-lobjet)
3. [Concept 2 — L'Encapsulation](#concept-2--lencapsulation)
4. [Concept 3 — Le Constructeur et le Destructeur](#concept-3--le-constructeur-et-le-destructeur)
5. [Concept 4 — L'Héritage](#concept-4--lhéritage)
6. [Concept 5 — Le Polymorphisme](#concept-5--le-polymorphisme)
7. [Architecture globale du projet](#architecture-globale-du-projet)
8. [Tableau récapitulatif](#tableau-récapitulatif)

---

## C'est quoi la POO ? 🤔

La **Programmation Orientée Objet** est une façon d'organiser le code en utilisant des **objets**.

Imagine le monde réel :
- Une **voiture** a des caractéristiques (couleur, vitesse) et des actions (démarrer, freiner).
- Un **compte bancaire** a des caractéristiques (solde, titulaire) et des actions (déposer, retirer).

En POO, on modélise ces éléments avec des **classes** et des **objets**.

```
Monde réel        →     POO C++
─────────────────────────────────
Catégorie          →    Classe
Un exemplaire      →    Objet
Caractéristiques   →    Attributs
Actions            →    Méthodes
```

---

## Concept 1 — La Classe et l'Objet

### 📘 Définition

| Terme | Définition simple |
|-------|------------------|
| **Classe** | C'est le **plan / moule** pour créer des objets. |
| **Objet** | C'est **une instance** (un exemplaire) créé à partir d'une classe. |

### 🔍 Dans notre projet

```cpp
// DECLARATION de la classe (le moule)
class CompteBancaire {
    string titulaire;
    double solde;
    // ...
};

// CREATION d'un objet (l'exemplaire)
CompteEpargne monCompte("Aro", 50000.0, 3.5);
//             ↑
//         C'est l'objet !
```

> **Analogie** : La classe `CompteBancaire` est comme un **formulaire vierge de banque**.  
> `monCompte` est un **formulaire rempli** pour Ahmed Benali.

---

## Concept 2 — L'Encapsulation

### 📘 Définition

L'encapsulation signifie **cacher les données sensibles** à l'intérieur de la classe  
et n'autoriser l'accès qu'à travers des **méthodes contrôlées**.

On utilise les mots-clés :
- `private` → accessible **uniquement à l'intérieur** de la classe
- `public`  → accessible **de partout**

### 🔍 Dans notre projet

```cpp
class CompteBancaire {

private:                      // 🔒 PROTÉGÉ — accès interdit de l'extérieur
    string titulaire;
    double solde;

public:                       // 🔓 ACCESSIBLE — méthodes contrôlées
    double getSolde() {       // Getter : lire le solde (lecture seule)
        return solde;
    }
    void deposer(double montant) {   // Méthode qui modifie le solde
        solde += montant;            // de façon sécurisée
    }
};
```

**Sans encapsulation (❌ mauvaise pratique) :**
```cpp
monCompte.solde = -99999; // N'importe qui peut mettre n'importe quoi !
```

**Avec encapsulation (✅ bonne pratique) :**
```cpp
monCompte.deposer(-99999); // La méthode vérifie si le montant est valide !
```

> **Pourquoi ?** Pour **protéger les données** contre les erreurs ou la triche.

---

## Concept 3 — Le Constructeur et le Destructeur

### 📘 Définition

| Terme | Rôle | Quand est-il appelé ? |
|-------|------|----------------------|
| **Constructeur** | Initialise l'objet à sa création | Automatiquement à la création |
| **Destructeur** | Libère les ressources | Automatiquement à la fin de vie |

### 🔍 Dans notre projet

```cpp
class CompteBancaire {
public:

    // ─── CONSTRUCTEUR ───────────────────────────────────────────
    // Même nom que la classe, pas de type de retour.
    CompteBancaire(string nom, double soldeInitial) {
        titulaire = nom;           // Initialisation
        solde     = soldeInitial;  // Initialisation
        cout << "Compte créé pour : " << titulaire << endl;
    }

    // ─── DESTRUCTEUR ────────────────────────────────────────────
    // Même nom que la classe, précédé du symbole ~
    virtual ~CompteBancaire() {
        cout << "Compte de " << titulaire << " fermé." << endl;
    }
};
```

**Utilisation :**
```cpp
// Le constructeur est appelé ICI automatiquement
CompteEpargne monCompte("Ahmed Benali", 50000.0, 3.5);

// ... code ...

// Le destructeur est appelé ICI automatiquement (fin du main)
return 0;
```

> **Analogie** : Le constructeur = **ouvrir un compte à la banque**.  
>               Le destructeur = **fermer le compte et récupérer ses affaires**.

---

## Concept 4 — L'Héritage

### 📘 Définition

L'héritage permet à une **classe enfant** de **réutiliser** tout le code d'une **classe parent**,  
et d'y ajouter de nouvelles fonctionnalités.

```
CompteBancaire  (classe parent / base)
      │
      └──► CompteEpargne  (classe enfant / dérivée)
              + tauxInteret
              + appliquerInterets()
```

### 🔍 Dans notre projet

```cpp
// PARENT : classe de base
class CompteBancaire {
    // titulaire, solde, deposer(), retirer() ...
};

// ENFANT : hérite de CompteBancaire avec ": public CompteBancaire"
class CompteEpargne : public CompteBancaire {

    double tauxInteret; // Attribut SUPPLÉMENTAIRE

    // Constructeur de l'enfant — appelle d'abord le parent !
    CompteEpargne(string nom, double solde, double taux)
        : CompteBancaire(nom, solde) {  // ← appel du constructeur parent
        tauxInteret = taux;
    }

    void appliquerInterets() {          // Méthode SUPPLÉMENTAIRE
        double gain = getSolde() * (tauxInteret / 100.0);
        deposer(gain); // méthode héritée du parent !
    }
};
```

**Ce que CompteEpargne hérite automatiquement :**
- ✅ `titulaire`, `solde` (attributs du parent)
- ✅ `deposer()`, `retirer()`, `afficherInfo()` (méthodes du parent)

**Ce que CompteEpargne ajoute en plus :**
- ➕ `tauxInteret` (nouvel attribut)
- ➕ `appliquerInterets()` (nouvelle méthode)

> **Analogie** : Un `CompteEpargne` **est un** `CompteBancaire` (mais avec des bonus).  
>               Comme une voiture de sport **est une** voiture (mais plus rapide).

---

## Concept 5 — Le Polymorphisme

### 📘 Définition

Le polymorphisme signifie qu'**une même méthode peut se comporter différemment**  
selon le type d'objet qui l'appelle.

On utilise le mot-clé `virtual` dans le parent et `override` dans l'enfant.

### 🔍 Dans notre projet

```cpp
// Dans le PARENT
class CompteBancaire {
public:
    virtual void afficherType() {                    // "virtual" = redéfinissable
        cout << "Type : Compte Standard" << endl;
    }
};

// Dans l'ENFANT
class CompteEpargne : public CompteBancaire {
public:
    void afficherType() override {                   // "override" = on redéfinit
        cout << "Type : Compte Epargne (Taux: " << tauxInteret << "%)" << endl;
    }
};
```

**La magie du polymorphisme :**
```cpp
// La méthode afficherInfo() appelle afficherType()
// C++ choisit AUTOMATIQUEMENT la bonne version !
monCompte.afficherInfo();
// → "Type : Compte Epargne (Taux: 3.5%)"
```

> **Analogie** : Dire "parle !" à un chien → il **aboie**.  
>               Dire "parle !" à un chat  → il **miaule**.  
>               Même commande, comportement différent selon l'objet.

---

## Architecture globale du projet

```
┌─────────────────────────────────────────────────────┐
│                  DEMO_ATM_SYSTEM.CPP                │
├─────────────────────────────────────────────────────┤
│                                                     │
│   ┌──────────────────────────────┐                  │
│   │     CompteBancaire           │ ← CLASSE PARENT  │
│   │  ─────────────────────────  │                  │
│   │  - titulaire : string        │ ← Encapsulation  │
│   │  - solde : double            │   (private)      │
│   │  ─────────────────────────  │                  │
│   │  + CompteBancaire(...)       │ ← Constructeur   │
│   │  + ~CompteBancaire()         │ ← Destructeur    │
│   │  + getSolde()                │ ← Getter         │
│   │  + deposer(montant)          │ ← Méthode        │
│   │  + retirer(montant)          │ ← Méthode        │
│   │  + afficherInfo()            │ ← Méthode        │
│   │  + virtual afficherType()    │ ← Polymorphisme  │
│   └──────────────┬───────────────┘                  │
│                  │ Héritage                         │
│   ┌──────────────▼───────────────┐                  │
│   │     CompteEpargne            │ ← CLASSE ENFANT  │
│   │  ─────────────────────────  │                  │
│   │  - tauxInteret : double      │ ← Attribut +     │
│   │  ─────────────────────────  │                  │
│   │  + CompteEpargne(...)        │ ← Constructeur   │
│   │  + appliquerInterets()       │ ← Méthode +      │
│   │  + afficherType() override   │ ← Polymorphisme  │
│   └──────────────────────────────┘                  │
│                                                     │
│   ┌──────────────────────────────┐                  │
│   │     main()                   │ ← Programme      │
│   │  monCompte = CompteEpargne   │   principal      │
│   │  Boucle menu ATM             │                  │
│   └──────────────────────────────┘                  │
└─────────────────────────────────────────────────────┘
```

---

## Tableau récapitulatif

| Concept POO | Mot-clé C++ | Où dans le code | But |
|---|---|---|---|
| **Classe** | `class` | `CompteBancaire`, `CompteEpargne` | Définir un modèle |
| **Objet** | — | `monCompte` dans `main()` | Créer un exemplaire |
| **Encapsulation** | `private` / `public` | Attributs de `CompteBancaire` | Protéger les données |
| **Constructeur** | Même nom que la classe | `CompteBancaire(...)` | Initialiser l'objet |
| **Destructeur** | `~NomClasse()` | `~CompteBancaire()` | Libérer l'objet |
| **Héritage** | `: public` | `CompteEpargne : public CompteBancaire` | Réutiliser le code |
| **Polymorphisme** | `virtual` / `override` | `afficherType()` | Comportement adaptatif |

---

## 🛠️ Comment compiler et exécuter

```bash
# Compiler
g++ DEMO_ATM_SYSTEM.CPP -o atm

# Exécuter
./atm
```

---

## 🎯 Ce que vous devez retenir

> **Les 5 piliers POO présents dans ce projet :**
> 1. ✅ **Classe / Objet** — `class CompteBancaire` → `monCompte`
> 2. ✅ **Encapsulation** — `private` + getters/setters
> 3. ✅ **Constructeur/Destructeur** — initialisation automatique
> 4. ✅ **Héritage** — `CompteEpargne : public CompteBancaire`
> 5. ✅ **Polymorphisme** — `virtual` + `override`

> **Pour bien comprendre, lisez le code dans cet ordre :**
> 1. La classe `CompteBancaire` (en haut du fichier)
> 2. La classe `CompteEpargne` (au milieu)
> 3. La fonction `main()` (en bas)

---

*Document rédigé pour un examen éducatif sur la Programmation Orientée Objet en C++*
