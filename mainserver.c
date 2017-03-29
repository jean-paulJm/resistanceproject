#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#include <pthread.h>

# define END 3 

int portno;
int nbj;  // compteur joueur == nbj pour commencer
int nbespions;
int meneurCourant;
int compteurJoueurs; // pour compter les joueurs
int compteurMissions; // pour savoir à quelle mission on est
int compteurVotes; // combien de votes ont été éffectués
int compteurReussites; // combien de vote reussite
int compteurRebelles; // combien de rebelles
int compteurEspions;  // combien d'Espions
int participantsMissions[5]={2,2,3,3,2};	// pour savoir combien de participant à la mission
int Nbparticipants = 1;
char serverbuffer[256];

char com;
char adrip[20];
int dportno;
char name[20];
char team[10];
//int equipe[10];


/* PRINCIPE :
		attente 'C'
		remplir la structure
		renvoyer le joueur aux autres joueurs
		si compteur == nbj
		fsmstate = 101 ;

*/
struct joueur
{
	char nom[20];
	char ipaddress[20];
	int portno;
	int equipe; // 0=pas dans equipe, 1=dans equipe
	int role; // 0=rebelle, 1=espion
	int vote; // 0=refus, 1=accept
	int reussite; // 0=echec, 1=reussite
} tableauJoueurs[5];
int roles[5];


void sendMessage(int j, char *mess);
void broadcast(char *message);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void initRoles(int *roles)
{
	int i;
	for(i=0;i<5;i++)
	{
		roles[i]=0;
	}
	srand(time(NULL)); // initialisation de rand
	int case_aleatoire;
	for(i=0; i<2; i++)  //attribue le role d'espion
	{
		do{
	        case_aleatoire = rand()%5;
		} while(roles[case_aleatoire]==1);
		roles[case_aleatoire]=1;
	}
	for(i=0; i<5; i++)
	{
		printf("%d ", roles[i]);
	}
	printf("\n");
}

void *server(void *ptr)  //
{
     int sockfd, newsockfd; //CRÉE UNE PRISE VIDE DANS L'APPLICATion
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,// je dis à cette prise qu'on est lappli qui a pour port 32000  
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5); //ecoute jusqu'à 5 connections 
     clilen = sizeof(cli_addr);
     while (1)  //serveur qui s'arrête jamais
     {
  	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //accept:appel système bloquant , tant qu'aucun client se connecte le serveur reste tel quel
  	if (newsockfd < 0)
       	error("ERROR on accept");
  	bzero(serverbuffer,256);
  	n = read(newsockfd,serverbuffer,255); // lire l'accept,et on remplit le buffer serverbuffer
  	if (n < 0) error("ERROR reading from socket");
  	printf("Here is a message from a client: '%s' '%c'\n",serverbuffer,serverbuffer[0]);

	if ( serverbuffer[0] == 'C' )  // si client demande message par C alors demande de faire quelque chose
	{
		char connect;
		char mess[100];

		printf("Commande C\n");
		sscanf ( serverbuffer , "%c %s %d %s " , &connect ,
			tableauJoueurs[compteurJoueurs].ipaddress , 
			&tableauJoueurs[compteurJoueurs].portno , 
			tableauJoueurs[compteurJoueurs].nom ) ;
		sprintf(mess,"C %s %d",tableauJoueurs[compteurJoueurs].nom,compteurJoueurs);// construit message qui prendra les joueurs et le nombre de joueurs
		compteurJoueurs++;
		broadcast(mess); // diffusion du message à tout le monde
	}
  	close(newsockfd); //close tout et on attend une nouvelle requete
     }
     close(sockfd);
}

void sendMessage(int j, char *mess)
{
        int sockfd, n;
        struct sockaddr_in serv_addr;
        struct hostent *playerserver;
        char buffer[256];

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket");
        playerserver = gethostbyname(tableauJoueurs[j].ipaddress);
        if (playerserver == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)playerserver->h_addr, (char *)&serv_addr.sin_addr.s_addr,
                                playerserver->h_length);
        serv_addr.sin_port = htons(tableauJoueurs[j].portno);
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
                        error("ERROR connecting");

        n = write(sockfd,mess,strlen(mess));
        if (n < 0)
                error("ERROR writing to socket");
        close(sockfd);
}

void broadcast(char *message)   //diffuse le meme message à tous les joueurs
{
        int i;

        printf("broadcast %s\n",message);
        for (i=0;i<compteurJoueurs;i++)
                sendMessage(i,message); // c'est un client TCP , car il fait des socket, write et connect

}

void sendRoles(roles) //envoyer les roles une fois qu'on aura tiré au sort
{
	
}

void sendMeneur()  // dis si on est meneur ou non , c'est le joueur a gauche
{
}

void sendEquipe() //renvoie l'equipe qui a ete consitué par le meneur
{
}

void sendChosenOnes()  // on accepte ou rejette une mission choisi par le meneur
{
}

int main(int argc, char *argv[])
{
     pthread_t thread1, thread2; //contient les num de thread
     int  iret1, iret2;  // code de retour des thread

	if (argc!=3)   // si on est serveur du jeu on a besoin argument du joueur , de mainserver, et  numéro de port
	{
		printf("Usage : ./mainserver nbjoueurs numport\n");
		exit(1);
	}

     com='0';
     nbj=atoi(argv[1]);   // on transforme l'arg en entier
     printf("Nombre de joueurs=%d\n",nbj);
     portno=atoi(argv[2]);
     printf("Serveur ecoute sur port %d\n",portno);
     compteurJoueurs=0;   // initialisation

     compteurRebelles=0;
     compteurEspions=0;
     compteurMissions=0;
     meneurCourant = 0;
     nbespions=1;
     initRoles(roles);  //à faire

    /* Create independent threads each of which will execute function */

     iret1 = pthread_create( &thread1, NULL, server, NULL);   //on crée le premier thread , la fonction doit etre de type static void, pthread créé et son numéro est thread1
     if(iret1)  // si pas égale à 0
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
         exit(EXIT_FAILURE);
     }

     printf("pthread_create() for thread 1 returns: %d\n",iret1);

     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

     pthread_join( thread1, NULL);

     exit(0);
}
