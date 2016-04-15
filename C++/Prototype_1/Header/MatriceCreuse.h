#ifndef PROTOTYPE_1_MATRICECREUSE_H
#define PROTOTYPE_1_MATRICECREUSE_H

#include "Conteneur.h"

// Conteneur défini de manière récursive
template<typename T, size_t dimSM>
class MatriceCreuse : public Conteneur
{
    // On suppose que T est ici forcément un type de MatriceCreuse !
    // Le cas de Particule sera géré plus tard

public:
    MatriceCreuse(size_t w, size_t h)
     : Conteneur(), m_tab(NULL),
       m_w(w), m_h(h), m_smX((int)ceil((double)w/(double)dimSM)), m_smY((int)ceil((double)h/(double)dimSM))
    {}

    // Constructeur par défaut (utlisé uniquement par le conteneur !!!)
    MatriceCreuse()
     : Conteneur(), m_tab(NULL)
    {}

    // Utilisé après pour définir les dimensions
    void setDim(size_t dim)
    {
        m_w = dim;
        m_h = dim;
        m_smX = dim/dimSM;
        m_smY = m_smX;
    }

    ~MatriceCreuse()
    {
        if (m_tab != NULL)
            delete [] m_tab;
    }

    static inline int getProfondeur() { return 1+T::getProfondeur(); }

    virtual void actualiserBarycentre()
    {
        if (m_tab==NULL)
        {
            m_masse = 0.0;
            m_pTot = 0;
            m_nbE = 0;
        }
        else
        {
            // Valeurs physiques
            m_masse = 0.0;
            m_pos = 0.0;
            m_v = 0.0;

            // Variables pour la couleur
            m_pTot = 0;
            double r = 0.0, g = 0.0, b = 0.0, a = 0.0;

            for(int i = 0 ; i < m_smX*m_smY ; i++)
            {
                m_masse += m_tab[i].getMasse();
                m_pos += m_tab[i].getPos();
                m_v += m_tab[i].getV();

                m_pTot += m_tab[i].getPTot();
                SDL_Color c = m_tab[i].getCouleur();
                r += (double)c.r;
                g += (double)c.g;
                b += (double)c.b;
                a += (double)c.a;
            }
            m_couleur = {
                    (Uint8)(r/(double)m_pTot),
                    (Uint8)(g/(double)m_pTot),
                    (Uint8)(b/(double)m_pTot),
                    (Uint8)(a/(double)m_pTot) };
        }
    }

    // Retourne true si la matrice était auparavant vide, et qu'elle se remplit
    bool set(int x, int y, Particule* p)
    {
        if (x < 0 || x >= m_w || y < 0 || y >= m_h)
            return false;

        // Création du tableaux des sous-matrices, s'il n'existait pas encore
        if (m_tab == NULL)
        {
            m_tab = new T[m_smX*m_smY];
            for(int i = 0 ; i < m_smX*m_smY ; i++)
                m_tab[i].setDim(dimSM);
        }

        // Coordonnées de la sous-matrice concernée
        int sx = x/dimSM;
        int sy = y/dimSM;
        int indSM = sx*m_smY + sy;
        T& sm = m_tab[indSM];

        // Insertion du nouvel élément dans la sous-matrice
        if (sm.set(x%dimSM, y%dimSM, p))
        {
            m_nbE++;
            return m_nbE==1;
        }
        else
            return false;
    }

    // Retourne true si cela vide la matrice
    bool suppr(int x, int y)
    {
        if (m_tab == NULL || x < 0 || x >= m_w || y < 0 || y >= m_h)
            return false;

        // Coordonnées de la sous-matrice concernée
        int sx = x/dimSM;
        int sy = y/dimSM;
        int indSM = sx*m_smY + sy;

        T& sm = m_tab[indSM];

        // On vide le niveau inférieur
        if (sm.suppr(x%dimSM,y%dimSM))
        {
            // Si cela vide la sous-matrice, on la supprime
            m_nbE--;
            if (m_nbE == 0)
            {
                delete[] m_tab;
                m_tab = NULL;
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }

    // Supprime tous les sous-éléments, après les avoir vidés
    virtual void reinit()
    {
        if (m_tab != NULL)
        {
            for(int i = 0 ; i < m_smX*m_smY ; i++)
                m_tab[i].reinit();

            delete[] m_tab;
            m_tab = NULL;
            Conteneur::reinit();
        }
    }

    // Retourne la particule contenue aux coordonnées (x,y)
    Particule* get(int x, int y)
    {
        if (m_tab == NULL)
            return NULL;

        // Coordonnées de la sous-matrice concernée
        int sx = x/dimSM;
        int sy = y/dimSM;
        int indSM = sx*m_smY + sy;

        T& sm = m_tab[indSM];
        return sm.get(x%dimSM,y%dimSM);
    }

    virtual void afficher(SDL_Renderer* rendu, int coucheAffichage, int taillePixel)
    {
        afficher(rendu, coucheAffichage, taillePixel, 0, 0);
    }

    // Calcule les couleurs des pixels, et les affiche sur le rendu SDL
    virtual void afficher(SDL_Renderer* rendu, int coucheAffichage, int taillePixel, int x, int y)
    {
        if (coucheAffichage < getProfondeur())
        {
            if (m_tab != NULL)
            {
                for(int i = 0 ; i < m_smX ; i++)
                {
                    for(int j = 0 ; j < m_smY ; j++)
                        m_tab[i*m_smY+j].afficher(rendu, coucheAffichage, taillePixel, x+dimSM*i, y+dimSM*j);
                }
            }
        }
        else if (coucheAffichage == getProfondeur())
        {
            SDL_SetRenderDrawColor(rendu, m_couleur.r, m_couleur.g, m_couleur.b, m_couleur.a);
            SDL_Rect rect = {taillePixel*x, taillePixel*y,taillePixel,taillePixel};
            SDL_RenderFillRect(rendu, &rect);
        }
    }

    virtual void actualiser(double dt)
    {
        if (m_tab != NULL)
        {
            for(int i = 0 ; i < m_smX*m_smY ; i++)
                m_tab[i].actualiser(dt);
        }
    }

protected:
    T* m_tab; // Tableaux des sous-éléments

    size_t m_w, m_h; // Dimensions
    int m_smX, m_smY; // Nombre de sous-matrices selon X et Y
};

// Spécialisation : Cas de base = matrice simple de particules, n'allouant pas forcément son tableau
template<>
class MatriceCreuse<Particule,1> : public Conteneur
{
public:
    MatriceCreuse(size_t w, size_t h)
     : Conteneur(), m_tab(NULL),
       m_w(w), m_h(h)
    {}

    // Constructeur par défaut (utlisé uniquement par le conteneur !!!)
    MatriceCreuse()
     : Conteneur(), m_tab(NULL)
    {}

    // Utilisé après pour définir les dimensions
    void setDim(size_t dim)
    {
        m_w = dim;
        m_h = dim;
    }

    ~MatriceCreuse()
    {
        if (m_tab != NULL)
            delete [] m_tab;
    }

    static inline int getProfondeur() { return 1; }

    virtual void actualiserBarycentre()
    {
        if (m_tab==NULL)
        {
            m_masse = 0.0;
            m_pTot = 0;
            m_nbE = 0;
        }
        else
        {
            // Valeurs physiques
            m_masse = 0.0;
            m_pos = 0.0;
            m_v = 0.0;

            // Variables pour la couleur
            m_pTot = 0;
            double r = 0.0, g = 0.0, b = 0.0, a = 0.0;

            for(int i = 0 ; i < m_w*m_h ; i++)
            {
                if (m_tab[i] != NULL)
                {
                    m_masse += m_tab[i]->getMasse();
                    m_pos += m_tab[i]->getPos();
                    m_v += m_tab[i]->getV();

                    m_pTot++;
                    SDL_Color c = m_tab[i]->getCouleur();
                    r += (double)c.r;
                    g += (double)c.g;
                    b += (double)c.b;
                    a += (double)c.a;
                }
            }
            m_couleur = {
                    (Uint8)(r/(double)m_pTot),
                    (Uint8)(g/(double)m_pTot),
                    (Uint8)(b/(double)m_pTot),
                    (Uint8)(a/(double)m_pTot) };
        }
    }

    // Retourne true si la matrice était auparavant vide, et qu'elle se remplit
    bool set(int x, int y, Particule* p)
    {
        if (x < 0 || x >= m_w || y < 0 || y >= m_h)
            return false;

        // Création du tableaux des sous-matrices, s'il n'existait pas encore
        if (m_tab == NULL)
        {
            m_tab = new Particule*[m_w*m_h];
            for(int i = 0 ; i < m_w*m_h ; i++)
                m_tab[i]=NULL;
        }

        // Coordonnées de la particule concernée
        int indP = x*m_h + y;
        Particule*& p2 = m_tab[indP];

        // Insertion du nouvel élément dans la sous-matrice
        if (p2 == NULL)
        {
            p2 = p;
            m_nbE++;
            return m_nbE==1;
        }
        else
            return false;
    }

    // Retourne true si cela vide la matrice
    bool suppr(int x, int y)
    {
        if (m_tab == NULL || x < 0 || x >= m_w || y < 0 || y >= m_h)
            return false;

        // Coordonnées de la particule concernée
        int indSM = x*m_h + y;

        Particule*& p2 = m_tab[indSM];
        if (p2 == NULL) // (x,y) est déjà vide...
            return false; // Cette matrice n'est pas plus vide qu'avant donc

        // On supprime la particule
        p2 = NULL;

        // Si cela vide la sous-matrice, on la supprime
        m_nbE--;
        if (m_nbE == 0)
        {
            delete[] m_tab;
            m_tab = NULL;
            return true;
        }
        else
            return false;
    }

    virtual void reinit()
    {
        if (m_tab != NULL)
        {
            for(int i = 0 ; i < m_w*m_h ; i++)
                m_tab[i] = NULL;

            delete[] m_tab;
            m_tab = NULL;
            Conteneur::reinit();
        }
    }

    Particule* get(int x, int y)
    {
        if (m_tab == NULL)
            return NULL;

        return (m_tab[x*m_h + y]);
    }

    virtual void afficher(SDL_Renderer* rendu, int coucheAffichage, int taillePixel)
    {
        afficher(rendu, coucheAffichage, taillePixel, 0, 0);
    }

    virtual void afficher(SDL_Renderer* rendu, int coucheAffichage, int taillePixel, int x, int y)
    {
        if (coucheAffichage == 0)
        {
            if (m_tab != NULL)
            {
                for(int i = 0 ; i < m_w ; i++)
                {
                    for(int j = 0 ; j < m_h ; j++)
                    {
                        Particule* p = m_tab[i*m_h+j];
                        if (p != NULL)
                            p->afficher(rendu, coucheAffichage, taillePixel);
                    }
                }
            }
        }
        else if (coucheAffichage == 1)
        {
            SDL_SetRenderDrawColor(rendu, m_couleur.r, m_couleur.g, m_couleur.b, m_couleur.a);
            SDL_Rect rect = {taillePixel*x, taillePixel*y,taillePixel,taillePixel};
            SDL_RenderFillRect(rendu, &rect);
        }
    }

    virtual void actualiser(double dt)
    {
        if (m_tab != NULL)
        {
            for(int i = 0 ; i < m_w*m_h ; i++)
            {
                if (m_tab[i] != NULL)
                    m_tab[i]->actualiser(dt);
            }
        }
    }

protected:
    Particule** m_tab; // Tableaux des particules

    size_t m_w, m_h; // Dimensions
};

// DEBUG : sert à avoir un apperçu du conteneur de particules
template<typename T, size_t dimSM>
void afficher(MatriceCreuse<T, dimSM>& m, int w, int h)
{
    std::cout << "Nombre de sous-elements : " << m.getNbE() << std::endl;
    std::cout << "Profondeur : " << m.getProfondeur() << std::endl;
    for(int j = 0 ; j < h ; j++)
    {
        for(int i = 0 ; i < w ; i++)
            std::cout << (m.get(i,j)!=NULL) << " ";
        std::cout << std::endl;
    }
}

#endif //PROTOTYPE_1_MATRICECREUSE_H
