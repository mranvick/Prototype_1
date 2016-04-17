#ifndef PROTOTYPE_1_MATRICEP_H
#define PROTOTYPE_1_MATRICEP_H

#include "CouchesParticules.h"

// Classe définie à partir de CouchesParticules(MatriceCreuse)
// Les méthodes de gestion des particules sont hybrides : soit à partir du tableau de particules,
// soit en parcours en profondeur
template<size_t ...dims>
class MatriceParticules : public CouchesParticules<dims...>
{
    using Parent = CouchesParticules<dims...>;

public:
    MatriceParticules(size_t w, size_t h, Particule* particules, int nbParticules)
     : Parent(w,h), m_part(particules), m_nbPart(nbParticules)
    {
        ajouterParticules();
        this->actualiserBarycentre();
    }

    // Ajoute les particules de m_particules dans la matrice
    void ajouterParticules()
    {
        for(int i = 0 ; i < m_nbPart ; i++)
        {
            Particule* p = &m_part[i];
            if (estValide(*p))
                this->set(p->getXInt(), p->getYInt(), p);
            else
                p->supprimerLiaisons();
        }
    }

    // Réinitialise la matrice de particules à partir du tableau
    // afin qu'elle soit gérée de manière correcte.
    virtual void reinit()
    {
        // Suppression du contenu de la matrice Creuse
        Parent::reinit();

        // Rajout des particules dans la matrice
        ajouterParticules();
        this->actualiserBarycentre();

        // Suppression des forces rémanentes
        for(int i = 0 ; i < m_nbPart ; i++)
            m_part[i].annulerForces();
    }

    // Indique si une particule est bien dans la matrice
    bool estValide(Particule& p)
    {
        int x = p.getXInt();
        int y = p.getYInt();
        return (x >= 0 && x < this->m_w && y >= 0 && y < this->m_h);
    }

    // Calcule et applique les forces de liaison entre les particules
    void forcesLiaison()
    {
        for(int i = 0 ; i < m_nbPart ; i++)
        {
            Particule& p = m_part[i];
            if (estValide(p))
                p.appliquerForcesLiaison();
        }
    }

    //  Calcule la prochaine position des particules
    void calculerDeplacement(double dt)
    {
        for(int i = 0 ; i < m_nbPart ; i++)
        {
            Particule& p = m_part[i];
            if (estValide(p))
                p.calculerDeplacement(dt);
        }
    }

    // Effectue le déplacement des particules dans la matrice
    void deplacer(double dt, int coucheCollision)
    {
        for(int i = 0 ; i < m_nbPart ; i++)
        {
            Particule& p = m_part[i];
            if (estValide(p))
            {
                int xOldPart = p.getXInt();
                int yOldPart = p.getYInt();

                Vecteur pos = p.getPos(); // Position où mettre le pixel
                int xNouvPart = (int)pos.getX();
                int yNouvPart = (int)pos.getY();

                // On tente de mettre la particule aux coordonnees (x,y)

                // Seulement si la particule bouge :
                if (xOldPart != xNouvPart || yOldPart != yNouvPart)
                {
                    // Si la particule sort de la grille
                    if (xNouvPart < 0 || xNouvPart >= this->m_w || yNouvPart < 0 || yNouvPart >= this->m_h)
                    {
                        // On la supprime
                        p.supprimerLiaisons();
                        this->suppr(xOldPart,yOldPart);
                    }
                    // Sinon, on calcule les collisions
                    else if (!this->gererCollision(p, xNouvPart, yNouvPart, coucheCollision))
                    {
                        // S'il n'y a pas eu de collision, on bouge la particule dans la grille (pourrait être fait dans gererCollision() ?)
                        p.setInt(xNouvPart, yNouvPart);
                        this->set(xNouvPart, yNouvPart, &p);
                        this->suppr(xOldPart, yOldPart);
                        // Le faire dans cet ordre est plus optimisé : s'il y a des SM vidées puis reremplies,
                        // elles seraient détruites puis recrées
                    }
                }
            }
        }
    }

    // Calcule la frame suivante, à partir des méthodes ci-dessus
    void actualiser(double dt, int coucheCollision)
    {
        // Calculer la force à appliquer et l'appliquer à chaque particule
        forcesLiaison();

        // Modifier les coordonnées de ces particules
        calculerDeplacement(dt);

        // Deplacer effectivement ces coordonnées dans la grille
        deplacer(dt, coucheCollision);

        // Actualisation des barycentres
        this->actualiserBarycentre();
    }

    void afficherLiaisons(SDL_Renderer* rendu, int coucheAffichage, double tailleParticule)
    {
        SDL_SetRenderDrawColor(rendu, 0,255,0,60);

        for(int i = 0 ; i < m_nbPart ; i++)
            m_part[i].afficherLiaisons(rendu, coucheAffichage, tailleParticule);
    }

private:
    Particule* m_part; // Tableau des particules à gérer
    int m_nbPart; // Nombre de particules dans le tableau
};

#endif //PROTOTYPE_1_MATRICEP_H
