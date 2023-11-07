#if !defined(_LIST_H)
#define _LIST_H

//struttura di un semplice nodo che rappresenta un elemento di una lista  
struct nodo {
	char* path;			//pathname di un file 
	struct nodo *next;	// riferimento al prossimo elemento
};
typedef struct nodo *lista;


//struttura di un semplice nodo che rappresenta un elemento della lista ordinata Finale che si trova nel Collector
struct nodoF {
	long result; 		//valore della sommatoria 
	char* path;			//pathname di un file
	struct nodoF *next;	// riferimento al prossimo elemento
};
typedef struct nodoF *listaF;

void deleteLista(lista *);
void deleteListaF(listaF *);

lista inserisciTestaLista(lista, char* );


void stampaLista(lista);
void stampaListaF(listaF);


long longcmp(long, long );
void inserisciOrdina(listaF *, long, char*);

#endif /* _LIST_H */
