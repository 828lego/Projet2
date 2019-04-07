#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

enum Etat {sain, immunise, malade, mort}; //états possibles des individus

int nombre_jours; //nombre de jours de la simulation
int nombre_malade; //nombre de malade initial
double lambda = 0.2; //probabilité de tomber malade
double beta = 0.95; //probabilité de mourir
double gama = 0.95; //probabilité d'être immunisé

//création d'une liste chainee des liens entre les individus
typedef struct _element{
    int numero_voisin;
    struct _element *suiv;
} Element;

//creation de la structure du graphe
typedef struct {
    int nombre_sommets; //nombre de sommets du graphe
    int nombre_arretes; //nombre d'arretes du graphe
    Element** liste_successeurs; //tableau de tetes des listes chainees
    enum Etat* etat_individu_actuel; //tableau des etats des individus au tour actuel
    enum Etat* etat_individu_futur; //tableau des etats des individus au tour suivant
} Graph;

void creation_txt(); //creation d'un graphe dans un fichier texte
void creer_graph(); //creation d'un graphe
void infection(); //infecte un individu du graphe
void afficher_graph(); //affiche le graphe cree
void evolution_graph(); //simule une journee du graphe
void afficher_etat(); //affiche les etats des individus
void jour_initial(); //affiche l'etat des individus du jour 0



//creation d'un graphe de dimension n*n aleatoire dans un fichier texte
void creation_txt(int n,const char* nom_graphe) {
    FILE *fp;
    fp = fopen(nom_graphe,"w");
    fprintf(fp,"%d\n%d\n", n*n, n*(n-1)*2);

    //creation des arretes lignes:
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n-1; j++){
            fprintf(fp, "%d %d \n", i*n +j, i*n +j+1);
        }
    }
    //creation des arretes colones:
    for (int i = 0; i < n-1; i++){
        for (int j = 0; j < n; j++){
            fprintf(fp, "%d %d\n", i*n +j, i*n +j+n);
        }
    }
    fclose(fp);
}

//creation du graphe a partir du fichier text
void creer_graph(Graph *G, const char *file_name) {
    FILE *fp;
    fp = fopen(file_name, "r");

    if (fp != NULL) {
        int nb_nodes, nb_arcs, u, v;
        fscanf(fp, "%d %d", &nb_nodes, &nb_arcs);
        G->nombre_sommets = nb_nodes;
        G->nombre_arretes = nb_arcs;
        G->liste_successeurs = (Element**) malloc(nb_nodes * sizeof(Element*));
        G->etat_individu_actuel = (enum Etat*) malloc(nb_nodes * sizeof(enum Etat));
        G->etat_individu_futur = (enum Etat*) malloc(nb_nodes * sizeof(enum Etat));

        for (int i = 0; i < nb_nodes; i++) {
            G->liste_successeurs[i] = NULL;
            G->etat_individu_actuel[i] = sain;
        }
        for (int i = 0; i < nb_arcs; i++) {
            fscanf(fp, "%d %d", &u, &v);
            //Ajout d'une arrete
            Element* s	= (Element*) malloc(sizeof(Element));
            s->numero_voisin = v;
            s->suiv = G->liste_successeurs[u];
            G->liste_successeurs[u]	= s;
            //Ajout de l'arrete reciproque
            Element* t	= (Element*) malloc(sizeof(Element));
            t->numero_voisin = u;
            t->suiv = G->liste_successeurs[v];
            G->liste_successeurs[v]	= t;
        }
    }
    else printf("File not found.");
    fclose(fp);
}

//infection d'un certain nombre d'individu dans le graphe
void infection(Graph *G, int nombre_malade) {
    for (int i = 0; i < nombre_malade; ++i) {
        int position_malade = rand()%G->nombre_sommets+1;
        G->etat_individu_actuel[position_malade] = malade;
    }
}

void afficher_graphe(Graph *G) {
    for(int i = 0; i < G->nombre_sommets; i++) {
        printf("Element %d : ", i);
        Element* current = G->liste_successeurs[i];
        while(current != NULL) {
            printf("%d, ", current->numero_voisin);
            current =  current->suiv;
        }
        printf("\n");
    }
}

//fait evoluer le graphe pour un tour
void evolution_graphe(Graph *G) {
    Element *voisin_actuel;
    for (int i = 0; i < G->nombre_sommets; ++i) {
        if (G->etat_individu_actuel[i] == sain) {
            voisin_actuel = G->liste_successeurs[i];
            int nombre_malade = 0;
            //balaye la liste chainee des voisin de l'individu i
            while (voisin_actuel->suiv != NULL) {
                if (G->etat_individu_actuel[voisin_actuel->numero_voisin] == malade) {
                    nombre_malade = nombre_malade + 1; //compte le nombre de malade voisin
                }
                voisin_actuel=voisin_actuel->suiv;
            }
            //si l'individu i n'a pas de voisin malade
            if (nombre_malade==0) {
                G->etat_individu_futur[i]=sain;
            }
            else {
                double aleatoire;
                //probabilite de tomber malade
                if ((double) (rand() % 100) / 100 <= 1 - pow(lambda, nombre_malade)) {
                    G->etat_individu_futur[i] = malade;
                }
                else {
                    G->etat_individu_futur[i] = sain;
                }
            }
        }
        if (G->etat_individu_actuel[i] == malade) {
            //probabilite de devenir immunise
            if ((double)(rand()%100)/100<=1-gama) {
                G->etat_individu_futur[i] = immunise;
            }
            //probabilite de mourir
            else if ((double)(rand()%100)/100<=1-beta) {
                G->etat_individu_futur[i] = mort;
            }
            else {
                G->etat_individu_futur[i] = malade;
            }
        }
        if (G->etat_individu_actuel[i] == immunise) {
            G->etat_individu_futur[i] = immunise;
        }
        if (G->etat_individu_actuel[i] == mort) {
            G->etat_individu_futur[i] = mort;
        }
    }
    for (int j = 0; j < G->nombre_sommets ; ++j) {
        G->etat_individu_actuel= G->etat_individu_futur;
    }
}

void afficher_etat(Graph *G) {
    int dimension_graphe = sqrt(G->nombre_sommets);
    for (int i = 0; i <G->nombre_sommets ; ++i) {
        switch (G->etat_individu_actuel[i]) {
            case sain:
                printf(".");
                break;
            case malade:
                printf("x");
                break;
            case mort:
                printf(" ");
                break;
            case immunise:
                printf("o");
                break;
            default:
                printf("E");
        }
        if ((i+1)%dimension_graphe==0) {
            printf("\n");
        }
    }
}

void jour_initial(Graph *G) {
    printf("Jour 0: \n");
    afficher_etat(G);
    printf("\n");
}

int main() {
    srand(time(NULL)); // permet d'avoir tout le temps des nombres aléatoires
    //creation_txt(20,"test3.txt");
    Graph G;
    creer_graph(&G, "test3.txt");
    printf("Nombre de jours: ");
    scanf("%d",&nombre_jours);
    printf("Nombre de malades: ");
    scanf("%d",&nombre_malade);
    infection(&G,nombre_malade);
    jour_initial(&G);
    for (int i = 0; i < nombre_jours ; ++i) {
        printf("Jour %d: \n",i+1);
        evolution_graphe(&G);
        afficher_etat(&G);
        printf("\n");
    }
    //afficher_graphe(&G);
    return 0;
}
