#include "../Header/Particule.h"
#include "../Header/Definitions.h"

Particule::Particule(int x, int y, Matiere* matiere)
 : m_x(x), m_y(y), m_pos(x+0.5, y+0.5), m_v(), m_resf(), m_matiere(matiere)
{
    m_liaisons = new Particule*[def::nbLiaisons];
    for(int i = 0 ; i < def::nbLiaisons ; i++)
        m_liaisons[i] = NULL;
}

Particule::Particule(int x, int y, double xd, double yd, Matiere* matiere)
 : m_x(x), m_y(y), m_pos(xd,yd), m_v(), m_resf(), m_matiere(matiere)
{
    m_liaisons = new Particule*[def::nbLiaisons];
    for(int i = 0 ; i < def::nbLiaisons ; i++)
        m_liaisons[i] = NULL;
}

Particule::~Particule()
{
    delete[] m_liaisons;
}

Particule& Particule::operator=(const Particule& p)
{
    m_x = p.m_x;
    m_y = p.m_y;
    m_pos = p.m_pos;
    m_v = p.m_v;
    m_resf = p.m_resf;
    m_matiere = p.m_matiere;

    creerLiaisons(p.m_liaisons);

    return *this;
}

void Particule::creerLiaisons(Particule** liaisons)
{
    m_liaisons=new Particule*[def::nbLiaisons];
    for(int i=0;i<def::nbLiaisons;i++)
    {
        m_liaisons[i]=liaisons[i];
    }
}

bool Particule::lier(Particule* p)
{
    // Une particule ne peut être liée à elle-même
    if (this == p)
        return false;

    //Recherche des indices dans les tableaux de liaisons
    int i, j;
    for(i = 0 ; i < def::nbLiaisons && m_liaisons[i] != NULL && m_liaisons[i] != p ; i++) ;
    if (i == def::nbLiaisons)
        return false;

    for(j = 0 ; j < def::nbLiaisons && p->m_liaisons[j] != NULL && p->m_liaisons[j] != this ; j++) ;
    if (j == def::nbLiaisons)
        return false;

    m_liaisons[i] = p;
    p->m_liaisons[j] = this;

    return true;
}

void Particule::setInt(int x, int y)
{
    m_x = x;
    m_y = y;
}

void Particule::setPos(Vecteur pos)
{
    m_pos = pos;
}


double Particule::getMasse() const
{
    if (m_matiere == NULL)
        return 0.0;
    return m_matiere->getMasse(*this);
}

void Particule::appliquerForce(Vecteur f)
{
    m_resf += f;
}

// EULER
void Particule::calculerDeplacement(double dt)
{
    if (m_matiere != NULL)
    {
        Vecteur a = 1.0/getMasse() * m_resf;
        m_pos += dt*(m_v + 0.5*dt*a);
        m_v += dt*a;
        m_resf = Vecteur();
    }
}

void Particule::supprimerLiaisons() {
    // Supprime les liaisons, de cette particule et des particules qui y sont liées
    for(int i = 0 ; i < def::nbLiaisons ; i++)
    {
        if (m_liaisons[i] != NULL)
        {
            for(int j = 0 ; j < def::nbLiaisons ; j++)
            {
                if (m_liaisons[i]->m_liaisons[j] == this)
                {
                    m_liaisons[i]->m_liaisons[j] = NULL;
                    break;
                }
            }
            m_liaisons[i] = NULL;
        }
    }
}

void Particule::appliquerForcesLiaison()
{
    if (m_liaisons == NULL)
        return;

    for(int i = 0 ; i < def::nbLiaisons ; i++)
    {
        Particule* p = m_liaisons[i];
        if (p != NULL)
            appliquerForce(m_matiere->forceLiaison(this, p));
    }
}

void Particule::setPosInt(Vecteur pos)
{
    m_pos = pos;
    m_x = (int)pos.getX();
    m_y = (int)pos.getY();
}

void Particule::surligner(SDL_Renderer* rendu, int partPP, int taillePixel, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(rendu, 0, 0, 255, 255);
    SDL_Rect rect = {taillePixel*m_x, taillePixel*m_y,taillePixel,taillePixel};
    SDL_RenderFillRect(rendu, &rect);
}

void Particule::collision(Particule& p, double dt)
{
    // A ce stade, p et cette particule sont dans la même "boîte"

    double xCol, yCol; // Coordonnées double de la collision (au bord de la "boîte" de p)
    double vx = m_v.getX();
    double vy = m_v.getY();
    bool deplacementX = false; // Détecte si cette particule s'est déplacé selon m_x (int)

    // Réglage de la position : au bord de la "boîte"
    double newX = m_pos.getX();
    double newY = m_pos.getY();

    if (p.m_x < m_x)
    {
        newX = (double)m_x;

        // Logiquement, vx != 0.0
        xCol = (double)(p.m_x+1);
        yCol = m_pos.getY() + vy/vx*((double)(p.m_x+1)-m_pos.getX());
        deplacementX = true;
    }
    else if (p.m_x > m_x)
    {
        newX = (double)(m_x+1);

        // Logiquement, vx != 0.0
        xCol = (double)p.m_x;
        yCol = m_pos.getY() + vy/vx*((double)p.m_x-m_pos.getX());
        deplacementX = true;
    }

    if (p.m_y < m_y)
    {
        newY = (double)m_y;

        if (deplacementX && yCol > (double)(p.m_y+1))
        {
            // Logiquement, vy != 0.0
            yCol = (double)(p.m_y+1);
            xCol = m_pos.getX() + vx/vy*((double)(p.m_y+1)-m_pos.getY());
        }
    }
    else if (p.m_x > m_x)
    {
        newY = (double)(m_y+1);

        if (deplacementX && yCol < (double)p.m_y)
        {
            // Logiquement, vy != 0.0
            yCol = (double)p.m_y;
            xCol = m_pos.getX() + vx/vy*((double)p.m_y-m_pos.getY());
        }
    }

    m_pos = Vecteur(newX,newY);

    // Calcul de la force de collision (peut être optimisé ?)
    Vecteur n(
            Point((double)p.m_x+0.5,(double)p.m_y+0.5), // Centre
            Point(xCol,yCol) ); // Point de collision
    Vecteur vr = m_v - p.m_v; // Vitesse relative
    Vecteur dv = -2.0 * (vr*n)/n.normeCarre()*n; // Variation de vitesse à la collision
    Vecteur f = 1.0/getMasse()/dt * dv; // Force correspondante sur cette particule

    // Application de la force de collision
    appliquerForce(f);
    p.appliquerForce(-f);
}