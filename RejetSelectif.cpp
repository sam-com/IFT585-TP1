#include <iostream>
#include "Connexion.h"
#include "Trame.h"

#define dim 7
#define dim_BUFS (dim+1)/2

using namespace std;

int main()
{
    /*  int tamponEnvoi[dim];
      int tamponReception[dim];
      int fenetreRecept[dim_BUFS];
      int fenetreEmet[dim_BUFS]; */


//
    bool reception[dim_BUFS];

//enum etat {SEND, RECEIVE};
//etat etatActuel;

    for(int i=0; i<dim_BUFS; i++)
    {
        reception[i]= false;
    }


    void EmetteurRecepteur(Trame trame)
    {
        bool acquittement = true;
            int outOfRange = dim_BUFS;
        int trameDesire=0; // numero de la trame attendue

        for(int i=0; ; i++)
        {

            nextEvent(&situation);
            switch(situation)
            {
            case receptionTrame:

                FromPhysicalLayer(&trame);
                if(trame.getType()== 0)
                {
                    //etatActuel = RECEIVE;

                    if(getSequence()!= trameDesire && acquittement)
                        retour(trameDesire,nak,0,sortie); //sortie= couche physique et retour
                    /*else
                    to do

                    faire partir le timer ???
                    */
                    if ( intervalle(trameDesire,trame.getSequence(), outOfRange) && (reception[trame.getSequence()%dim_BUFS]== false))
                    {
                        reception[trame.getSequence()%dim_BUFS]=true;
                        entree[trame.getSequence()%dim_BUFS]=trame.getDonnees(); // buffer d'entree??----> introduire les donnees dans le tampon
                        for(int i= 0; i<dim_BUFS; i++)
                        {
                            if(reception[i]== true)
                            {
                                toNetworkLayer(& entree[trame.getSequence()%dim_BUFS]); // toNetworkLayer
                                acquittement= true;
                                reception[trame.getSequence()%dim_BUFS]= false;
                                trameDesire++;
                                outOfRange++;

                            }
                        }

                    }

                }



            }

        }
    }




    return 0;
}

