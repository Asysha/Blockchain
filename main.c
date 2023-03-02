#include "sha256.h"
#include "sha256_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define difficulte 4
#define max_nb_trans 10
#define max_nb_bloc 10
#define max_nb_users 10

// Strucuture générale d'un bloc
struct Bloc {
	int indice, nb_trans, nonce;
	char prev_hash[SHA256_BLOCK_SIZE * 2 + 1],
		hash_root[SHA256_BLOCK_SIZE * 2 + 1],
		curr_hash[SHA256_BLOCK_SIZE * 2 + 1];
	char *timestamp;
	char *liste[max_nb_trans];
};

// Strucuture générale de la blockchain
struct Blockchain {
	struct Bloc liste[max_nb_bloc];
	int diff;
	int nbBloc;
};

// Recuperer l'heure a l'instant
char *getTimeStamp() {
	time_t ltime;
	time(&ltime);
	return ctime(&ltime); // modele :  "Fri Jan 19 13 :40 :09 2018"
}

// pour appeler le sha56 : void sha256ofString(BYTE * str,char
// hashRes[SHA256_BLOCK_SIZE*2 + 1])

// Generer liste de transactions aleatoire (modele : "Source usern-Destination :
// userm $$$$$$$$" soit environ 50caracteres)
void generate_trans(
	char *liste[max_nb_trans],
	int nb_trans,
	char *liste_users[max_nb_users]) {
	// 1 Bnb = 10^8 satoBnb
	long int montant_s;
	// Pour aleatoire
	srand(time(NULL));
	// Boucle de remplissage de la liste
	for (int co = 0; co < nb_trans; co++) {
		// 100*10^8 car MAX_VALUE + conversion en satoBnB
		montant_s = rand() % 1000000000 + 1;
		// Generation de deux users aleatoirement
		int indice_usern = rand() % max_nb_users;
		int indice_userm = rand() % max_nb_users;
		// Verification que les deux utilisateurs sont differents
		while (indice_usern == indice_userm) {
			indice_userm = rand() % max_nb_users;
		}
		char transaction[50];
		// Construction de la chaine de caractere suivant le modele
		sprintf(
			transaction,
			"Source %s - Destination %s %ld",
			liste_users[indice_usern],
			liste_users[indice_userm],
			montant_s);
		// Inscription dans la liste
		liste[co] = malloc(50 * sizeof(char));
		strcpy(liste[co], transaction);
	}
}
// Arbre de Merkle
void mTree(
	char hashRes[SHA256_BLOCK_SIZE * 2 + 1],
	char *liste[max_nb_trans],
	int nb_trans) {
	// Initialisation :
	char hashRes1[SHA256_BLOCK_SIZE * 4 + 2];
	char hashRes2[SHA256_BLOCK_SIZE * 2 + 1];
	int taille = nb_trans;
	char **listeRes = NULL;
	// Liste de taille variable
	listeRes = malloc(nb_trans * sizeof(char *));
	// Verification de l'allocation
	if (listeRes == NULL) {
		perror("erreur d'allocation");
	}
	// Copie de la liste dans la liste dynamique
	for (int i = 0; i < nb_trans; i++) {
		listeRes[i] = malloc((SHA256_BLOCK_SIZE * 2 + 1) * sizeof(char));
		strcpy(listeRes[i], liste[i]);
	}
	int indice;
	// Boucle réalisant l'arbre de merkle
	while (taille > 1) {
		indice = 0;
		for (int co = 0; co < taille; co += 2) {
			sha256ofString((BYTE *)listeRes[co], hashRes1);
			// Si dernier, liste taille impaire donc else, dedoublement du
			// solitaire
			if (co < taille - 1) {
				sha256ofString((BYTE *)listeRes[co + 1], hashRes2);
				strcat(hashRes1, hashRes2);
			} else {
				char hashResTmp[SHA256_BLOCK_SIZE * 4 + 1];
				strcpy(hashResTmp, hashRes1);
				strcat(hashRes1, hashResTmp);
			}
			// On calcule le hash des deux hash concaténés, et on l'inscrit dans
			// la liste
			sha256ofString((BYTE *)hashRes1, listeRes[indice]);
			indice++;
		}
		// Si impair, on ajoute 1 car on debouble le solitaire
		if (taille % 2 == 0) {
			taille = taille / 2;
		} else {
			taille = (taille + 1) / 2;
		}
	}
	// Sauvegarde du resultat
	memcpy(hashRes, listeRes[0], SHA256_BLOCK_SIZE * 2 + 1);
}

// Creatiion d'un bloc quelconque
void initBloc(
	struct Blockchain *blockchain,
	char *liste[max_nb_trans],
	int nb_trans) {
	if ((*blockchain).nbBloc > max_nb_bloc || (*blockchain).nbBloc == 0) {
		printf("Pas de Genesis ou Blockchain pleine, création de bloc "
			   "impossible.\n");
	} else {
		// Sinon pb de depassement de memoire
		srand(time(NULL));
		// Initialisation bloc
		struct Bloc *bloc = malloc(sizeof(struct Bloc));
		// Initialisation timestamp, renvoie chaine type  "Fri Jan 19 13 :40 :09
		// 2018"
		bloc->timestamp = "Fri Jan 19 13 :40 :09 2018";
		// time(&(bloc->timestamp));
		// bloc->timestamp = getTimeStamp();
		// ceci est fourni par le sujet et doit marché mais comment le malloc ?
		// (ligne 140)

		// Initilisation de prev_hash, nb_transaction, liste (la liste de
		// transactions)
		memcpy(
			(*bloc).prev_hash,
			(*blockchain).liste[(*blockchain).nbBloc - 1].curr_hash,
			SHA256_BLOCK_SIZE * 2 + 1);
		(*bloc).nb_trans = nb_trans;
		printf("palier 1\n");
		printf("%ld", strlen(liste[0]));
		for (int co = 0; co < nb_trans; co++) {
			(*bloc).liste[co] = malloc(50 * sizeof(char));
			for (int co2 = 0; co2 < 5; co2++) {
				printf("%c\n", liste[co][co2]);
			}
			strcpy((*bloc).liste[co], liste[co]);
			printf("palier srtcpy\n");
		}
		printf("palier 2\n");
		// Calcul merkle tree
		mTree((*bloc).hash_root, (*bloc).liste, (*bloc).nb_trans);
		printf("palier 3\n");
		// Initialisation nonce et indice
		(*bloc).nonce = 0;
		(*bloc).indice = (*blockchain).nbBloc;
		// Calcul du hash du bloc
		char chaine_curr_hash[(SHA256_BLOCK_SIZE * 2 + 1) * 2 + 50];
		sprintf(
			chaine_curr_hash,
			"%s%s%s%d",
			(*bloc).prev_hash,
			(*bloc).timestamp,
			(*bloc).hash_root,
			(*bloc).nonce);
		sha256ofString((BYTE *)chaine_curr_hash, (*bloc).curr_hash);
		// Ajout du bloc a la blockchain
		printf("palier 4\n");
		(*blockchain).liste[(*blockchain).nbBloc] = *bloc;
		++(*blockchain).nbBloc;
		printf("palier 5\n");
	}
}

// Creation d'un bloc particulier qui distribue 50BnB a tous les utilisateurs
void helicopter_money(
	struct Blockchain *blockchain,
	char *liste_users[max_nb_users]) {
	if ((*blockchain).nbBloc != 1) {
		printf("Pas de bloc Genesis, ou d'autre(s) transaction(s) ont deja ete "
			   "effectuees.\n");
	} else {
		srand(time(NULL));
		// Initialisation bloc
		struct Bloc *bloc = malloc(sizeof(struct Bloc));
		// Initialisation timestamp
		// time(&bloc->timestamp);
		bloc->timestamp = "Fri Jan 19 13 :40 :09 2018";

		// Distribution de 50BnB a tous les users
		for (int co = 1; co < max_nb_users;
			 co++) { // car strlen(liste_user) == 10 moins le creator & qu'on
					 // sait qu'il y moins de transactions que le max_nb_trans
					 // par bloc
			sprintf(
				(*bloc).liste[co - 1],
				"Source coinbase - Destination %s %d",
				liste_users[co],
				50 * 1000 * 10000);
		}
		// Initialisation nb de transactions
		(*bloc).nb_trans = strlen(*liste_users) - 1;
		// Previous hash
		memcpy(
			(*bloc).prev_hash,
			(*blockchain).liste[(*blockchain).nbBloc - 1].curr_hash,
			SHA256_BLOCK_SIZE * 2 + 1);
		// arbre de merkle
		mTree((*bloc).hash_root, (*bloc).liste, (*bloc).nb_trans);
		// Initialisation nonce et indice
		(*bloc).nonce = 0;
		(*bloc).indice = (*blockchain).nbBloc;
		// Calcul du hash du bloc
		char chaine_curr_hash[(SHA256_BLOCK_SIZE * 2 + 1) * 2 + 50];
		sprintf(
			chaine_curr_hash,
			"%s%s%s%d",
			(*bloc).prev_hash,
			(*bloc).timestamp,
			(*bloc).hash_root,
			(*bloc).nonce);
		sha256ofString((BYTE *)chaine_curr_hash, (*bloc).curr_hash);
		// Ajout du bloc a la blockchain
		(*blockchain).liste[(*blockchain).nbBloc] = *bloc;
		++(*blockchain).nbBloc;
	}
}

struct Blockchain *createBlockchain() {
	struct Blockchain *blockchain = malloc(sizeof(struct Blockchain));
	blockchain->diff = difficulte;
	(*blockchain).nbBloc = 0;
	// Initialisation bloc
	struct Bloc *bloc = malloc(sizeof(struct Bloc));
	// Initialisation timestamp, renvoie chaine type  "Fri Jan 19 13 :40 :09
	// 2018" srand(time(NULL));
	bloc->timestamp = "Fri Jan 19 13 :40 :09 2018";
	// Previous hash, liste transaction, nonce, indice
	memcpy(
		(*bloc).prev_hash,
		"0000000000000000000000000000000000000000000000000000000000000000",
		SHA256_BLOCK_SIZE * 2 + 1);
	(*bloc).nb_trans = 1;
	(*bloc).liste[0] = malloc(47 * sizeof(char));
	sprintf(
		(*bloc).liste[0],
		"Source Creator - Destination Creator %d",
		50 * 1000 * 10000);
	sha256ofString((BYTE *)(*bloc).liste[0], (*bloc).hash_root);
	(*bloc).nonce = 0;
	(*bloc).indice = 0;
	// Calcul du hash du bloc
	char chaine_curr_hash[(SHA256_BLOCK_SIZE * 2 + 1) * 2 + 50];
	sprintf(
		chaine_curr_hash,
		"%s%s%s%d",
		(*bloc).prev_hash,
		(*bloc).timestamp,
		(*bloc).hash_root,
		(*bloc).nonce);
	sha256ofString((BYTE *)chaine_curr_hash, (*bloc).curr_hash);
	// Ajout du bloc a la blockchain
	(*blockchain).liste[0] = *bloc;
	(*blockchain).nbBloc = 1;
	return blockchain;
}

void miner(
	char *user,
	struct Blockchain *blockchain,
	char *liste_users[max_nb_users]) {
	printf("NB BLOC : %d\n", (*blockchain).nbBloc);
	if ((*blockchain).nbBloc == 0 || (*blockchain).nbBloc > max_nb_bloc) {
		printf("Pas de bloc Genesis, ou impossible de miner.\n");
	} else {
		// Initialisation
		int nb_trans;
		char *liste_trans[max_nb_trans];
		// On choisit combien de transaction(s)
		nb_trans = rand() % max_nb_trans + 1;
		// On genere la liste de transactions en fonction
		generate_trans(liste_users, nb_trans, liste_trans);
		printf("gener fin  0\n");
		// On initialise le bloc dans la bloc chain, on sait qu'il n'y a qu'un
		// seul bloc car max_nb_trans<=10 donc contenable dans 1 seul bloc
		printf("init 0\n");
		initBloc(blockchain, liste_trans, nb_trans);
		printf("init 1\n");
		// On creee une chaine pour verifier le debut du hash en fonction de la
		// difficulte
		char verif[difficulte + 1];
		for (int co = 0; co < difficulte; co++) {
			verif[co] = '0';
		}
		// Verification du hash du bloc
		int co = 0;
		while (strncmp(
				   blockchain->liste[blockchain->nbBloc - 1].curr_hash,
				   verif,
				   difficulte) != 0 ||
			   co > 1000) {
			++(blockchain->liste[blockchain->nbBloc - 1].nonce);
			char chaine_curr_hash[(SHA256_BLOCK_SIZE * 2 + 1) * 2 + 28];
			sprintf(
				chaine_curr_hash,
				"%s%s%s%d",
				blockchain->liste[blockchain->nbBloc - 1].prev_hash,
				blockchain->liste[blockchain->nbBloc - 1].timestamp,
				blockchain->liste[blockchain->nbBloc - 1].hash_root,
				blockchain->liste[blockchain->nbBloc - 1].nonce);
			sha256ofString(
				(BYTE *)chaine_curr_hash,
				blockchain->liste[blockchain->nbBloc - 1].curr_hash);
			++co;
		}
		printf(
			"Hash curr : %s\n",
			blockchain->liste[blockchain->nbBloc - 1].curr_hash);
	}
}

int main(void) {
	char *liste_users[10] = {"Creator",
							 "Thomas",
							 "Solene",
							 "Lola",
							 "Antonin",
							 "Valentin",
							 "Alicia",
							 "Pauline",
							 "Remi",
							 "Papion"};
	struct Blockchain *test = createBlockchain();
	printf("debut\n");
	miner(liste_users[1], test, liste_users);
	printf("fin\n");
	/* for (int co = 1 ; co < max_nb_bloc ; co++) {
	  miner(liste_users[co], test, liste_users);
	  printf("NB BLOC : %d\n", test->nbBloc);
	}*/
	return 0;
}
