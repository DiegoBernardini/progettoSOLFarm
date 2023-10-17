#include <cmdLineParser.h>

void printOptionList(char* s){
	fprintf(stderr, "usa: %s [-n <num> -q <num> -d <nome-file>]  [<nome-file> ...]\n", s);
	fprintf(stderr, " -n <nthread> specifica il numero di thread worker a <nthread>, default: %d\n", DEFAULT_NTHREAD);
	fprintf(stderr, " -q <qlen> specifica la lunghezza della coda concorrente a <qlen>, default: %d\n", DEFAULT_QUEUE_LEN);
	fprintf(stderr, " -d <directory-name> specifica una directory che contiene file bin o altre directory che contengono file bin\n");
	fprintf(stderr, " -t <delay> specifica un tempo in millisec che intercorre tra l'invio di due richieste successive agli worker da parte del master, default: %d\n", DEFAULT_TIME_DELAY);
}//end printOptionList


int isdot(const char dir[]) {
	int l = strlen(dir);
	if ( (l>0 && dir[l-1] == '.') ) return 1;
	return 0;
}//end isDot


char* cwd() {
	char* buf = malloc(LUNGHEZZA_STRINGHE*sizeof(char));
	if (!buf) {
		perror("cwd malloc");
		return NULL;
	}
	if (getcwd(buf, LUNGHEZZA_STRINGHE) == NULL) {
		if (errno==ERANGE) { // il buffer e' troppo piccolo, lo allargo
			char* buf2 = realloc(buf, 2*LUNGHEZZA_STRINGHE*sizeof(char));
			if (!buf2) {
				perror("cwd realloc");
				free(buf);
				return NULL;
			}
			buf = buf2;
			if (getcwd(buf,2*LUNGHEZZA_STRINGHE)==NULL) { // mi arrendo....
				perror("cwd eseguendo getcwd");
				free(buf);
				return NULL;
			}
		} else {
			perror("cwd eseguendo getcwd");
			free(buf);
			return NULL;
		  }
	}
	return buf;
}//end cwd

/*funzione che va a visitare ricorsivamente una cartella
* @nomedir: nome della directory che si vuol visitare
* @lista_file: elenco dei file regolari ottenuti a partire dalla cartella nomedir
*/ 
void exploreDir(const char nomedir[], lista *lista_file, char* pathRelativo) {
	// printf("sono appena entrato e pathrelativo =%s\n", pathRelativo );

    struct stat statbuf;
    int r;
    SYSCALL_EXIT(stat,r,stat(nomedir,&statbuf),"Facendo stat del nome %s: errno=%d\n", nomedir, errno);

    DIR * dir;
    // fprintf(stdout, "-----------------------\n");
    // fprintf(stdout, "Directory %s:\n",nomedir);
    
    if ((dir=opendir(nomedir)) == NULL) {
		perror("opendir");
		print_error("Errore aprendo la directory %s\n", nomedir);
		return;
    }
    struct dirent *file;
   	//lista l=NULL;
    
	while((errno=0, file = readdir(dir)) != NULL) {
	    struct stat statbuf;
	    char filename[LUNGHEZZA_STRINGHE];//array in cui memorizzo il path assoluto dei file 
	    char buf[LUNGHEZZA_STRINGHE]; //array in cui vado a memorizzare il path relativo dei file che incontro a partire dal nome della directory passata come argomento nella cmd

	    int len1 = strlen(nomedir);
	    int len2 = strlen(file->d_name);
	    if ((len1+len2+2)>LUNGHEZZA_STRINGHE) {  //si assume che non succeda
			fprintf(stderr, "ERRORE: LUNGHEZZA_STRINGHE troppo piccolo\n");
			exit(EXIT_FAILURE);
	    }

	    //creo manualmente il path assoluto
	    strncpy(filename,nomedir,      LUNGHEZZA_STRINGHE-1);
	    strncat(filename,"/",          LUNGHEZZA_STRINGHE-1);
	    strncat(filename,file->d_name, LUNGHEZZA_STRINGHE-1);

	    // printf("FILENAME APPENA CREATO =%s\n", filename );

	    // creo manualmente il path relativo
	    strncpy(buf, pathRelativo, LUNGHEZZA_STRINGHE-1);
		strncat(buf, "/",          LUNGHEZZA_STRINGHE-1);
	    strncat(buf, file->d_name, LUNGHEZZA_STRINGHE-1);

	    // printf("PATHRELATIVO APPENA CREATO =%s\n", buf);

	    if (stat(filename, &statbuf)==-1) {
			perror("eseguendo la stat");
			print_error("Errore nel file %s\n", filename);
			return;
	    }
	    //se e' una directory ci entro 
	    if(S_ISDIR(statbuf.st_mode)) {
			// if ( !isdot(filename) ) exploreDir(filename, lista_file);
			if ( !isdot(filename) ) exploreDir(filename, lista_file, buf);

	    }
	    // se e' un file regolare
	    else if(S_ISREG(statbuf.st_mode)){
	    	/*le righe seguenti sono state commentate per superare i test, 
	    	* dato che facevano uso del path assoluto dei file
	    	*/
	    	// *lista_file = inserisciTestaLista(*lista_file, filename);
	    	//inserisciCodaLista(lista_file, filename);
			//listaOrdinata(lista_file, filename);

	    	// printf("FILE REGOLARE: filename= %s, file->d_name=%s\n, pathRelativo=%s\n", filename, file->d_name, pathRelativo);
	    	// printf("STO PER INSERIRE NELLA LISTA %s\n", buf );
	    	*lista_file = inserisciTestaLista(*lista_file, buf);
		}
	}
	if (errno != 0) perror("readdir");//TODO gestione errore?
	closedir(dir);
	// fprintf(stdout, "-----------------------\n");
}//end exploreDir

int cmdParse(int argc, char* argv[], flag* options){
	int opt;
	long tmp;
	while((opt = getopt(argc, argv, ":n:q:d:t:")) != -1) {
		switch(opt) {
		case 'd': //specifico la directory
			if (strlen(optarg) > LUNGHEZZA_STRINGHE) { //si assume che non succeda 
		    	printf("la string di '-%c e' troppo lunga\n", optopt);
				return EXIT_FAILURE;
			}			
			//check tipo file
			struct stat file_info;
			if(stat(optarg, &file_info) == -1 ){
				perror("stat");
				exit(EXIT_FAILURE);
			}
			if (!S_ISDIR(file_info.st_mode))//NON e' una cartella
				break;
			// recupero la cwd 
			char* now=cwd();
			// cambio directory
			// fprintf(stdout, "cwd nel cmdParse:%s\n\n", now);
			// printf("optarg prima del parsing  = %s\n", optarg);
			//imposto la nuova cwd
			if(chdir(optarg) == -1){ 
				perror("chdir");
				free(now);
				exit(EXIT_FAILURE);
			}
			char *new_cwd = cwd();
			//vado alla nuova cartella
			// printf("new_cwd=%s now=%s optarg=%s\n", new_cwd, now, optarg);

			exploreDir(new_cwd, &(options->l), optarg);//CON PATH ASSOLUTO

			free(new_cwd);  
			// stampaLista(options->l);
			
			if(chdir(now) == -1){ 
				perror("chdir");
				free(now);
				exit(EXIT_FAILURE);
			}
			free(now);
			// printf("cwd dopo exploreDir  = %s\n", cwd);
			break;
		case '?': 
			printf("WARNING: l'opzione '-%c' non e' riconosciuta\n", optopt);
			break;
		case ':':
		    printf("WARNING, l'opzione '-%c' richiede un argomento <uso valore di default>\n", optopt);
		    fprintf(stderr, "usa: %s [-n <num> -q <num> -d <nome-file>]  [<nome-file> ...]\n", argv[0]);
			fprintf(stderr, " -n <nthread> specifica il numero di thread worker a <nthread>, default: %d\n", DEFAULT_NTHREAD);
			fprintf(stderr, " -q <qlen> specifica la lunghezza della coda concorrente a <qlen>, default: %d\n", DEFAULT_QUEUE_LEN);
			fprintf(stderr, " -d <directory-name> specifica una directory che contiene file bin o altre directory che contengono file bin\n");
			fprintf(stderr, " -t <delay> specifica un tempo in millisec che intercorre tra l'invio di due richieste successive agli worker da parte del master, default: %d\n", DEFAULT_TIME_DELAY);
			//printOptionList(argv[0]);
		    break;
		case 'n':
		case 'q':
		case 't'://n: mod 4 = 2, q: mod 4 = 1, t: mod 4 = 0 SEPARARE T DAGLI ALTRI DUE 
			// printf("sono dentro lo switch opzione %c optarg = %s \n", opt, optarg);
			if (isNumber(optarg, &tmp) != 0) {
		    	fprintf(stderr, "WARNING: l'argomento di '-%c' non e' un numero valido,\n", optopt);//TODO uso il valore di default
		  		return EXIT_FAILURE;
		  	}
			//vedo quale opzione e' stata inserita
	  		int i = opt % 4;
		  	if (tmp <= 0) {
		  		if(i == 2) // option '-n'
			    	fprintf(stderr, "WARNING: deve esserci almeno 1 thread\n");//TODO uso il valore di default
			    else if(i == 1) // option '-q'
			    	fprintf(stderr, "WARNING: la coda deve essere lunga almeno 1\n");//TODO uso il valore di default
				else //option '-t' 
					fprintf(stderr, "WARNING: il tempo di delay deve essere >= 0\n");//TODO uso il valore di default
				return EXIT_FAILURE; 
		  	}
		  	if(i == 2) 		options->n = tmp;
		  	else if (i ==1) options->q = tmp;
		  	else 			options->t = tmp;
		  	break;
		default:;
		} //end switch
    } //end while

	for(int i=optind; i<argc; i++){
		struct stat statbuf;
    	char *path = cwd();
		if (stat(argv[i], &statbuf)==-1) {
			perror("eseguendo la stat");
			print_error("Errore nel file %s (non verra' considerato)\n", argv[i]);
	    }else if(S_ISREG(statbuf.st_mode)){
	    	/*le righe successive sono state commentate per superare i test 
	    	 *in quanto sfruttavano il percorso assoluto del file
	    	 */ 
	  		
	  		// (path,"/",          LUNGHEZZA_STRINGHE-1);
	  		// strncat(path, argv[i],     LUNGHEZZA_STRINGHE-1);

			//inserisciCodaLista(&(options->l),path);
			// options->l = inserisciTestaLista(options->l, path);
	    	//listaOrdinata( &(options->l), path);
			
			options->l = inserisciTestaLista(options->l, argv[i]);
	    }
		free(path);
	}
    return EXIT_SUCCESS;
}//end cmdParse