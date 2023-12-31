#include <list.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void deleteLista(lista *head){
	lista next_elem =NULL;
	while(*head != NULL){
		next_elem = (*head)->next;
		free((*head)->path);
		free(*head);
		*head = next_elem;
	}
}//end deleteLista

lista inserisciTestaLista(lista root, char* stringa){
	lista t;
	int lenght = strlen(stringa);

	t = malloc(sizeof(struct nodo));
	t -> path=malloc(1+sizeof(char)*lenght);
	t -> next = root;

	strcpy(t->path, stringa);

	return t;
}


void stampaLista(lista l) {
	lista s;
	s=l;
	while(s!=NULL) {
		int dim = strlen(s->path);
		printf("-> %s (dim = %d)  ", s->path, dim);
		s = s-> next;
		printf("\n");
	}
	printf("->NULL\n");
	fprintf(stdout, "-----------------------\n");
}//end stampaLista

// -----------------------------------------------
void deleteListaF(listaF *head){
	listaF next_elem = NULL;

	while(*head != NULL){
		next_elem = (*head)->next;
		free((*head)->path);
		free(*head);
		*head = next_elem;
	}
}//end deleteLista

void inserisciOrdina(listaF *head, long value, char* stringa){
	int lenght = strlen(stringa) +1; //+ '\0'

	listaF corr = *head;
	listaF prec = NULL;
	listaF new = malloc(sizeof(struct nodoF));
		new -> result = value; 
		new -> path   = malloc(sizeof(char)*lenght);
		new -> next   = NULL;

		strncpy(new->path, stringa, lenght);
	//nuovo nodo creato

	while(corr != NULL && longcmp(corr->result, new->result)<0){
		prec = corr;
		corr = corr-> next;
	}
	if(prec != NULL){
		prec-> next = new;
		new->next = corr;
	}
	else {
		new->next = *head;
		*head = new;
	}
}//listaOrdinata

long longcmp(long a, long b){	return a-b;	}

void stampaListaF(listaF l){
	while(l!=NULL) {
		printf("%ld\t%s\n", l->result, l->path);
		l = l-> next;
	}
	// printf("->NULL\n");
}